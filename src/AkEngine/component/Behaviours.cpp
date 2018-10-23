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

#include <AkEngine/component/Behaviours.hpp>
#include <AkEngine/entity/Entity.hpp>
#include <AkEngine/entity/EntityManager.hpp>
#include <AkEngine/event/Dispatcher.hpp>
#include <AkEngine/scene/Scene.hpp>
#include <algorithm>
#include <iterator>
#include <utility>

using namespace ake;

void Behaviour::onStart() {}
void Behaviour::onUpdate(fpSingle /*deltaTime*/) {}

Entity Behaviour::entity() { return Entity(m_owner->entityManager(), m_owner->id()); }
const Entity Behaviour::entity() const { return Entity(m_owner->entityManager(), m_owner->id()); }

Behaviour::Behaviour(const std::string& name) : m_owner(nullptr), m_behaviourID(), m_name(name) {}
Behaviour::~Behaviour() {}

void Behaviour::setActive(bool active) { m_isActive = active; }
bool Behaviour::isActive() const { return m_isActive; }

BehaviourID Behaviour::id() const { return m_behaviourID; }
const std::string& Behaviour::name() const { return m_name; }

Behaviours& Behaviour::owner() { return *m_owner; }
const Behaviours& Behaviour::owner() const { return *m_owner; }









void Behaviours::onUpdate(fpSingle deltaTime) {
	for(auto& behaviour : m_behaviours) if (behaviour->isActive()) behaviour->onUpdate(deltaTime);
}

Behaviours::Behaviours(EntityManager& entityManager, BehavioursManager& behavioursManager, EntityID id)
	: m_entityManager(&entityManager), m_behavioursManager(&behavioursManager), m_id(id) {}

BehaviourID Behaviours::addBehaviour(std::unique_ptr<Behaviour>&& behaviour) {
	auto result = m_behaviours.insert(std::move(behaviour));
	(*result.second)->m_owner = this;
	(*result.second)->m_behaviourID = result.first;
	return result.first;
}

Behaviour& Behaviours::getBehaviour(BehaviourID behaviourID) { return *m_behaviours[behaviourID]; }
const Behaviour& Behaviours::getBehaviour(BehaviourID behaviourID) const { return *m_behaviours[behaviourID]; }

bool Behaviours::removeBehaviour(BehaviourID behaviourID) { return m_behaviours.erase(behaviourID); }

BehavioursManager& Behaviours::behavioursManager() { return *m_behavioursManager; }
const BehavioursManager& Behaviours::behavioursManager() const { return *m_behavioursManager; }

EntityManager& Behaviours::entityManager() { return *m_entityManager; }
const EntityManager& Behaviours::entityManager() const { return *m_entityManager; }

EntityID Behaviours::id() const { return m_id; }

void Behaviours::setActive(bool active) { m_isActive = active; }
bool Behaviours::isActive() const { return m_isActive; }





bool BehavioursManager::createComponent(EntityID entityID) {
	return m_behaviours.emplace(entityID, Behaviours(entityManager(), *this, entityID)).second;
}

bool BehavioursManager::destroyComponent(EntityID entityID) {
	return m_behaviours.erase(entityID) > 0;
}

void BehavioursManager::registerHooks() {
	entityManager().scene().updateEvent().subscribe([&](SceneUpdateEvent& ev){
		for(auto& behaviour : m_behaviours) if (behaviour.second.isActive()) behaviour.second.onUpdate(ev.data());
	});
}

BehavioursManager::BehavioursManager() : m_behaviours() {}

Behaviours& BehavioursManager::component(EntityID entityID) {
	return m_behaviours.at(entityID);
}

const Behaviours& BehavioursManager::component(EntityID entityID) const {
	return m_behaviours.at(entityID);
}

bool BehavioursManager::hasComponent(EntityID entityID) const {
	return m_behaviours.find(entityID) != m_behaviours.end();
}















