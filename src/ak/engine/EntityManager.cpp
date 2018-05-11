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

#include <ak/engine/EntityManager.hpp>

using namespace ake;

// /////////////////// //
// // EntityManager // //
// /////////////////// //

EntityManager::EntityManager(std::function<EntityUIDGenerator_f> entityUIDGenerator) : m_components(), m_entityUIDGenerator(entityUIDGenerator) {
	for(auto& component : m_components) component.second->m_entityManager = this;
}

EntityManager::EntityManager(EntityManager&& other)
	: m_components(std::move(other.m_components)), m_entityUIDGenerator(std::move(other.m_entityUIDGenerator)),
	  //m_nameStorage(std::move(other.m_nameStorage)), m_entityNameID(std::move(other.m_entityNameID)), m_lookupEntitiesByName(std::move(other.m_lookupEntitiesByName)),
	  m_entityNameStorage(std::move(other.m_entityNameStorage)), m_entityComponentIDs(std::move(other.m_entityComponentIDs)), m_entityUID(std::move(other.m_entityUID)),
	  m_lookupEntityByUID(std::move(other.m_lookupEntityByUID)) {
	for(auto& component : m_components) {
		component.second->m_entityManager = this;
		component.second->registerHooks();
	}
}

EntityManager& EntityManager::operator=(EntityManager&& other) {
	m_components = std::move(other.m_components);
	m_entityUIDGenerator = std::move(other.m_entityUIDGenerator);

	m_entityNameStorage = std::move(other.m_entityNameStorage);
	m_entityComponentIDs = std::move(other.m_entityComponentIDs);
	m_entityUID = std::move(other.m_entityUID);

	m_lookupEntityByUID = std::move(other.m_lookupEntityByUID);

	for(auto& component : m_components) {
		component.second->m_entityManager = this;
		component.second->registerHooks();
	}

	return *this;
}


// ////////////// //
// // Entities // //
// ////////////// //
EntityUID EntityManager::nextEntityUID() {
	EntityUID uid;
	while(m_lookupEntityByUID.find(uid = m_entityUIDGenerator(*this)) != m_lookupEntityByUID.end());
	return uid;
}

Entity EntityManager::newEntity(const std::string& name, EntityID parentID, EntityUID targetUID) {
	// Validation
	if (targetUID == 0) targetUID = nextEntityUID();
	if (m_lookupEntityByUID.find(targetUID) != m_lookupEntityByUID.end()) throw std::logic_error("EntityManager: Entity with given UID already exists.");

	// Create Entity
	auto entityID = m_entityComponentIDs.insert({}).first;
	m_entityNameStorage.registerEntity(entityID, name);
	m_entityGraph.registerEntity(entityID, parentID);
	m_entityUID.insert(targetUID);

	m_lookupEntityByUID.emplace(targetUID, entityID);

	return Entity(*this, entityID);
}

bool EntityManager::deleteEntity(EntityID entityID) {
	auto componentIDs = m_entityComponentIDs[entityID];
	for(auto componentID : componentIDs) destroyComponent(entityID, componentID);

	m_entityNameStorage.deregisterEntity(entityID);
	m_entityComponentIDs.erase(entityID);
	m_entityUID.erase(entityID);
	m_entityGraph.deregisterEntity(entityID);

	m_lookupEntityByUID.erase(m_entityUID[entityID]);

	return true;
}

const std::string& EntityManager::entityName(EntityID entityID) const {
	return m_entityNameStorage.getNameByEntity(entityID);
}

std::unordered_set<ComponentID> EntityManager::entityComponents(EntityID entityID) const {
	return m_entityComponentIDs[entityID];
}

EntityUID EntityManager::entityUID(EntityID entityID) const {
	return m_lookupEntityByUID.at(entityID);
}

bool EntityManager::setEntityParentID(EntityID entityID, EntityID parentID) {
	return m_entityGraph.setParent(entityID, parentID);
}

EntityID EntityManager::entityParentID(EntityID entityID) const {
	return m_entityGraph.parent(entityID);
}

const akc::UnorderedVector<EntityID>& EntityManager::entityChildrenIDs(EntityID entityID) const {
	return m_entityGraph.children(entityID);
}

EntityID EntityManager::entityFindFirstChildNamed(EntityID entityID, const std::string& name) const {
	return m_entityGraph.findFirstNamed(entityID, name);
}

akc::UnorderedVector<EntityID> EntityManager::entityFindAllChildrenNamed(EntityID entityID, const std::string& name) const {
	return m_entityGraph.findAllNamed(entityID, name);
}

const std::unordered_set<EntityID>& EntityManager::entityGraphRoot() const {
	return m_entityGraph.root();
}

// ////////////// //
// // Instance // //
// ////////////// //
bool EntityManager::destroyComponent(EntityID entityID, ComponentID componentID) {
	auto& componentIDs = m_entityComponentIDs[entityID];
	auto iter = componentIDs.find(componentID);
	if (iter == componentIDs.end()) return true;

	if (!m_components.at(componentID)->destroyComponent(entityID)) return false;
	componentIDs.erase(componentID);

	return true;
}

bool EntityManager::serializeComponent(akd::PValue& dest, EntityID entityID, ComponentID componentID) {
	auto& componentIDs = m_entityComponentIDs[entityID];
	auto iter = componentIDs.find(componentID);
	if (iter != componentIDs.end()) return false;

	return m_components.at(componentID)->serializeComponent(dest, entityID);
}

bool EntityManager::deserializeComponent(EntityID entityID, ComponentID componentID, const akd::PValue& src) {
	auto& componentIDs = m_entityComponentIDs[entityID];
	auto iter = componentIDs.find(componentID);
	if (iter != componentIDs.end()) return false;

	if (!m_components.at(componentID)->deserializeComponent(entityID, src)) return false;
	componentIDs.insert(componentID);
	return true;
}

bool EntityManager::hasComponent(EntityID entityID, ComponentID componentID) const {
	return componentManager(componentID).hasComponent(entityID);
}

// //////////////// //
// // Components // //
// //////////////// //

bool EntityManager::registerComponentManager(std::unique_ptr<ComponentManager>&& component) {
	auto result = m_components.emplace(component->id(), std::move(component));
	if (!result.second) return false;
	result.first->second->m_entityManager = this;
	return true;
}

bool EntityManager::componentManagerExists(ComponentID componentID) {
	return m_components.find(componentID) != m_components.end();
}

ComponentManager& EntityManager::componentManager(ComponentID componentID) {
	return *m_components.at(componentID);
}

const ComponentManager& EntityManager::componentManager(ComponentID componentID) const {
	return *m_components.at(componentID);
}


// //////////////// //
// // SceneGraph // //
// //////////////// //

const akev::DispatcherProxy<EntityParentChangedEvent> EntityManager::entityParentChanged() const {
	return m_entityGraph.entityGraphChanged();
}

const akev::DispatcherProxy<EntityParentChangedEvent> EntityManager::entityParentChanged(EntityID entityID) const {
	return m_entityGraph.entityGraphChanged(entityID);
}





