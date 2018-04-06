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

#ifndef AK_ENGINE_COMPONENTS_SCENEGRAPH_HPP_
#define AK_ENGINE_COMPONENTS_SCENEGRAPH_HPP_

#include <ak/container/SlotMap.hpp>
#include <ak/container/UnorderedVector.hpp>
#include <ak/engine/ECS.hpp>
#include <ak/event/Event.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/Iter.hpp>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace ake {

	class SceneGraphManager;

	class SceneGraph final {
		AKE_DEFINE_COMPONENT(SceneGraphManager, SceneGraph)
		private:
			SceneGraphManager* m_manager;
			EntityID m_id;

		public:
			SceneGraph(SceneGraphManager& manager, EntityID id);

			bool setParent(EntityID newParent);

			SceneGraph parent() const;
			akc::UnorderedVector<SceneGraph> children() const;

			SceneGraph findFirstNamed(EntityID baseID, const std::string& name);
			akc::UnorderedVector<SceneGraph> findAllNamed(EntityID baseID, const std::string& name);

			const SceneGraph findFirstNamed(EntityID baseID, const std::string& name) const;
			const akc::UnorderedVector<SceneGraph> findAllNamed(EntityID baseID, const std::string& name) const;

			EntityID id() const;
	};

	class SceneGraphChangeEventData {
		private:
			SceneGraph m_oldParent, m_newParent;
			SceneGraph m_modifiedEntity;

		public:
			SceneGraphChangeEventData(SceneGraph modifiedEntity, SceneGraph oldParent, SceneGraph newParent) : m_oldParent(oldParent), m_newParent(newParent), m_modifiedEntity(modifiedEntity) {}

			SceneGraph modifiedEntity() { return m_modifiedEntity; }
			SceneGraph newParent() { return m_newParent; }
			SceneGraph oldParent() { return m_oldParent; }

			const SceneGraph modifiedEntity() const { return m_modifiedEntity; }
			const SceneGraph newParent() const { return m_newParent; }
			const SceneGraph oldParent() const { return m_oldParent; }
	};
	AK_DEFINE_EVENT(SceneGraphChangeEvent, const SceneGraphChangeEventData, false);

	class SceneGraphManager final : public ComponentManager {
		AKE_DEFINE_COMPONENT_MANAGER(SceneGraphManager, SceneGraph)
		private:
			struct GraphNode {
				EntityID parent;
				akc::UnorderedVector<EntityID> children;
			};

			// If speed/cache coherancy becomes a concern:
			// - Add slotmap to store data
			// - Store EntityID <-> SlotID mapping in map
			// - Add second reverse lookup map
			// - Change component to store SlotID instead
			// - Change GraphNode to store SlotID instead

			std::unordered_set<EntityID> m_root;
			std::unordered_map<EntityID, GraphNode> m_data;

			mutable akev::Dispatcher<SceneGraphChangeEvent> m_generalChangeDispatcher;
			mutable std::unordered_map<EntityID, akev::Dispatcher<SceneGraphChangeEvent>> m_specificChangeDispatcher;

			void sendChangeEvent(EntityID modifiedEntity, EntityID oldParent, EntityID newParent) {
				SceneGraphChangeEvent event(SceneGraphChangeEventData(component(modifiedEntity), component(oldParent), component(newParent)));
				m_generalChangeDispatcher.send(event);
				auto specificIter = m_specificChangeDispatcher.find(modifiedEntity);
				if (specificIter != m_specificChangeDispatcher.end()) specificIter->second.send(event);
			}

		protected:
			bool createComponent(EntityID entityID, EntityID parent = EntityID()) {
				std::unordered_map<EntityID, GraphNode>::iterator parentIter;
				if ((parent) && ((parentIter = m_data.find(parent)) == m_data.end())) return false;
				if (!m_data.emplace(entityID, GraphNode{parent, {}}).second) return false;
				if (parent) parentIter->second.children.insert(entityID);
				else m_root.insert(entityID);
				return true;
			}

			bool destroyComponent(EntityID entityID) override {
				auto iter = m_data.find(entityID);
				if (iter == m_data.end()) throw std::logic_error("ake::SceneGraphComponnent: Data corruption, tried to delete non-existent instance.");
				for(auto childID : iter->second.children) entityManager().deleteEntity(childID);
				m_data.erase(iter);
				m_specificChangeDispatcher.erase(entityID);
				m_root.erase(entityID);
				return true;
			}

			//bool serializeInstance(akd::PValue& dest, ake::EntityID entityID) override {}
			//bool deserializeInstance(ake::EntityID entityID, const akd::PValue& src) override {}

		public:

			bool setParent(EntityID entityID, EntityID newParent) {
				// Validation
				auto iter = m_data.find(entityID);
				if (iter == m_data.end()) throw std::out_of_range("ake::SceneGraphComponent: Entity does not have SceneGraph component.");
				if (iter->second.parent == newParent) return true;

				// Cycle detection
				EntityID cParentID = newParent;
				while(cParentID.isValid()) {
					if (cParentID == entityID) return false;
					cParentID = m_data.at(cParentID).parent;
				}

				// Store old parentID
				auto oldParent = iter->second.parent;

				// Change parent
				auto& parent = m_data.at(iter->second.parent);
				parent.children.erase(ak::find(parent.children, entityID));
				iter->second.parent = newParent;
				if (newParent) m_data.at(newParent).children.insert(entityID); // @todo add root node

				// Send update notification
				sendChangeEvent(entityID, oldParent, newParent);

				return true;
			}

			// /////////////// //
			// // Component // //
			// /////////////// //

			EntityID parent(EntityID entityID) const {
				return m_data.at(entityID).parent;
			}

			const akc::UnorderedVector<EntityID>& children(EntityID entityID) const {
				return m_data.at(entityID).children;
			}

			EntityID findFirstNamed(EntityID baseID, const std::string& name) const {
				auto findFirstNamed = [&](const auto& childIDs, const std::string& childName) {
					for(auto childID : childIDs) if (entityManager().entityName(childID) == childName) return childID;
					return EntityID();
				};
				return baseID ? findFirstNamed(children(baseID), name) : findFirstNamed(m_root, name);
			}

			akc::UnorderedVector<EntityID> findAllNamed(EntityID baseID, const std::string& name) const {
				auto findAllNamed = [&](const auto& childIDs, const std::string& childName) {
					akc::UnorderedVector<EntityID> result;
					for(auto childID : childIDs) if (entityManager().entityName(childID) == childName) result.insert(childID);
					return result;
				};
				return baseID ? findAllNamed(children(baseID), name) : findAllNamed(m_root, name);
			}

			const std::unordered_set<EntityID>& root() const { return m_root; }

			// /////////// //
			// // Event // //
			// /////////// //

			const akev::DispatcherProxy<SceneGraphChangeEvent> sceneGraphChanged() const {
				return akev::DispatcherProxy<SceneGraphChangeEvent>(m_generalChangeDispatcher);
			}

			const akev::DispatcherProxy<SceneGraphChangeEvent> sceneGraphChanged(EntityID entityID) const {
				if (!hasComponent(entityID)) throw std::logic_error("ake::SceneGraph: Entity doesn't have SceneGraph component");
				return akev::DispatcherProxy<SceneGraphChangeEvent>(m_specificChangeDispatcher[entityID]);
			}

			// /////////// //
			// // Other // //
			// /////////// //

			SceneGraph component(EntityID entityID) { return SceneGraph(*this, entityID); }
			const SceneGraph component(EntityID entityID) const { return SceneGraph(const_cast<SceneGraphManager&>(*this), entityID); }

			bool hasComponent(EntityID entityID) const override { return m_data.find(entityID) != m_data.end(); }
	};

}

namespace ake {
	inline SceneGraph::SceneGraph(SceneGraphManager& manager, EntityID id) : m_manager(&manager), m_id(id) {}

	inline bool SceneGraph::setParent(EntityID newParent) {
		return m_manager->setParent(m_id, newParent);
	}

	inline SceneGraph SceneGraph::parent() const {
		return SceneGraph(*m_manager, m_manager->parent(m_id));
	}

	inline akc::UnorderedVector<SceneGraph> SceneGraph::children() const {
		auto transformChildren = [&](const auto& childContainer) {
			akc::UnorderedVector<SceneGraph> result; result.reserve(childContainer.size());
			std::transform(childContainer.begin(), childContainer.end(), result.begin(), [&](const auto& id){ return SceneGraph(*m_manager, id); });
			return result;
		};
		return m_id ? transformChildren(m_manager->children(m_id)) : transformChildren(m_manager->root());
	}

	inline SceneGraph SceneGraph::findFirstNamed(EntityID baseID, const std::string& name) {
		return SceneGraph(*m_manager, m_manager->findFirstNamed(baseID, name));
	}

	inline akc::UnorderedVector<SceneGraph> SceneGraph::findAllNamed(EntityID baseID, const std::string& name) {
		auto childIDs = m_manager->findAllNamed(baseID, name);
		 akc::UnorderedVector<SceneGraph> result; result.reserve(childIDs.size());
		 std::transform(childIDs.begin(), childIDs.end(), result.begin(), [&](auto id){ return SceneGraph(*m_manager, id); });
		 return result;
	}

	inline const SceneGraph SceneGraph::findFirstNamed(EntityID baseID, const std::string& name) const {
		return SceneGraph(*m_manager, m_manager->findFirstNamed(baseID, name));
	}

	inline const akc::UnorderedVector<SceneGraph> SceneGraph::findAllNamed(EntityID baseID, const std::string& name) const {
		auto childIDs = m_manager->findAllNamed(baseID, name);
		 akc::UnorderedVector<SceneGraph> result; result.reserve(childIDs.size());
		 std::transform(childIDs.begin(), childIDs.end(), result.begin(), [&](auto id){ return SceneGraph(*m_manager, id); });
		 return result;
	}

	inline EntityID SceneGraph::id() const {
		return m_id;
	}
}

#endif
