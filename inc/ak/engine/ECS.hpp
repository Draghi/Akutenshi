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
#include <ak/data/PValue.hpp>
#include <ak/data/SUID.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/event/Event.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#define AKE_DEFINE_COMPONENT_MANAGER(componentName) \
	private: friend class ::ake::EntityManager; \
	public:  constexpr static ::std::string_view COMPONENT_NAME = std::string_view(#componentName, sizeof(#componentName) - 1); \
	public:  constexpr static ::ake::ComponentTID COMPONENT_TID = akd::hash32FNV1A<char>(#componentName, sizeof(#componentName) - 1); \
	public:  const ::std::string_view& name() const override { return COMPONENT_NAME; } \
	public:  ::ake::ComponentTID tid() const override { return COMPONENT_TID; } \

namespace ake {
	class ComponentManager;
	class EntityManager;
	using    EntityID = akc::SlotID;
	using   EntityUID = uint64;

	using  ComponentID = akc::SlotID;
	using ComponentUID = uint64;
	using ComponentTID = uint32;

	using EUIDGenerator_f = EntityUID(EntityManager&);
	using CUIDGenerator_f = ComponentUID(EntityManager&);

	struct CUIDRecord {
		EntityID entityID;
		ComponentTID componentTypeID;
		ComponentID componentID;
		bool isValid() const { return entityID && componentID; }
		bool operator==(const CUIDRecord& o) const { return (entityID == o.entityID) && (componentTypeID == o.componentTypeID) && (componentID == o.componentID); }
		operator bool() const { return isValid(); }
	};

	namespace internal {
		struct ComponentManagerAccessor;
	}
}

namespace std {
	template<> struct hash<ake::CUIDRecord> {
		size_t operator()(const ake::CUIDRecord& v) const {
			return 281474976710656ull*v.componentID.index ^ 16777216ull*v.componentTypeID ^ v.entityID.index;
		}
	};
}

namespace ake {
	bool initEntityComponentSystem();
	EntityManager createEntityManager(const std::function<EUIDGenerator_f>& euidGenerator, const std::function<CUIDGenerator_f>& cuidGenerator);

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
			//ComponentID newComponent(EntityManager& manager, EntityID entityID, ... args) = 0;

			virtual bool deleteComponent(EntityManager& manager, EntityID entityID, ComponentID componentID) = 0;

			virtual bool serialize(akd::PValue& /*dest*/, EntityManager& /*manager*/, EntityID /*entityID*/, ComponentID /*componentID*/) {
				return false;
			}

			virtual ComponentID deserialize(EntityManager& /*manager*/, EntityID /*entityID*/, const akd::PValue& /*src*/) {
				throw std::logic_error("ComponentManager::deserialize: Deserialize not implemented.");
			}

			//bool serialize(akd::PValue& dest, ComponentID componentID) = 0;
			//InstanceID deserialize(EntityID entityID, const akd::PValue& src) = 0;

		public:
			virtual ~ComponentManager() = default;

			virtual const std::string_view& name() const = 0;
			virtual ComponentTID tid() const = 0;
	};

	/**
	 * Temp Notes:
	 * Create entity w/entityUID returns !isValid if entityUID exists
	 * Create instance w/componentUID returns !isValid id componentUID exists
	 *
	 * Entity/Instance insert repeats until it gets a valid id.
	 * This causes id reuse when loading, and is slow.
	 *
	 * Two options are:
	 * - Integrate id generation and serialize info (Decent solution)
	 * - Remap uuids (Better... ish... Condenses ids and frees up ids on save/load, dangling uids would be bad though...)
	 */
	class EntityManager final {
		EntityManager(const EntityManager&) = delete;
		EntityManager& operator=(const EntityManager&) = delete;
		private:
			struct Entity {
				EntityUID uid;
				akc::SlotID name;
				std::unordered_map<ComponentTID, std::vector<ComponentID>> components;
			};

			// Component Managers
			std::unordered_map<ComponentTID, std::unique_ptr<ComponentManager>> m_componentTypes;

			// 'Compact' Name Storage/Lookup
			using names_t = std::pair<std::string, akc::UnorderedVector<EntityID>>;
			akc::SlotMap<names_t> m_names;
			std::unordered_map<std::string, akc::SlotID> m_lookupNIDbyName;
			std::unordered_map<EntityUID,   EntityID   > m_lookupEIDbyEUID;
			std::unordered_map<ComponentUID, CUIDRecord> m_lookupCIDbyCUID;
			std::unordered_map<CUIDRecord, ComponentUID> m_lookupCUIDByCID;

			// Entity Data
			akc::SlotMap<Entity> m_entities;

			std::function<EUIDGenerator_f> m_euidGenerator;
			std::function<CUIDGenerator_f> m_cuidGenerator;

			void associateCUID(ComponentUID componentUID, CUIDRecord cuidRecord);
			void deassociateCUID(ComponentUID componentUID, CUIDRecord cuidRecord);

		public:
			EntityManager(std::unordered_map<ComponentTID, std::unique_ptr<ComponentManager>>&& registry, const std::function<EUIDGenerator_f>& euidGenerator, const std::function<CUIDGenerator_f>& cuidGenerator);
			EntityManager(EntityManager&& o) = default;
			EntityManager& operator=(EntityManager&& o) = default;

			// ////////////// //
			// // Entities // //
			// ////////////// //
			EntityID newEntity(EntityUID uid, const akc::SlotID& nameID);
			EntityID newEntity(const akc::SlotID& nameID);

			EntityID newEntity(EntityUID uid, const std::string& name);
			EntityID newEntity(const std::string& name);

			bool deleteEntity(EntityID entityID);
			void deleteAllEntities();

			std::string getEntityName(EntityID entityID) const;
			EntityUID getEntityUID(EntityID entityID) const;
			akSize getEntityCount() const;

			std::vector<EntityID> findEntitiesNamed(const std::string& name) const;

			// //////////////// //
			// // Components // //
			// //////////////// //
			template<typename type_t, typename... vargs_t> ComponentID addComponent(ComponentUID componentUID, EntityID entityID, const vargs_t&... vargs);
			template<typename type_t, typename... vargs_t> ComponentID addComponent(EntityID entityID, const vargs_t&... vargs);

			bool removeComponent(EntityID entityID, ComponentTID componentTID, ComponentID componentID);
			template<typename type_t> bool removeComponent(EntityID entityID, ComponentID componentID);

			template<typename type_t> decltype(std::declval<type_t>().getInstance(ComponentID())) getComponent(EntityID entityID, ComponentID componentID);
			template<typename type_t> decltype(std::declval<type_t>().getInstance(ComponentID())) getComponent(EntityID entityID);
			template<typename type_t> ComponentID getComponentID(EntityID entityID) const;

			ComponentUID getInstanceUID(EntityID entityID, ComponentTID componentTID, ComponentID componentID) const;
			CUIDRecord getInstanceID(ComponentUID componentID) const;

			std::vector<ComponentID> findComponentsOfTypeFor(ake::EntityID entityID, ake::ComponentTID componentID) const;
			template<typename type_t> std::vector<ComponentID> findComponentsOfTypeFor(ake::EntityID entityID);

			// ///////////////////// //
			// // Component Types // //
			// ///////////////////// //
			ComponentManager& getComponentType(ComponentTID componentID);
			const ComponentManager& getComponentType(ComponentTID componentID) const;

			template<typename type_t> type_t& getComponentType();
			template<typename type_t> const type_t& getComponentType() const;

			std::vector<ComponentTID> findComponentTypesFor(ake::EntityID entityID) const;
	};

	using ComponentManagerFactory_f  = std::unique_ptr<ComponentManager>();
	using RegisterComponentManger_f = void(ComponentTID id, const std::function<ComponentManagerFactory_f>& factoryFunc);
	AK_DEFINE_EVENT(RegisterComponentManagersEvent, RegisterComponentManger_f*, false);
	const akev::DispatcherProxy<RegisterComponentManagersEvent>& registerComponentManagersEventDispatch();
}

namespace ake {
	template<typename type_t, typename... vargs_t> ComponentID EntityManager::addComponent(ComponentUID componentUID, EntityID entityID, const vargs_t&... vargs) {
		if (!m_entities.exists(entityID)) throw std::out_of_range("EntityManager::addNewComponent: Attempt to index out of bounds");
		if (getInstanceID(componentUID)) return {};

		auto componentID = getComponentType<type_t>().newComponent(*this, entityID, std::forward<const vargs_t&>(vargs)...);
		if (!componentID) throw std::runtime_error("EntityManager::addComponent: Failed to create component");

		m_entities[entityID].components[type_t::COMPONENT_TID].push_back(componentID);
		associateCUID(componentUID, CUIDRecord{entityID, type_t::COMPONENT_TID, componentID});

		return componentID;
	}

	template<typename type_t, typename... vargs_t> ComponentID EntityManager::addComponent(EntityID entityID, const vargs_t&... vargs) {
		ComponentID result;
		while(!(result = addComponent<type_t, vargs_t...>(m_cuidGenerator(*this), entityID, std::forward<const vargs_t&>(vargs)...)));
		return result;
	}

	template<typename type_t> bool EntityManager::removeComponent(EntityID entityID, ComponentID componentID) {
		return removeComponent(entityID, type_t::COMPONENT_ID, componentID);
	}

	template<typename type_t> decltype(std::declval<type_t>().getInstance(ComponentID())) EntityManager::getComponent(EntityID entityID, ComponentID componentID) {
		if (!m_entities.exists(entityID)) throw std::out_of_range("EntityManager::getComponent: Attempt to index entity out of bounds");
		if (m_lookupCUIDByCID.find(CUIDRecord{entityID, type_t::COMPONENT_ID, componentID}) == m_lookupCUIDByCID.end()) throw std::out_of_range("EntityManager::getComponent: Attempt to index component out of bounds");
		return getComponentType<type_t>().getInstance(componentID);
	}

	template<typename type_t> decltype(std::declval<type_t>().getInstance(ComponentID())) EntityManager::getComponent(EntityID entityID) {
		return getComponentType<type_t>().getInstance(getComponentID<type_t>(entityID));
	}

	template<typename type_t> ComponentID EntityManager::getComponentID(EntityID entityID) const {
		if (!m_entities.exists(entityID)) throw std::out_of_range("EntityManager::getComponentID: Attempt to index entity out of bounds");

		auto componentIter = m_entities[entityID].components.find(type_t::COMPONENT_TID);
		if (componentIter == m_entities[entityID].components.end()) throw std::out_of_range("EntityManager::getComponentID: Attempt to index component out of bounds");
		if (componentIter->second.size() != 1) throw std::out_of_range("EntityManager::getComponentID: Attempt to index component out of bounds");

		return componentIter->second.front();
	}

	template<typename type_t> type_t& EntityManager::getComponentType() {
		return dynamic_cast<type_t&>(getComponentType(type_t::COMPONENT_TID));
	}

	template<typename type_t> const type_t& EntityManager::getComponentType() const {
		return dynamic_cast<type_t&>(getComponentType(type_t::COMPONENT_TID));
	}

	template<typename type_t> std::vector<ComponentID> EntityManager::findComponentsOfTypeFor(ake::EntityID entityID) {
		return findComponentsOfTypeFor(entityID, type_t::COMPONENT_ID);
	}
}

#endif
