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

#include <ak/engine/ECS.hpp>

using namespace ake;

// /////////////////// //
// // EntityManager // //
// /////////////////// //

EntityManager::EntityManager(std::function<EntityUIDGenerator_f> entityUIDGenerator) : m_components(), m_entityUIDGenerator(entityUIDGenerator) {
	for(auto& component : m_components) component.second->m_entityManager = this;
}

EntityManager::EntityManager(EntityManager&& other)
	: m_components(std::move(other.m_components)), m_entityUIDGenerator(std::move(other.m_entityUIDGenerator)),
	  m_nameStorage(std::move(other.m_nameStorage)), m_entityNameID(std::move(other.m_entityNameID)), m_lookupEntitiesByName(std::move(other.m_lookupEntitiesByName)),
	  m_entityComponentIDs(std::move(other.m_entityComponentIDs)), m_entityUID(std::move(other.m_entityUID)), m_lookupEntityByUID(std::move(other.m_lookupEntityByUID)) {
	for(auto& component : m_components) component.second->m_entityManager = this;
}

EntityManager& EntityManager::operator=(EntityManager&& other) {
	m_components = std::move(other.m_components);
	m_entityUIDGenerator = std::move(other.m_entityUIDGenerator);

	m_nameStorage = std::move(other.m_nameStorage);
	m_entityNameID = std::move(other.m_entityNameID);
	m_lookupEntitiesByName = std::move(other.m_lookupEntitiesByName);

	m_entityComponentIDs = std::move(other.m_entityComponentIDs);
	m_entityUID = std::move(other.m_entityUID);
	m_lookupEntityByUID = std::move(other.m_lookupEntityByUID);

	for(auto& component : m_components) component.second->m_entityManager = this;

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

EntityID EntityManager::newEntity(EntityUID targetUID, akc::SlotID nameID) {
	auto entityID = m_entityNameID.insert(nameID).first;
	if (m_entityComponentIDs.insert({}).first != entityID) throw std::logic_error("EntityManager: Corrupted data, entityID mismatch in storage.");
	if (!m_lookupEntityByUID.emplace(targetUID, entityID).second) throw std::logic_error("EntityManager: Corrupted data, entityUID already in use.");
	auto name = m_nameStorage[nameID];
	m_lookupEntitiesByName[name].second.insert(entityID);
	return entityID;
}

EntityID EntityManager::newEntity(EntityUID targetUID, const std::string& name) {
	// Ensure no conflict
	if (m_lookupEntityByUID.find(targetUID) != m_lookupEntityByUID.end()) return EntityID();

	// Create lookup record
	auto nameRecord = m_lookupEntitiesByName.insert({name, {akc::SlotID(), {}}});
	if (nameRecord.second) nameRecord.first->second.first = m_nameStorage.insert(name).first;

	// Create entity
	return newEntity(targetUID, nameRecord.first->second.first);
}

EntityID EntityManager::newEntity(const std::string& name) {
	return newEntity(nextEntityUID(), name);
}

bool EntityManager::deleteEntity(EntityID entityID) {
	auto componentIDs = m_entityComponentIDs[entityID];
	for(auto componentID : componentIDs) destroyComponent(entityID, componentID);
	m_entityComponentIDs.erase(entityID);

	auto& nameLookup = m_lookupEntitiesByName[m_nameStorage[m_entityNameID[entityID]]].second;
	nameLookup.erase(ak::find(nameLookup, entityID));
	if (nameLookup.size() == 0) {
		m_lookupEntitiesByName.erase(m_nameStorage[m_entityNameID[entityID]]);
		m_nameStorage.erase(m_entityNameID[entityID]);
	}
	m_entityNameID.erase(entityID);

	m_lookupEntityByUID.erase(m_entityUID[entityID]);
	m_entityUID.erase(entityID);

	return true;
}

std::string EntityManager::entityName(EntityID entityID) const {
	return m_nameStorage[m_entityNameID[entityID]];
}

std::unordered_set<ComponentID> EntityManager::entityComponentIDs(EntityID entityID) const {
	return m_entityComponentIDs[entityID];
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



