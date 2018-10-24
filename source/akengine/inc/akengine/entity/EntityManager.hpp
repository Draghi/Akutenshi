/**
 * Copyright 2018 Michael J. Baker
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

#include <akcommon/SlotMap.hpp>
#include <akengine/entity/ComponentManager.hpp>
#include <akengine/entity/internal/EntityGraph.hpp>
#include <akengine/entity/internal/NameStorage.hpp>
#include <akengine/entity/Type.hpp>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>


namespace akd {
	class PValue;
}

namespace ake {

	using EntityUIDGenerator_f = EntityUID(EntityManager&);

	class Scene;
	class Entity;

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

			//internal::EntityGraphManager m_entityGraph;
			EntityGraph m_entityGraph;

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
			const Entity getEntity(EntityID entityID) const;

			const std::string& entityName(EntityID entityID) const;
			std::unordered_set<ComponentID> entityComponents(EntityID entityID) const;
			EntityUID entityUID(EntityID entityID) const;

			EntityGraph& entityGraph();
			const EntityGraph& entityGraph() const;

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

			//const akev::DispatcherProxy<EntityParentChangedEvent> entityParentChanged() const;
			//const akev::DispatcherProxy<EntityParentChangedEvent> entityParentChanged(EntityID entityID) const;

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

#endif
