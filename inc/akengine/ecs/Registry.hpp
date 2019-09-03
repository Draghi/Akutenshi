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

#ifndef AKENGINE_ECS_REGISTRY_HPP_
#define AKENGINE_ECS_REGISTRY_HPP_

#include <akcommon/Meta.hpp>
#include <akcommon/ObjectPool.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/SlotMap.hpp>
#include <akcommon/Traits.hpp>
#include <akengine/debug/Log.hpp>
#include <akengine/ecs/BaseRegistry.hpp>
#include <akengine/ecs/Component.hpp>
#include <akengine/ecs/Entity.hpp>
#include <akengine/ecs/Types.hpp>
#include <crtdefs.h>
#include <array>
#include <initializer_list>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace akecs {

	template<typename... components_t> class Registry final : public BaseRegistry {
		static_assert(akc::traits::IsUniqueList<ComponentTypeUID, components_t::COMPONENT_UID...>::value, "Component types do not have a unique IDs.");
		private:
			static constexpr ComponentTypeID COMPONENT_TYPE_COUNT = sizeof...(components_t);

			// ////////////////////////// //
			// // Component UID Lookup // //
			// ////////////////////////// //
			static const std::unordered_map<ComponentTypeUID, ComponentTypeID> lookupUIDToID;
			static ComponentTypeID tryComponentTypeID(ComponentTypeUID componentUID);

			// //////////////////// //
			// // Entity Storage // //
			// //////////////////// //
			akc::SlotMap<Entity> m_entities;

			// /////////////////////// //
			// // Component Storage // //
			// /////////////////////// //
			template<typename component_t> struct ComponentType {
				using component_type = component_t;
				akc::ObjectPool<component_t> components;
			};

			using component_storage = typename std::aligned_union<0, ComponentType<components_t>...>::type;
			std::array<component_storage, COMPONENT_TYPE_COUNT> m_componentTypes;

			template<typename func_t> auto withComponentStorage(size_t id, const func_t& func);
			template<typename func_t> auto withComponentStorage(size_t id, const func_t& func) const;

			template<typename component_t> static constexpr ComponentTypeID componentTypeID();
			template<typename component_t> ComponentType<component_t>& componentType();

			template<typename component_t, typename... vargs_t> static Component* tryAttach(typename std::enable_if< std::is_constructible<component_t, Registry&, EntityRef, vargs_t...>::value>::type*, Registry& registry, EntityRef entity, vargs_t&&... vargs);
			template<typename component_t, typename... vargs_t> static Component* tryAttach(typename std::enable_if<!std::is_constructible<component_t, Registry&, EntityRef, vargs_t...>::value>::type*, Registry&, EntityRef, vargs_t&&...);

		protected:
			bool detachByID(EntityRef entity, ComponentTypeID typeID) override;

				  Component* componentByID(EntityRef entity,       ComponentTypeID typeUID)       override;
			const Component* componentByID(EntityRef entity, const ComponentTypeID typeUID) const override;

		public:
			Registry();
			~Registry() override;

			// //////////// //
			// // Entity // //
			// //////////// //

			//======//
			// Base //
			//======//

			EntityRef create() override;

			void destroy(EntityRef entity) override;

			      Entity* entity(EntityID uid)       override;
			const Entity* entity(EntityID uid) const override;

			      EntityRef entityRef(EntityID id)       override;
			const EntityRef entityRef(EntityID id) const override;

			void reserveEntities(akSize count) override;

			// //////////////// //
			// // Components // //
			// //////////////// //

			//======//
			// Base //
			//======//

			Component* attach(EntityRef entity, ComponentTypeUID typeUID) override;

			bool detach(EntityRef entity, ComponentTypeUID typeUID) override;

				  Component* component(EntityRef entity, ComponentTypeUID typeUID)       override;
			const Component* component(EntityRef entity, ComponentTypeUID typeUID) const override;

				  ComponentRef componentRef(EntityRef entity, ComponentTypeUID typeUID)       override;
			const ComponentRef componentRef(EntityRef entity, ComponentTypeUID typeUID) const override;

			//======//
			// Util //
			//======//

			template<typename component_t, typename... vargs_t> component_t* attach(EntityRef entity, vargs_t&&... vargs);
			template<typename... vargs_t> Component* attach(EntityRef entity, ComponentTypeUID typeUID, vargs_t&&... vargs);

			template<typename component_t> bool detach(EntityRef entity);

			template<typename component_t>       component_t* component(EntityRef entity);
			template<typename component_t> const component_t* component(EntityRef entity) const;
	};
}

namespace akecs {
	template<typename... components_t> Registry<components_t...>::Registry() {
		 // Placement new component type managers, initializer list facilitates for-each
		(void) std::initializer_list<int>{((new(&m_componentTypes.at(componentTypeID<components_t>())) ComponentType<components_t>()), 0)...};
	 }

	template<typename... components_t> Registry<components_t...>::~Registry() {
		// Placement delete component type managers, initializer list facilitates for-each
		(void) std::initializer_list<int>{((reinterpret_cast<ComponentType<components_t>&>(m_componentTypes[componentTypeID<components_t>()]).~ComponentType()), 0)...};
	}
}

namespace akecs {

	template<typename... components_t> EntityRef Registry<components_t...>::create() {
		auto entityID = m_entities.insert(Entity(*this, EntityID()));
		auto& entity = m_entities[entityID];
		entity.m_ref.m_id = entityID;
		return entity.ref();
	}

	template<typename... components_t> void Registry<components_t...>::destroy(EntityRef entity) {
		if (entity.get() == nullptr) return;
		for(auto& componentEntry : entity->m_components) detachByID(entity, componentEntry.first);
		m_entities.erase(entity.m_id);
	}

}

namespace akecs {

	template<typename... components_t> Entity* Registry<components_t...>::entity(EntityID id) {
		Entity& result = m_entities[id];
		return result.id() == id ? &result : nullptr;
	}

	template<typename... components_t> const Entity* Registry<components_t...>::entity(EntityID id) const {
		const Entity& result = m_entities[id];
		return result.id() == id ? &result : nullptr;
	}

	template<typename... components_t> EntityRef Registry<components_t...>::entityRef(EntityID uid) {
		auto ent = entity(uid);
		return ent ? ent->ref() : EntityRef();
	}

	template<typename... components_t> const EntityRef Registry<components_t...>::entityRef(EntityID uid) const {
		auto ent = entity(uid);
		return ent ? ent->ref() : EntityRef();
	}

	template<typename... components_t> void Registry<components_t...>::reserveEntities(akSize count) {
		m_entities.reserve(count);
	}
}

namespace akecs {

	template<typename... components_t> template<typename component_t, typename... vargs_t> component_t* Registry<components_t...>::attach(EntityRef entity, vargs_t&&... vargs) {
		if (entity->m_components.find(componentTypeID<component_t>()) != entity->m_components.end()) return nullptr;
		try {
			auto componentOffset = componentType<component_t>().components.emplace(*this, entity, std::forward<vargs_t>(vargs)...);
			entity->m_components.emplace(componentTypeID<component_t>(), componentOffset);
			return &componentType<component_t>().components[componentOffset];
		} catch(const ComponentConstraintViolation& e) {
			akl::Logger("Registry").error("Failed to construct and attach component to entity due to a constraint violation:\n", e.what());
			return nullptr;
		}
	}

	template<typename... components_t> template<typename... vargs_t> Component* Registry<components_t...>::attach(EntityRef entity, ComponentTypeUID typeUID, vargs_t&&... vargs) {
		auto typeID = tryComponentTypeID(typeUID);
		if (typeID <= 0) return nullptr;
		return *withComponentStorage(typeID - 1, [&](auto& storage){
			using storage_type = typename std::remove_reference<decltype(storage)>::type;
			return tryAttach<typename storage_type::component_type>(nullptr, *this, entity, std::forward<vargs_t>(vargs)...); //static_cast<Component*>(attach<typename storage_type::component_type, vargs_t...>(entity, vargs...));
		});
	}

	template<typename... components_t> Component* Registry<components_t...>::attach(EntityRef entity, ComponentTypeUID typeUID) {
		auto typeID = tryComponentTypeID(typeUID);
		if (typeID <= 0) return nullptr;
		return *withComponentStorage(typeID - 1, [&](auto& storage){
			using storage_type = typename std::remove_reference<decltype(storage)>::type;
			return tryAttach<typename storage_type::component_type>(nullptr, *this, entity); //static_cast<Component*>(attach<typename storage_type::component_type, vargs_t...>(entity, vargs...));
		});
	}

	template<typename... components_t> template<typename component_t> bool Registry<components_t...>::detach(EntityRef entity) {
		auto iter = entity->m_components.find(componentTypeID<component_t>());
		if (iter == entity->m_components.end()) return true;
		auto result = componentType<component_t>().components.erase(iter->second);
		if (result) entity->m_components.erase(iter);
		return result;
	}

	template<typename... components_t> bool Registry<components_t...>::detach(EntityRef entity, ComponentTypeUID typeUID) {
		auto typeID = tryComponentTypeID(typeUID);
		if (typeID <= 0) return true;
		return detachByID(entity, typeID - 1);
	}

}

namespace akecs {

	template<typename... components_t> template<typename component_t> component_t* Registry<components_t...>::component(EntityRef entity) {
		if (entity->m_components.find(componentTypeID<component_t>()) == entity->m_components.end()) return nullptr;
		return &componentType<component_t>().components.at(entity->m_components.at(componentTypeID<component_t>()));
	}

	template<typename... components_t> template<typename component_t> const component_t* Registry<components_t...>::component(EntityRef entity) const {
		if (entity->m_components.find(componentTypeID<component_t>()) == entity->m_components.end()) return nullptr;
		return &componentType<component_t>().components.at(entity->m_components.at(componentTypeID<component_t>()));
	}

	template<typename... components_t> Component* Registry<components_t...>::component(EntityRef entity, ComponentTypeUID typeUID) {
		auto typeID = tryComponentTypeID(typeUID);
		if ((typeID <= 0) || (entity->m_components.find(typeID - 1) == entity->m_components.end())) return nullptr;
		return componentByID(entity, typeID - 1);
	}

	template<typename... components_t> const Component* Registry<components_t...>::component(EntityRef entity, ComponentTypeUID typeUID) const {
		auto typeID = tryComponentTypeID(typeUID);
		if ((typeID <= 0) || (entity->m_components.find(typeID - 1) == entity->m_components.end())) return nullptr;
		return componentByID(entity, typeID - 1);
	}

	template<typename... components_t> ComponentRef Registry<components_t...>::componentRef(EntityRef entity, ComponentTypeUID typeUID) {
		return ComponentRef(entity, typeUID, tryComponentTypeID(typeUID));
	}

	template<typename... components_t> const ComponentRef Registry<components_t...>::componentRef(EntityRef entity, ComponentTypeUID typeUID) const {
		return ComponentRef(entity, typeUID, tryComponentTypeID(typeUID));
	}

}

namespace akecs {

	template<typename... components_t> inline const std::unordered_map<ComponentTypeUID, ComponentTypeID> Registry<components_t...>::lookupUIDToID = {
		std::pair<ComponentTypeUID, ComponentTypeID>{components_t::COMPONENT_UID, componentTypeID<components_t>()}...
	};

	template<typename... components_t> ComponentTypeID Registry<components_t...>::tryComponentTypeID(ComponentTypeUID componentUID) {
		auto iter = lookupUIDToID.find(componentUID);
		return iter != lookupUIDToID.cend() ? iter->second + 1 : ComponentTypeID();
	}

}

namespace akecs {

	template<typename... components_t> template<typename func_t> auto Registry<components_t...>::withComponentStorage(size_t id, const func_t& func) {
		return akc::vardicSwitch(id, func, reinterpret_cast<ComponentType<components_t>&>(m_componentTypes[componentTypeID<components_t>()])...);
	}

	template<typename... components_t> template<typename func_t> auto Registry<components_t...>::withComponentStorage(size_t id, const func_t& func) const {
		return akc::vardicSwitch(id, func, reinterpret_cast<const ComponentType<components_t>&>(m_componentTypes[componentTypeID<components_t>()])...);
	}


	template<typename... components_t> template<typename componentType_t> constexpr ComponentTypeID Registry<components_t...>::componentTypeID() {
		return akc::traits::VargIndexer<componentType_t, components_t...>::value;
	}

	template<typename... components_t> template<typename component_t> typename Registry<components_t...>::template ComponentType<component_t>& Registry<components_t...>::componentType() {
		return reinterpret_cast<ComponentType<component_t>&>(m_componentTypes[componentTypeID<component_t>()]);
	}

}

namespace akecs {

	template<typename... components_t> template<typename component_t, typename... vargs_t> Component* Registry<components_t...>::tryAttach(typename std::enable_if< std::is_constructible<component_t, Registry&, EntityRef, vargs_t...>::value>::type*, Registry& registry, EntityRef entity, vargs_t&&... vargs) {
		return registry.template attach<component_t>(entity, std::forward<vargs_t>(vargs)...);
	}

	template<typename... components_t> template<typename component_t, typename... vargs_t> Component* Registry<components_t...>::tryAttach(typename std::enable_if<!std::is_constructible<component_t, Registry&, EntityRef, vargs_t...>::value>::type*, Registry&, EntityRef, vargs_t&&...) {
		throw std::logic_error("Attempting to construct component with no matching constructor.");
	}

	template<typename... components_t> bool Registry<components_t...>::detachByID(EntityRef entity, ComponentTypeID typeID) {
		return *withComponentStorage(typeID, [&](auto& storage) { using storage_type = typename std::remove_reference<decltype(storage)>::type; return detach<typename storage_type::component_type>(entity); });
	}

	template<typename... components_t> Component* Registry<components_t...>::componentByID(EntityRef entity, ComponentTypeID typeID) {
		return *withComponentStorage(typeID, [&](auto& storage) { return static_cast<Component*>(&storage.components.at(entity->m_components.at(typeID))); });
	}

	template<typename... components_t> const Component* Registry<components_t...>::componentByID(const EntityRef entity, ComponentTypeID typeID) const {
		return *withComponentStorage(typeID, [&](auto& storage) { return static_cast<const Component*>(&storage.components.at(entity->m_components.at(typeID))); });
	}
}

#endif /* AKENGINE_ECS_REGISTRY_HPP_ */
