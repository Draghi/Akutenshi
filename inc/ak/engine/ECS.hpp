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
#include <ak/data/Hash.hpp>
#include <ak/Iter.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ake {
	using EntityUID = uint64;
	using EntityID = akc::SlotID_t<4>;
	using ComponentID = uint32;
}

#define AKE_DEFINE_COMPONENT_MANAGER(componentManagerType, componentType) \
	private: friend class ::ake::EntityManager; \
	public:  using MANAGER  = componentManagerType; \
	public:  using COMPONENT = componentType; \
	public:  constexpr static ::std::string_view NAME = std::string_view(#componentManagerType, sizeof(#componentManagerType) - 1); \
	public:  constexpr static ::ake::ComponentID ID = akd::hash32FNV1A<char>(#componentManagerType, sizeof(#componentManagerType) - 1); \
	public:  const ::std::string_view& name() const override { return NAME; } \
	public:  ::ake::ComponentID id() const override { return ID; }

#define AKE_DEFINE_COMPONENT(componentManagerType, componentType) \
	public:  using MANAGER  = componentManagerType; \
	public:  using COMPONENT = componentType;

namespace ake {

	class EntityManager;

	class ComponentManager {
		friend ::ake::EntityManager;
		private:
			EntityManager* m_entityManager;

		protected:
			EntityManager& entityManager() { return *m_entityManager; }
			const EntityManager& entityManager() const { return *m_entityManager; }

			// bool createInstance(EntityID entityID, ...args);
			virtual bool destroyComponent(EntityID entityID) = 0;

			virtual bool serializeComponent(akd::PValue& /*dest*/, ake::EntityID /*entityID*/) { return false; }
			virtual bool deserializeComponent(ake::EntityID /*entityID*/, const akd::PValue& /*src*/) { return false; }

		public:
			ComponentManager() = default;
			virtual ~ComponentManager() = default;

			virtual bool hasComponent(EntityID entityID) const = 0;

			virtual const std::string_view& name() const = 0;
			virtual ComponentID id() const = 0;
	};

}

namespace ake {

	using EntityUIDGenerator_f = EntityUID(EntityManager&);

	class EntityManager final {
		EntityManager(const EntityManager&) = delete;
		EntityManager& operator=(const EntityManager&) = delete;
		private:
			// Component Storage
			std::unordered_map<ComponentID, std::unique_ptr<ComponentManager>> m_components;

			// Entity UID Generation
			std::function<EntityUIDGenerator_f> m_entityUIDGenerator;


			// Entity Name Storage
			akc::SlotMap<std::string> m_nameStorage;
			akc::SlotMap<akc::SlotID> m_entityNameID;
			std::unordered_map<std::string, std::pair<akc::SlotID, akc::UnorderedVector<EntityID>>> m_lookupEntitiesByName;

			// Entity Component Storage
			akc::SlotMap<std::unordered_set<ComponentID>> m_entityComponentIDs;

			// Entity UID
			akc::SlotMap<EntityUID> m_entityUID;
			std::unordered_map<EntityUID, EntityID> m_lookupEntityByUID;

			EntityUID nextEntityUID();
			EntityID newEntity(EntityUID targetUID, akc::SlotID nameID);

		public:
			EntityManager(std::function<EntityUIDGenerator_f> entityUIDGenerator);
			EntityManager(EntityManager&& other);

			EntityManager& operator=(EntityManager&& other);


			// ////////////// //
			// // Entities // //
			// ////////////// //
			EntityID newEntity(EntityUID targetUID, const std::string& name);
			EntityID newEntity(const std::string& name);

			bool deleteEntity(EntityID entityID);

			std::string entityName(EntityID entityID) const;
			std::unordered_set<ComponentID> entityComponentIDs(EntityID entityID) const;

			// ////////////// //
			// // Instance // //
			// ////////////// //
			template<typename component_t, typename... vargs_t> bool createComponent(EntityID entityID, const vargs_t&... vargs);

			bool destroyComponent(EntityID entityID, ComponentID componentID);
			template<typename component_t> bool destroyComponent(EntityID entityID);

			bool serializeComponent(akd::PValue& dest, EntityID entityID, ComponentID componentID);
			bool deserializeComponent(EntityID entityID, ComponentID componentID, const akd::PValue& src);

			bool hasComponent(EntityID entityID, ComponentID componentID) const;
			template<typename component_t> bool hasComponent(EntityID entity) const;

			template<typename component_t> decltype(std::declval<typename component_t::MANAGER>().component(EntityID())) component(EntityID entity);
			template<typename component_t> decltype(std::declval<const typename component_t::MANAGER>().component(EntityID())) component(EntityID entity) const;

			// //////////////// //
			// // Components // //
			// //////////////// //

			bool registerComponentManager(std::unique_ptr<ComponentManager>&& component);
			bool componentManagerExists(ComponentID componentID);

			ComponentManager& componentManager(ComponentID componentID);
			template<typename component_t> typename component_t::MANAGER& componentManager();

			const ComponentManager& componentManager(ComponentID componentID) const;
			template<typename component_t> const typename component_t::MANAGER& componentManager() const;
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
