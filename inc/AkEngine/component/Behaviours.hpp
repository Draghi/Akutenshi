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

#ifndef AK_ENGINE_COMPONENTS_BEHAVIOURS_HPP_
#define AK_ENGINE_COMPONENTS_BEHAVIOURS_HPP_

#include <AkCommon/PrimitiveTypes.hpp>
#include <AkCommon/SlotMap.hpp>
#include <AkEngine/entity/ComponentManager.hpp>
#include <AkEngine/entity/Type.hpp>
#include <memory>
#include <string>

namespace ake {
	class EntityManager;
	class Transform;
} /* namespace ake */

namespace ake {

	class BehavioursManager;
	class Behaviours;

	using BehaviourID = akc::SlotID;
	class Behaviours;

	class Entity;

	class Behaviour {
		friend Behaviours;
		private:
			Behaviours* m_owner;
			BehaviourID m_behaviourID;
			std::string m_name;

			bool m_isActive = true;

		protected:
			virtual void onStart();
			virtual void onUpdate(fpSingle deltaTime);

			Entity entity();
			const Entity entity() const;

		public:
			Behaviour(const std::string& name);
			virtual ~Behaviour();

			void setActive(bool active);
			bool isActive() const;

			BehaviourID id() const;
			const std::string& name() const;

			Behaviours& owner();
			const Behaviours& owner() const;
	};

	class Behaviours final {
		friend BehavioursManager;
		AKE_DEFINE_COMPONENT(BehavioursManager, Behaviours)
		private:
			EntityManager* m_entityManager;
			BehavioursManager* m_behavioursManager;
			EntityID m_id;

			akc::SlotMap<std::unique_ptr<Behaviour>> m_behaviours;

			bool m_isActive = true;

		protected:
			void onUpdate(fpSingle deltaTime);

		public:
			Behaviours(EntityManager& entityManager, BehavioursManager& behavioursManager, EntityID id);

			BehaviourID addBehaviour(std::unique_ptr<Behaviour>&& behaviour);

			Behaviour& getBehaviour(BehaviourID behaviourID);
			const Behaviour& getBehaviour(BehaviourID behaviourID) const;

			bool removeBehaviour(BehaviourID behaviourID);

			BehavioursManager& behavioursManager();
			const BehavioursManager& behavioursManager() const;

			EntityManager& entityManager();
			const EntityManager& entityManager() const;

			EntityID id() const;

			void setActive(bool active);
			bool isActive() const;
	};

	class BehavioursManager final : public ComponentManager {
		AKE_DEFINE_COMPONENT_MANAGER(BehavioursManager, Behaviours)
		private:
			std::unordered_map<EntityID, Behaviours> m_behaviours;

		protected:
			bool createComponent(EntityID entityID);
			bool destroyComponent(EntityID entityID) override;

			void registerHooks() override;

		public:
			BehavioursManager();

			Behaviours& component(EntityID entityID);
			const Behaviours& component(EntityID entityID) const;

			bool hasComponent(EntityID entityID) const override;
	};
}



#endif
