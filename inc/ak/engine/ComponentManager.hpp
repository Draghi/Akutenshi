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

#ifndef AK_ENGINE_COMPONENTMANAGER_HPP_
#define AK_ENGINE_COMPONENTMANAGER_HPP_

#include <ak/data/PValue.hpp>
#include <ak/engine/Type.hpp>
#include <string_view>

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


			// @todo Can we handle this better?
			virtual void registerHooks() {}

		public:
			ComponentManager() = default;
			virtual ~ComponentManager() = default;

			virtual bool hasComponent(EntityID entityID) const = 0;

			virtual const std::string_view& name() const = 0;
			virtual ComponentID id() const = 0;
	};

}

#endif
