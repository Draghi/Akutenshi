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

#ifndef AKENGINE_ECS_ENTITY_HPP_
#define AKENGINE_ECS_ENTITY_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/ecs/Component.hpp>
#include <akengine/ecs/BaseRegistry.hpp>
#include <akengine/ecs/Types.hpp>
#include <stdexcept>
#include <unordered_map>

namespace akecs {

	class EntityRef final {
		friend class Entity;
		friend class ComponentRef;
		template<typename... components_t> friend class Registry;
		private:
			BaseRegistry* m_registry;
			EntityID m_id;

		public:
			EntityRef() : m_registry(nullptr), m_id() {}
			EntityRef(BaseRegistry& registry, EntityID entityID) : m_registry(&registry), m_id(entityID) {}
			EntityRef(const EntityRef& other) : m_registry(other.m_registry), m_id(other.m_id) {}

			      Entity* get()       { return !m_registry ? static_cast<Entity*>(nullptr) : m_registry->entity(m_id); }
			const Entity* get() const { return !m_registry ? static_cast<Entity*>(nullptr) : m_registry->entity(m_id); }

			EntityID id() const { return m_id; }

			bool valid() const { return get() != nullptr; }
			operator bool() const { return valid(); }

			      Entity& operator*()       { auto* entity = get(); if (!entity) throw std::logic_error("Entity does not exist."); return *entity; }
			const Entity& operator*() const { auto* entity = get(); if (!entity) throw std::logic_error("Entity does not exist."); return *entity; }

			      Entity* operator->()       { return get(); }
			const Entity* operator->() const { return get(); }

			EntityRef& operator=(const EntityRef& other) { m_registry = other.m_registry; m_id = other.m_id; return *this; }

			      BaseRegistry& registry()       { return *m_registry; }
			const BaseRegistry& registry() const { return *m_registry; }
	};

	class Entity final {
		template<typename... components_t> friend class Registry;
		private:
			EntityRef m_ref;
			std::unordered_map<ComponentTypeID, akSize> m_components;

			Entity(BaseRegistry& registry, EntityID id) : m_ref(registry, id) {}

		public:

			// template<typename component_t, typename... vargs_t> component_t* attach(vargs_t&&... vargs) { return m_ref.m_registry->template attach<component_t>(*this, std::forward<vargs_t>(vargs)...); }
			// template<typename... vargs_t> Component* attach(ComponentTypeUID typeUID, vargs_t&&... vargs) { return m_ref.m_registry->template attach(*this, typeUID, std::forward<vargs_t>(vargs)...); }
			Component* attach(ComponentTypeUID typeUID) { return m_ref.m_registry->attach(m_ref, typeUID); }

			// template<typename component_t> bool detach() { return m_ref.m_registry->template detach<component_t>(*this); }
			bool detach(ComponentTypeUID typeUID) { return m_ref.m_registry->detach(m_ref, typeUID); }

			// template<typename component_t>       component_t* component()       { return m_ref.m_registry->template component<component_t>(*this); }
			// template<typename component_t> const component_t* component() const { return m_ref.m_registry->template component<component_t>(*this); }

			      EntityRef ref()       { return m_ref; }
			const EntityRef ref() const { return m_ref; }

			EntityID id() const { return m_ref.id(); }

			      BaseRegistry& registry()       { return m_ref.registry(); }
			const BaseRegistry& registry() const { return m_ref.registry(); }

			operator EntityRef() { return m_ref; }
			operator const EntityRef() const { return m_ref; }
	};



}

#endif /* AKENGINE_ECS_ENTITY_HPP_ */
