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

#include <ak/engine/Scene.hpp>

using namespace ake;

Scene::Scene(SceneManagerProxy sceneManager, const std::string name, std::function<EntityUIDGenerator_f> entityUIDGenerator) : m_sceneManager(sceneManager), m_entityManager(*this, entityUIDGenerator), m_isActive(true),  m_shouldRender(true), m_updateDispatch(), m_renderDispatch(), m_name(name) {}

EntityManager& Scene::entities() { return m_entityManager; }
const EntityManager& Scene::entities() const { return m_entityManager; }

 akev::DispatcherProxy<SceneUpdateEvent> Scene::updateEvent() { return akev::DispatcherProxy<SceneUpdateEvent>(m_updateDispatch); }
 akev::DispatcherProxy<SceneRenderEvent> Scene::renderEvent() { return akev::DispatcherProxy<SceneRenderEvent>(m_renderDispatch); }

 SceneManagerProxy& Scene::sceneManager() { return m_sceneManager; }
 const SceneManagerProxy& Scene::sceneManager() const { return m_sceneManager; }

 const std::string& Scene::name() const { return m_name; }
 SceneID Scene::id() const { return m_sceneID; }

 bool Scene::update(fpDouble deltaTime) {
 	if (m_isActive) { SceneUpdateEvent data(deltaTime); m_updateDispatch.send(data); }
 	return m_isActive;
 }

 bool Scene::render(fpDouble deltaTime) {
 	if (m_shouldRender) { SceneRenderEvent data(deltaTime); m_renderDispatch.send(data); }
 	return m_shouldRender;
 }
