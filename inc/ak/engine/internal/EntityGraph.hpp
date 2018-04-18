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

#ifndef AK_ENGINE_INTERNAL_ENTITYGRAPHMANAGER_HPP_
#define AK_ENGINE_INTERNAL_ENTITYGRAPHMANAGER_HPP_

#include <ak/container/SlotMap.hpp>
#include <ak/container/UnorderedVector.hpp>
#include <ak/engine/Type.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/event/Event.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ake {

	class EntityManager;

	class EventGraphChangedData {
		private:
			EntityID m_oldParent, m_newParent;
			EntityID m_modifiedEntity;

		public:
			EventGraphChangedData(EntityID modifiedEntity, EntityID oldParent, EntityID newParent) : m_oldParent(oldParent), m_newParent(newParent), m_modifiedEntity(modifiedEntity) {}

			EntityID modifiedEntity() { return m_modifiedEntity; }
			EntityID newParent() { return m_newParent; }
			EntityID oldParent() { return m_oldParent; }

			const EntityID modifiedEntity() const { return m_modifiedEntity; }
			const EntityID newParent() const { return m_newParent; }
			const EntityID oldParent() const { return m_oldParent; }
	};
	AK_DEFINE_EVENT(EntityParentChangedEvent, const EventGraphChangedData, false);

	namespace internal {

		class EntityGraphManager final {
			friend ::ake::EntityManager;
			private:
				struct GraphNode {
					EntityID parent;
					akc::UnorderedVector<EntityID> children;
				};

				EntityManager* m_entityManager;

				std::unordered_set<EntityID> m_root;
				akc::SlotMap<GraphNode> m_data;

				mutable akev::Dispatcher<EntityParentChangedEvent> m_generalChangeDispatcher;
				mutable std::unordered_map<EntityID, akev::Dispatcher<EntityParentChangedEvent>> m_specificChangeDispatcher;

				void sendChangeEvent(EntityID modifiedEntity, EntityID oldParent, EntityID newParent);

			public:
				// ////////////// //
				// // Register // //
				// ////////////// //

				bool registerEntity(EntityID entityID, EntityID parent);
				bool deregisterEntity(EntityID entityID);

				// /////////// //
				// // Graph // //
				// /////////// //

				bool setParent(EntityID entityID, EntityID newParent);

				EntityID parent(EntityID entityID) const;
				const akc::UnorderedVector<EntityID>& children(EntityID entityID) const;

				EntityID findFirstNamed(EntityID baseID, const std::string& name) const;
				akc::UnorderedVector<EntityID> findAllNamed(EntityID baseID, const std::string& name) const;

				// /////////// //
				// // Other // //
				// /////////// //

				const std::unordered_set<EntityID>& root() const;

				// /////////// //
				// // Event // //
				// /////////// //

				const akev::DispatcherProxy<EntityParentChangedEvent> entityGraphChanged() const;
				const akev::DispatcherProxy<EntityParentChangedEvent> entityGraphChanged(EntityID entityID) const;
		};

	}
}

#endif
