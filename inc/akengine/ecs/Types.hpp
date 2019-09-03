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

#ifndef AKENGINE_ECS_TYPES_HPP_
#define AKENGINE_ECS_TYPES_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/SlotMap.hpp>
#include <stdexcept>

namespace akecs {

	using  ComponentTypeID = uint32;
	using ComponentTypeUID = uint32;

	using  EntityID = akc::SlotID;

	class BaseRegistry;
	template<typename... components_t> class Registry;

	class Entity;
	class EntityRef;

	class Component;
	class ComponentRef;

	class ComponentConstraintViolation : public std::logic_error {
		public:
			using std::logic_error::logic_error;
	};

}

#endif /* AKENGINE_ECS_TYPES_HPP_ */
