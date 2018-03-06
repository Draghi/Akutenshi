/**
 * Copyright 2017 Michael J. Baker
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#ifndef AK_ENGINE_ECS_HPP_
#define AK_ENGINE_ECS_HPP_

#include <ak/container/SlotMap.hpp>
#include <ak/container/UnorderedVector.hpp>
#include <ak/data/Hash.hpp>
#include <ak/data/PValue.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/event/Event.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/String.hpp>
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#define AKE_DEFINE_COMPONENT_MANAGER(componentName) \
	private: friend class ::ake::EntityManager; \
	public:  constexpr static ::std::string_view COMPONENT_NAME = std::string_view(#componentName, sizeof(#componentName) - 1); \
	public:  constexpr static ::ake::ComponentID COMPONENT_ID = akd::hash32FNV1A<char>(#componentName, sizeof(#componentName) - 1); \
	public:  const ::std::string_view& name() const override { return COMPONENT_NAME; } \
	public:  ::ake::ComponentID id() const override { return COMPONENT_ID; } \

namespace ake {
	namespace internal {
		struct ComponentManagerAccessor;
	}

	class ComponentManager;
	class EntityManager;
	using ComponentID = uint32;
	using    EntityID = akc::SlotID;
	using  InstanceID = akc::SlotID;

	bool initEntityComponentSystem();
	EntityManager createEntityManager();

	// When inheriting from component manager be sure to start
	// the body definition with the AKE_DEFINE_COMPONENT_MANAGER macro.
	// It implements any required functionality that's fairly trivial, particularly if it's static in nature.
	class ComponentManager {
		friend struct ::ake::internal::ComponentManagerAccessor;
		friend class EntityManager;
		protected:

			// Components are created by a templated method that expects the following method to exist
			// The first argument is the id of the entity and the remaining arguments can be anything you want
			// Overload resolution should work fine here.
			// Components may also be created by the deserialize method, which is virtual.
			//InstanceID createComponentFor(EntityID entityID, ... args) = 0;

			virtual bool destroyComponent(InstanceID instanceID) = 0;

			//bool serialize(akd::PValue& dest, InstanceID instanceID) = 0;
			//InstanceID deserialize(EntityID entityID, const akd::PValue& src) = 0;

		public:
			virtual ~ComponentManager() = default;

			virtual const std::string_view& name() const = 0;
			virtual ComponentID id() const = 0;
	};

	class EntityManager final {
		EntityManager(const EntityManager&) = delete;
		EntityManager& operator=(const EntityManager&) = delete;
		private:
			using components_t = std::unordered_multimap<ComponentID, InstanceID>;
			using names_t = std::pair<std::string, akc::UnorderedVector<EntityID>>;

			std::unordered_map<ComponentID, std::unique_ptr<ComponentManager>> m_registry;

			akc::SlotMap<names_t> m_names;
			std::unordered_map<std::string, akc::SlotID> m_lookupEntitiesByName;

			akc::SlotMap<akc::SlotID> m_entityNames;
			akc::SlotMap<components_t> m_entityComponents;


		public:
			EntityManager(std::unordered_map<ComponentID, std::unique_ptr<ComponentManager>>&& registry) : m_registry(std::move(registry)) {}

			// ////////////// //
			// // Entities // //
			// ////////////// //

			akc::SlotID createEntity(const akc::SlotID& nameID) {
				auto entityID = m_entityNames.insert(nameID).first;
				if (entityID != m_entityComponents.insert({ }).first) throw std::logic_error("EntityManager::createEntity: Entity ID mismatch");
				m_names[nameID].second.insert(entityID);
				return entityID;
			}

			akc::SlotID createEntity(const std::string& name) {
				auto iter = m_lookupEntitiesByName.find(name);
				if (iter != m_lookupEntitiesByName.end()) return createEntity(iter->second);
				return createEntity(m_lookupEntitiesByName.insert(std::make_pair(name, m_names.insert(names_t{name,{}}).first)).first->second);
			}

			bool destroyEntity(EntityID entityID) {
				// Remove Components
				auto& components = m_entityComponents[entityID];
				ComponentID       lastComponentID  = 0;
				ComponentManager* componentManager = nullptr;
				for(auto iter = components.begin(); iter != components.end(); iter++) {
					if (std::exchange(lastComponentID, iter->first) != iter->first) componentManager = m_registry[lastComponentID].get();
					componentManager->destroyComponent(iter->second);
				}

				// Cache Entity Name
				auto name = m_entityNames[entityID];

				// Erase Entity Data
				m_entityNames.erase(entityID);
				m_entityComponents.erase(entityID);

				// Update Name Lookup
				auto & namedEntities = m_names.at(name);
				namedEntities.second.erase(std::find(namedEntities.second.begin(), namedEntities.second.end(), entityID));
				if (namedEntities.second.size() == 0) {
					m_lookupEntitiesByName.erase(namedEntities.first);
					m_names.erase(name);
				}

				return true;
			}

			void destroyAllEntities() {
				while(!m_entityNames.empty()) destroyEntity(m_entityNames.slotIDFor(m_entityNames.size() - 1));
			}

			// //////////////// //
			// // Components // //
			// //////////////// //
			template<typename type_t, typename... vargs_t> InstanceID addComponent(EntityID entityID, const vargs_t&... vargs) {
				if (!m_entityNames.exists(entityID)) throw std::out_of_range("EntityManager::addNewComponent: Attempt to index out of bounds");
				auto instanceID = getComponentManager<type_t>().createComponentFor(entityID, std::forward<const vargs_t&>(vargs)...);
				m_entityComponents[entityID].insert(std::make_pair(type_t::COMPONENT_ID, instanceID));
				return instanceID;
			}

			bool removeComponent(EntityID entityID, ComponentID componentID, const InstanceID& instanceID) {
				if (!m_entityNames.exists(entityID)) throw std::out_of_range("EntityManager::removeComponentsFrom: Attempt to index out of bounds");
				auto compRange = m_entityComponents[entityID].equal_range(componentID);
				auto entryIter = std::find_if(compRange.first, compRange.second, [&](const auto& v){return v.second == instanceID; });
				if (entryIter == compRange.second) return false;
				if (!m_registry[componentID]->destroyComponent(instanceID)) throw std::logic_error(ak::buildString("EntityManager::removeComponentsFrom: Component Data Mismatch between EntityManager and ComponentManager '", m_registry[componentID]->name(), "'"));
				m_entityComponents[entityID].erase(entryIter);
				return true;
			}

			template<typename type_t> bool removeComponent(EntityID entityID, const InstanceID& instanceID) {
				return removeComponent(entityID, type_t::COMPONENT_ID, instanceID);
			}

			// /////////// //
			// // Query // //
			// /////////// //

			ComponentManager& getComponentManager(ComponentID componentID) {
				return *m_registry.at(componentID);
			}

			ComponentManager& getComponentManager(ComponentID componentID) const {
				return *m_registry.at(componentID);
			}

			template<typename type_t> type_t& getComponentManager() {
				return dynamic_cast<type_t&>(*m_registry.at(type_t::COMPONENT_ID));
			}

			template<typename type_t> type_t& getComponentManager() const {
				return dynamic_cast<type_t&>(*m_registry.at(type_t::COMPONENT_ID));
			}

			std::vector<EntityID> findAllEntitiesNamed(const std::string& name) const {
				auto iter = m_lookupEntitiesByName.find(name);
				if (iter == m_lookupEntitiesByName.end()) return {};
				return m_names[iter->second].second.getContainer();
			}

			std::string nameFor(EntityID entityID) const { return m_names[m_entityNames[entityID]].first; }
			akSize entityCount() const { return m_entityNames.size(); }
	};

	using ComponentManagerFactory_f  = std::unique_ptr<ComponentManager>();
	using RegisterComponentManger_f = void(ComponentID id, const std::function<ComponentManagerFactory_f>& factoryFunc);
	AK_DEFINE_EVENT(RegisterComponentManagersEvent, RegisterComponentManger_f*, false);
	const akev::DispatcherProxy<RegisterComponentManagersEvent>& registerComponentManagersEventDispatch();
}

#endif
