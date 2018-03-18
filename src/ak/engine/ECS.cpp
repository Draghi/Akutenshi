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
#include <ak/engine/ECS.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/Iter.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/String.hpp>
#include <ext/type_traits.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace ake;

// //////////// //
// // Entity // //
// //////////// //

struct Entity final {
	std::string name;
	std::unordered_multimap<ComponentTID, ComponentID> components;
	bool persistant;
	Entity(const std::string& nameVal) : name(nameVal), persistant(false) {}
};

// ///////// //
// // Fwd // //
// ///////// //

namespace ake {
	namespace internal {
		struct ComponentManagerAccessor {
			static void registerComponentManager(ComponentTID id, const std::function<ComponentManagerFactory_f>& factoryFunc);
		};
	}
}

static akev::Dispatcher<RegisterComponentManagersEvent>& registerComponentManagersEventDispatcher();

// ////////// //
// // Data // //
// ////////// //
static std::unordered_map<ComponentTID, std::function<ComponentManagerFactory_f>>& registry() {
	static std::unordered_map<ComponentTID, std::function<ComponentManagerFactory_f>> instance;
	return instance;
}

// //////////////////// //
// // Implementation // //
// //////////////////// //

static bool& hasInit() { static bool hasInit = false; return hasInit; }
static void assertHasInit() { if (!hasInit()) throw std::logic_error("Entity Component System is not initialized."); }

bool ake::initEntityComponentSystem() {
	if (std::exchange(hasInit(), true)) return false;
	RegisterComponentManagersEvent event(ake::internal::ComponentManagerAccessor::registerComponentManager);
	registerComponentManagersEventDispatcher().send(event);
	return true;
}

EntityManager ake::createEntityManager(const std::function<EUIDGenerator_f>& euidGenerator, const std::function<CUIDGenerator_f>& cuidGenerator) {
	assertHasInit();
	std::unordered_map<ComponentTID, std::unique_ptr<ComponentManager>> componentRegistry;
	for(auto& entry : registry()) componentRegistry.emplace(entry.first, entry.second());
	return EntityManager(std::move(componentRegistry), euidGenerator, cuidGenerator);
}

// ////////////////////////////// //
// // Controlled Functionality // //
// ////////////////////////////// //

inline void ake::internal::ComponentManagerAccessor::registerComponentManager(ComponentTID id, const std::function<ComponentManagerFactory_f>& factoryFunc) {
	auto result = registry().insert(std::make_pair(id, factoryFunc));
	if (!result.second) throw std::logic_error(ak::buildString("EntityManager: ID conflict occurred for '", id, "'"));
}

// //////////////////// //
// // Entity Manager // //
// //////////////////// //
EntityManager::EntityManager(std::unordered_map<ComponentTID, std::unique_ptr<ComponentManager>>&& registry, const std::function<EUIDGenerator_f>& euidGenerator, const std::function<CUIDGenerator_f>& cuidGenerator) : m_componentTypes(std::move(registry)), m_euidGenerator(euidGenerator), m_cuidGenerator(cuidGenerator) {}

EntityID EntityManager::newEntity(EntityUID euid, const akc::SlotID& nameID) {
	auto entityID = m_entities.insert(Entity{euid, nameID, {}}).first;
	if (!entityID) throw std::runtime_error("EntityManager::createEntity: Failed to create new entity!");;
	m_names[nameID].second.insert(entityID);
	return m_lookupEIDbyEUID.emplace(euid, entityID).second ? entityID : EntityID{};
}

EntityID EntityManager::newEntity(EntityUID euid, const std::string& name) {
	auto nameResult = m_lookupNIDbyName.emplace(name, m_names.insert(names_t{name,{}}).first);
	auto result = newEntity(euid, nameResult.first->second);
	if ((!result) && (nameResult.second)) m_lookupNIDbyName.erase(name);
	return result;
}

EntityID EntityManager::newEntity(const akc::SlotID& nameID) {
	EntityID result;
	while(!(result = newEntity(m_euidGenerator(*this), nameID)));
	return result;
}

EntityID EntityManager::newEntity(const std::string& name) {
	auto nameID = m_lookupNIDbyName.emplace(name, m_names.insert(names_t{name,{}}).first).first->second;
	return newEntity(nameID);
}

bool EntityManager::deleteEntity(EntityID entityID) {
	// Remove Components
	auto& entity = m_entities[entityID];
	for(auto& entry : entity.components) {
		auto& componentType = getComponentType(entry.first);
		for(auto componentID : entry.second) componentType.deleteComponent(*this, entityID, componentID);
	}

	// Update Name Lookup
	auto & namedEntities = m_names.at(entity.name);
	namedEntities.second.erase(ak::find(namedEntities.second, entityID));
	if (namedEntities.second.size() == 0) {
		m_lookupNIDbyName.erase(namedEntities.first);
		m_names.erase(entity.name);
	}

	m_lookupEIDbyEUID.erase(entity.uid);
	m_entities.erase(entityID);

	return true;
}

void EntityManager::deleteAllEntities() {
	while(!m_entities.empty()) deleteEntity(m_entities.slotIDFor(m_entities.rbegin()));
}

bool EntityManager::removeComponent(EntityID entityID, ComponentTID componentTID, ComponentID componentID) {
	if (!m_entities.exists(entityID)) throw std::out_of_range("EntityManager::removeComponentsFrom: Attempt to index out of bounds");

	auto& components = m_entities[entityID].components;
	auto& componentsOfType = components.at(componentTID);

	auto entryIter = ak::find(componentsOfType, componentID);
	if (entryIter == componentsOfType.end()) return false;
	if (!m_componentTypes[componentTID]->deleteComponent(*this, entityID, componentID)) throw std::logic_error(ak::buildString("EntityManager::removeComponentsFrom: Component Data Mismatch between EntityManager and ComponentManager '", m_componentTypes[componentTID]->name(), "'"));

	componentsOfType.erase(entryIter);

	CUIDRecord cuidRecord{entityID, componentTID, componentID};
	deassociateCUID(m_lookupCUIDByCID.at(cuidRecord), cuidRecord);

	return true;
}

void EntityManager::associateCUID(ComponentUID componentUID, CUIDRecord cuidRecord) {
	m_lookupCIDbyCUID.emplace(componentUID, cuidRecord);
	m_lookupCUIDByCID.emplace(cuidRecord, componentUID);
}

void EntityManager::deassociateCUID(ComponentUID componentUID, CUIDRecord cuidRecord) {
	m_lookupCIDbyCUID.erase(componentUID);
	m_lookupCUIDByCID.erase(cuidRecord);
}

ComponentManager& EntityManager::getComponentType(ComponentTID componentTID) {
	return *m_componentTypes.at(componentTID);
}

const ComponentManager& EntityManager::getComponentType(ComponentTID componentTID) const {
	return *m_componentTypes.at(componentTID);
}

ComponentUID EntityManager::getInstanceUID(EntityID entityID, ComponentTID componentTID, ComponentID componentID) const {
	auto iter = m_lookupCUIDByCID.find({entityID, componentTID, componentID});
	return (iter != m_lookupCUIDByCID.end()) ? iter->second : 0;
}

CUIDRecord EntityManager::getInstanceID(ComponentUID componentUID) const {
	auto iter = m_lookupCIDbyCUID.find(componentUID);
	return (iter != m_lookupCIDbyCUID.end()) ? iter->second : CUIDRecord{{},{},{}};
}

std::vector<EntityID> EntityManager::findEntitiesNamed(const std::string& name) const {
	auto iter = m_lookupNIDbyName.find(name);
	return iter != m_lookupNIDbyName.end() ? m_names[iter->second].second.getContainer() : std::vector<EntityID>{};
}

std::vector<ComponentID> EntityManager::findComponentsOfTypeFor(ake::EntityID entityID, ake::ComponentTID componentTID) const {
	if (!m_entities.exists(entityID)) throw std::out_of_range("EntityManager::findAllComponentInstancesInEntity: Attempt to index out of bounds");
	auto& components = m_entities[entityID].components;
	auto iter = components.find(componentTID);
	return (iter != components.end()) ? iter->second : std::vector<ComponentID>{};
}

std::vector<ComponentTID> EntityManager::findComponentTypesFor(ake::EntityID entityID) const {
	if (!m_entities.exists(entityID)) throw std::out_of_range("EntityManager::findAllComponentIDsFor: Attempt to index out of bounds");
	auto& components = m_entities[entityID].components;
	if (components.size() == 0) return {};
	std::vector<ComponentTID> result; result.reserve(components.size());
	for(auto& kvPair : components) result.push_back(kvPair.first);
	return result;
}

std::string EntityManager::getEntityName(EntityID entityID) const {
	return m_names[m_entities[entityID].name].first;
}

EntityUID EntityManager::getEntityUID(EntityID entityID) const {
	return m_entities[entityID].uid;
}

akSize EntityManager::getEntityCount() const {
	return m_entities.size();
}

// /////////// //
// // Event // //
// /////////// //

static akev::Dispatcher<RegisterComponentManagersEvent>& registerComponentManagersEventDispatcher() {
	static akev::Dispatcher<RegisterComponentManagersEvent> instance;
	return instance;
}

const akev::DispatcherProxy<RegisterComponentManagersEvent>& ake::registerComponentManagersEventDispatch() {
	static akev::DispatcherProxy<RegisterComponentManagersEvent> instance(registerComponentManagersEventDispatcher());
	return instance;
}

