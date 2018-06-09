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

#ifndef AK_ENGINE_ENTITYMANAGER_HPP_
#define AK_ENGINE_ENTITYMANAGER_HPP_

#include <ak/container/SlotMap.hpp>
#include <ak/container/UnorderedVector.hpp>
#include <ak/data/PValue.hpp>
#include <ak/engine/ComponentManager.hpp>
#include <ak/engine/internal/EntityGraph.hpp>
#include <ak/engine/internal/NameStorage.hpp>
#include <ak/engine/Type.hpp>
#include <ak/event/Dispatcher.hpp>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ake {

	using EntityUIDGenerator_f = EntityUID(EntityManager&);

	class Scene;
	class EntityManager;

	class Entity final {
		private:
			EntityManager* m_manager;
			EntityID m_id;

		public:
			Entity(EntityManager& manager, EntityID id);

			// ////////////////// //
			// // Entity Graph // //
			// ////////////////// //
			bool setParent(EntityID newParent);

			bool isParentRoot() const;

			EntityID parentID() const;
			const akc::UnorderedVector<EntityID>& childrenIDs() const;

			Entity parent() const;
			akc::UnorderedVector<Entity> children() const;

			Entity findFirstChildNamed(const std::string& name) const;
			akc::UnorderedVector<Entity> findAllChildrenNamed(const std::string& name) const;

			// //////////////// //
			// // Components // //
			// //////////////// //
			template<typename component_t, typename... vargs_t> bool createComponent(const vargs_t&... vargs);

			bool destroyComponent(ComponentID componentID);
			template<typename component_t> bool destroyComponent();

			bool serializeComponent(akd::PValue& dest, ComponentID componentID);
			bool deserializeComponent(ComponentID componentID, const akd::PValue& src);

			bool hasComponent(ComponentID componentID) const;
			template<typename component_t> bool hasComponent() const;

			template<typename component_t> decltype(std::declval<typename component_t::MANAGER>().component(EntityID())) component();
			template<typename component_t> decltype(std::declval<const typename component_t::MANAGER>().component(EntityID())) component() const;

			// //////////// //
			// // Events // //
			// //////////// //

			const akev::DispatcherProxy<EntityParentChangedEvent> parentChanged() const;

			// /////////// //
			// // Other // //
			// /////////// //

			const std::string& name() const;
			EntityID id() const;
			EntityUID uid() const;

			operator EntityID() const { return m_id; }
	};

	class EntityManager final {
		EntityManager(const EntityManager&) = delete;
		EntityManager& operator=(const EntityManager&) = delete;
		private:
			ake::Scene* m_owner;

			// Component Storage
			std::unordered_map<ComponentID, std::unique_ptr<ComponentManager>> m_components;

			// Entity UID Generation
			std::function<EntityUIDGenerator_f> m_entityUIDGenerator;

			// Entity Data
			internal::NameStorage m_entityNameStorage;
			akc::SlotMap<std::unordered_set<ComponentID>> m_entityComponentIDs;
			akc::SlotMap<EntityUID> m_entityUID;

			internal::EntityGraphManager m_entityGraph;

			// Entity Lookup
			std::unordered_map<EntityUID, EntityID> m_lookupEntityByUID;

			EntityUID nextEntityUID();

		public:
			EntityManager(ake::Scene& owner, std::function<EntityUIDGenerator_f> entityUIDGenerator);

			// ////////////// //
			// // Entities // //
			// ////////////// //

			Entity newEntity(const std::string& name, EntityID parentID = EntityID(), EntityUID targetUID = 0);
			bool deleteEntity(EntityID entityID);
			Entity getEntity(EntityID entityID);

			const std::string& entityName(EntityID entityID) const;
			std::unordered_set<ComponentID> entityComponents(EntityID entityID) const;
			EntityUID entityUID(EntityID entityID) const;

			bool setEntityParentID(EntityID entityID, EntityID parentID);
			EntityID entityParentID(EntityID entityID) const;
			const akc::UnorderedVector<EntityID>& entityChildrenIDs(EntityID entityID) const;
			EntityID entityFindFirstChildNamed(EntityID entityID, const std::string& name) const;
			akc::UnorderedVector<EntityID> entityFindAllChildrenNamed(EntityID entityID, const std::string& name) const;

			const std::unordered_set<EntityID>& entityGraphRoot() const;

			// //////////////// //
			// // Components // //
			// //////////////// //

			template<typename component_t, typename... vargs_t> bool createComponent(EntityID entityID, const vargs_t&... vargs);

			bool destroyComponent(EntityID entityID, ComponentID componentID);
			template<typename component_t> bool destroyComponent(EntityID entityID);

			bool serializeComponent(akd::PValue& dest, EntityID entityID, ComponentID componentID);
			bool deserializeComponent(EntityID entityID, ComponentID componentID, const akd::PValue& src);

			bool hasComponent(EntityID entityID, ComponentID componentID) const;
			template<typename component_t> bool hasComponent(EntityID entity) const;

			template<typename component_t> decltype(std::declval<typename component_t::MANAGER>().component(EntityID())) component(EntityID entity);
			template<typename component_t> decltype(std::declval<const typename component_t::MANAGER>().component(EntityID())) component(EntityID entity) const;

			// /////////////////////// //
			// // ComponentsManager // //
			// /////////////////////// //

			bool registerComponentManager(std::unique_ptr<ComponentManager>&& component);
			bool componentManagerExists(ComponentID componentID);

			ComponentManager& componentManager(ComponentID componentID);
			template<typename component_t> typename component_t::MANAGER& componentManager();

			const ComponentManager& componentManager(ComponentID componentID) const;
			template<typename component_t> const typename component_t::MANAGER& componentManager() const;

			// //////////////// //
			// // SceneGraph // //
			// //////////////// //

			const akev::DispatcherProxy<EntityParentChangedEvent> entityParentChanged() const;
			const akev::DispatcherProxy<EntityParentChangedEvent> entityParentChanged(EntityID entityID) const;

			// /////////// //
			// // Scene // //
			// /////////// //

			Scene& scene() { return *m_owner; }
			const Scene& scene() const { return *m_owner; }
	};
}


namespace ake {
	template<typename component_t, typename... vargs_t> bool EntityManager::createComponent(EntityID entityID, const vargs_t&... vargs) {
		auto& componentIDs = m_entityComponentIDs[entityID];
		auto iter = componentIDs.find(component_t::MANAGER::ID);
		if (iter != componentIDs.end()) return false;

		if (!componentManager<component_t>().createComponent(entityID, vargs...)) return false;
		componentIDs.insert(component_t::MANAGER::ID);
		return true;
	}

	template<typename component_t> bool EntityManager::destroyComponent(EntityID entityID) {
		return destroyInstance(entityID, component_t::MANAGER::ID);
	}

	template<typename component_t> bool EntityManager::hasComponent(EntityID entity) const {
		return hasComponent(entity, component_t::Manager::ID);
	}

	template<typename component_t> decltype(std::declval<typename component_t::MANAGER>().component(EntityID())) EntityManager::component(EntityID entity) {
		return componentManager<component_t>().component(entity);
	}

	template<typename component_t> decltype(std::declval<const typename component_t::MANAGER>().component(EntityID())) EntityManager::component(EntityID entity) const {
		return componentManager<component_t>().component(entity);
	}

	template<typename component_t> typename component_t::MANAGER& EntityManager::componentManager() {
		return dynamic_cast<typename component_t::MANAGER&>(componentManager(component_t::MANAGER::ID));
	}

	template<typename component_t> const typename component_t::MANAGER& EntityManager::componentManager() const {
		return dynamic_cast<const typename component_t::MANAGER&>(componentManager(component_t::MANAGER::ID));
	}
}

namespace ake {
	inline Entity::Entity(EntityManager& manager, EntityID id) : m_manager(&manager), m_id(id) {}

	// ////////////////// //
	// // Entity Graph // //
	// ////////////////// //
	inline bool Entity::setParent(EntityID newParent) {
		return m_manager->setEntityParentID(m_id, newParent);
	}

	inline bool Entity::isParentRoot() const {
		return !parentID();
	}

	inline EntityID Entity::parentID() const {
		return m_manager->entityParentID(m_id);
	}

	inline const akc::UnorderedVector<EntityID>& Entity::childrenIDs() const{
		return m_manager->entityChildrenIDs(m_id);
	}

	inline Entity Entity::parent() const {
		auto id = parentID();
		if (!id) throw std::logic_error("Parent of object is root.");
		return Entity(*m_manager, id);
	}

	inline akc::UnorderedVector<Entity> Entity::children() const {
		auto& childVec = childrenIDs();
		akc::UnorderedVector<Entity> result; result.reserve(childVec.size());
		for(auto id : childVec) result.insert(Entity(*m_manager, id));
		return result;
	}

	inline Entity Entity::findFirstChildNamed(const std::string& name) const {
		return Entity(*m_manager, m_manager->entityFindFirstChildNamed(m_id, name));
	}

	inline akc::UnorderedVector<Entity> Entity::findAllChildrenNamed(const std::string& name) const {
		auto childVec = m_manager->entityFindAllChildrenNamed(m_id, name);
		akc::UnorderedVector<Entity> result; result.reserve(childVec.size());
		for(auto id : childVec) result.insert(Entity(*m_manager, id));
		return result;
	}

	// //////////////// //
	// // Components // //
	// //////////////// //
	template<typename component_t, typename... vargs_t> bool Entity::createComponent(const vargs_t&... vargs) {
		return m_manager->createComponent<component_t, vargs_t...>(m_id, vargs...);
	}

	inline bool Entity::destroyComponent(ComponentID componentID) {
		return m_manager->destroyComponent(m_id, componentID);
	}

	template<typename component_t> bool Entity::destroyComponent() {
		return m_manager->destroyComponent<component_t>(m_id);
	}

	inline bool Entity::serializeComponent(akd::PValue& dest, ComponentID componentID) {
		return m_manager->serializeComponent(dest, m_id, componentID);
	}

	inline bool Entity::deserializeComponent(ComponentID componentID, const akd::PValue& src) {
		return m_manager->deserializeComponent(m_id, componentID, src);
	}

	inline bool Entity::hasComponent(ComponentID componentID) const {
		return m_manager->hasComponent(m_id, componentID);
	}

	template<typename component_t> bool Entity::hasComponent() const {
		return m_manager->hasComponent<component_t>(m_id);
	}

	template<typename component_t> decltype(std::declval<typename component_t::MANAGER>().component(EntityID())) Entity::component() {
		return m_manager->component<component_t>(m_id);
	}

	template<typename component_t> decltype(std::declval<const typename component_t::MANAGER>().component(EntityID())) Entity::component() const {
		return m_manager->component<component_t>(m_id);
	}

	inline const akev::DispatcherProxy<EntityParentChangedEvent> Entity::parentChanged() const {
		return m_manager->entityParentChanged(m_id);
	}

	// /////////// //
	// // Other // //
	// /////////// //

	inline const std::string& Entity::name() const {
		return m_manager->entityName(m_id);
	}

	inline EntityID Entity::id() const {
		return m_id;
	}

	inline EntityUID Entity::uid() const {
		return m_manager->entityUID(m_id);
	}
}

#endif
