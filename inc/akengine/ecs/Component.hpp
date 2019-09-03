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

#ifndef AKENGINE_ECS_COMPONENT_HPP_
#define AKENGINE_ECS_COMPONENT_HPP_

#include <akengine/data/PValue.hpp>
#include <akengine/ecs/Entity.hpp>
#include <akengine/ecs/BaseRegistry.hpp>
#include <akengine/ecs/Types.hpp>
#include <stdexcept>

namespace akecs {

	class Component {
		public:
			Component() = default;
			virtual ~Component() = default;

			virtual bool serialize(akd::PValue& /*dst*/) const { return true; }
			virtual bool deserialize(const akd::PValue& /*src*/) { return true; }
	};


	class ComponentRef final {
		template<typename... components_t> friend class Registry;
		private:
			EntityRef m_ref;
			ComponentTypeUID m_componentTypeUID;
			ComponentTypeID m_componentTypeID;

			ComponentRef(EntityRef ref, ComponentTypeUID typeUID, ComponentTypeID typeID) : m_ref(ref), m_componentTypeUID(typeUID), m_componentTypeID(typeID) {}

		public:
				  Component* get()       { return !m_ref.valid() ? static_cast<Component*>(nullptr) : m_ref.m_registry->componentByID(m_ref, m_componentTypeID); }
			const Component* get() const { return !m_ref.valid() ? static_cast<Component*>(nullptr) : m_ref.m_registry->componentByID(m_ref, m_componentTypeID); }

			template<typename component_t>       component_t* get()       { return (!m_ref.valid() || m_componentTypeID == 0) ? static_cast<component_t*>(nullptr) : dynamic_cast<component_t*>(m_ref.m_registry->componentByID(m_ref, m_componentTypeID - 1)); }
			template<typename component_t> const component_t* get() const { return (!m_ref.valid() || m_componentTypeID == 0) ? static_cast<component_t*>(nullptr) : dynamic_cast<component_t*>(m_ref.m_registry->componentByID(m_ref, m_componentTypeID - 1)); }

			ComponentTypeUID uid() const { return m_componentTypeUID; }

			bool valid() const { return get() != nullptr; }
			operator bool() const { return valid(); }

			      Component& operator*()       { auto* component = get(); if (!component) throw std::logic_error("Entity does not exist."); return *component; }
			const Component& operator*() const { auto* component = get(); if (!component) throw std::logic_error("Entity does not exist."); return *component; }

                  Component* operator->()       { return get(); }
			const Component* operator->() const { return get(); }

			      EntityRef entityRef()       { return m_ref; }
			const EntityRef entityRef() const { return m_ref; }
	};

}

#endif /* AKENGINE_ECS_COMPONENT_HPP_ */
