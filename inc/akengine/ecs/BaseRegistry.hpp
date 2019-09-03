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

#ifndef AKENGINE_ECS_BASEREGISTRY_HPP_
#define AKENGINE_ECS_BASEREGISTRY_HPP_

#include <akengine/ecs/Types.hpp>

namespace akecs {

	class BaseRegistry {
		template<typename... components_t> friend class Registry;
		friend class ComponentRef;
		friend class EntityRef;
		private:
			BaseRegistry() = default;
			virtual ~BaseRegistry() = default;

		protected:
			virtual bool detachByID(EntityRef entity, ComponentTypeID typeID) = 0;

			virtual	      Component* componentByID(EntityRef entity,       ComponentTypeID typeID) = 0;
			virtual const Component* componentByID(EntityRef entity, const ComponentTypeID typeID) const = 0;

		public:

			// //////////// //
			// // Entity // //
			// //////////// //

			virtual EntityRef create() = 0;

			virtual void destroy(EntityRef entity) = 0;

			virtual       Entity* entity(EntityID id) = 0;
			virtual const Entity* entity(EntityID id) const = 0;

			virtual       EntityRef entityRef(EntityID id) = 0;
			virtual const EntityRef entityRef(EntityID id) const = 0;

			virtual void reserveEntities(akSize count) = 0;

			// //////////////// //
			// // Components // //
			// //////////////// //

			virtual Component* attach(EntityRef entity, ComponentTypeUID typeUID) = 0;

			virtual bool detach(EntityRef entity, ComponentTypeUID typeUID) = 0;

			virtual       Component* component(EntityRef entity, ComponentTypeUID typeUID) = 0;
			virtual const Component* component(EntityRef entity, ComponentTypeUID typeUID) const = 0;

			virtual       ComponentRef componentRef(EntityRef entity, ComponentTypeUID typeUID) = 0;
			virtual const ComponentRef componentRef(EntityRef entity, ComponentTypeUID typeUID) const = 0;
	};

}



#endif /* AKENGINE_ECS_BASEREGISTRY_HPP_ */
