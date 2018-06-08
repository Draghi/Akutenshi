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

#include <ak/container/SlotMap.hpp>
#include <ak/container/UnorderedVector.hpp>
#include <ak/engine/EntityManager.hpp>
#include <ak/engine/internal/EntityGraph.hpp>
#include <ak/engine/Type.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/Iterator.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

using namespace ake;
using namespace ake::internal;

// ////////////// //
// // Register // //
// ////////////// //

void EntityGraphManager::sendChangeEvent(EntityID modifiedEntity, EntityID oldParent, EntityID newParent) {
	EntityParentChangedEvent event(EventGraphChangedData(modifiedEntity, oldParent, newParent));
	m_generalChangeDispatcher.send(event);
	auto specificIter = m_specificChangeDispatcher.find(modifiedEntity);
	if (specificIter != m_specificChangeDispatcher.end()) specificIter->second.send(event);
}

bool EntityGraphManager::registerEntity(EntityID entityID, EntityID parent) {
	if ((parent) && (!m_data.exists(parent))) return false;
	if (m_data.insert(GraphNode{parent, {}}).first != entityID) return false;
	if (parent) m_data.at(parent).children.insert(entityID);
	else m_root.insert(entityID);
	return true;
}

bool EntityGraphManager::deregisterEntity(EntityID entityID) {
	for(auto childID : m_data.at(entityID).children) m_entityManager->deleteEntity(childID);
	m_root.erase(entityID);
	m_data.erase(entityID);
	m_specificChangeDispatcher.erase(entityID);
	return true;
}

// /////////// //
// // Graph // //
// /////////// //

bool EntityGraphManager::setParent(EntityID entityID, EntityID newParent) {
	auto& childEntry = m_data.at(entityID);
	if (childEntry.parent == newParent) return true;

	// Cycle detection
	EntityID cParentID = newParent;
	while(cParentID.isValid()) {
		if (cParentID == entityID) return false;
		cParentID = m_data.at(cParentID).parent;
	}

	// Store old parentID
	auto oldParent = childEntry.parent;

	// Change parent
	auto& parentEntry = m_data.at(childEntry.parent);
	parentEntry.children.erase(ak::find(parentEntry.children, entityID));
	childEntry.parent = newParent;
	if (newParent) m_data.at(newParent).children.insert(entityID);
	else m_root.insert(entityID);

	// Send update notification
	sendChangeEvent(entityID, oldParent, newParent);

	return true;
}

EntityID EntityGraphManager::parent(EntityID entityID) const {
	return m_data.at(entityID).parent;
}

const akc::UnorderedVector<EntityID>& EntityGraphManager::children(EntityID entityID) const {
	return m_data.at(entityID).children;
}

EntityID EntityGraphManager::findFirstNamed(EntityID baseID, const std::string& name) const {
	auto findFirstNamed = [&](const auto& childIDs, const std::string& childName) {
		for(auto childID : childIDs) {
			if (m_entityManager->entityName(childID) == childName) return childID;
		}
		return EntityID();
	};
	return baseID ? findFirstNamed(children(baseID), name) : findFirstNamed(m_root, name);
}

akc::UnorderedVector<EntityID> EntityGraphManager::findAllNamed(EntityID baseID, const std::string& name) const {
	auto findAllNamed = [&](const auto& childIDs, const std::string& childName) {
		akc::UnorderedVector<EntityID> result;
		for(auto childID : childIDs) {
			if (m_entityManager->entityName(childID) == childName) result.insert(childID);
		}
		return result;
	};
	return baseID ? findAllNamed(children(baseID), name) : findAllNamed(m_root, name);
}

// /////////// //
// // Other // //
// /////////// //

const std::unordered_set<EntityID>& EntityGraphManager::root() const {
	return m_root;
}

// /////////// //
// // Event // //
// /////////// //

const akev::DispatcherProxy<EntityParentChangedEvent> EntityGraphManager::entityGraphChanged() const {
	return akev::DispatcherProxy<EntityParentChangedEvent>(m_generalChangeDispatcher);
}

const akev::DispatcherProxy<EntityParentChangedEvent> EntityGraphManager::entityGraphChanged(EntityID entityID) const {
	return akev::DispatcherProxy<EntityParentChangedEvent>(m_specificChangeDispatcher[entityID]);
}


