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

#include <akcommon/Memory.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/Traits.hpp>
#include <array>
#include <memory>
#include <unordered_map>

namespace akecs {

	class ComponentType;

	using ComponentTypeID  = uint32;
	using ComponentTypeUID = uint32;

	template<typename... componentTypes_t> class Registry final {
		static_assert(akc::traits::IsUniqueList<ComponentTypeUID, componentTypes_t::COMPONENT_TYPE_UID...>::value, "Component types do not have a unique IDs.");
		private:
			// ////////////////////////////// //
			// // ComponentType UID Lookup // //
			// ////////////////////////////// //
			static const std::unordered_map<ComponentTypeUID, ComponentTypeID> uidMap;

			static ComponentTypeID getComponentTypeID(ComponentTypeUID componentUID);

			// ///////////////////////////// //
			// // ComponentType ID Lookup // //
			// ///////////////////////////// //
			std::array<std::unique_ptr<ComponentType>, sizeof...(componentTypes_t)> m_componentTypes{{akc::make_unique<ComponentType, componentTypes_t>()...}};

			template<typename componentType_t> static constexpr ComponentTypeID componentTypeID();

		public:
			Registry() {}

	};


}

namespace akecs {

	template<typename... componentTypes_t> template<typename componentType_t> constexpr ComponentTypeID Registry<componentTypes_t...>::componentTypeID() {
		return akc::traits::VargIndexer<componentType_t, componentTypes_t...>::value;
	}

	template<typename... componentTypes_t> ComponentTypeID Registry<componentTypes_t...>::getComponentTypeID(ComponentTypeUID componentUID) {
		auto iter = uidMap.find(componentUID);
		return iter != uidMap.cend() ? iter->second : ComponentTypeID();
	}

	template<typename... componentTypes_t> inline const std::unordered_map<ComponentTypeUID, ComponentTypeID> Registry<componentTypes_t...>::uidMap = {
		std::pair<ComponentTypeUID, ComponentTypeID>{componentTypes_t::COMPONENT_TYPE_UID, componentTypeID<componentTypes_t>()}...
	};
}

#endif /* AKENGINE_ECS_REGISTRY_HPP_ */
