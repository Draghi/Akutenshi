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

#include <AkEngine/entity/Type.hpp>
#include <AkEngine/scene/SceneManager.hpp>
#include <AkEngine/scene/Scene.hpp>
#include <AkEngine/scene/SceneManagerProxy.hpp>
#include <iterator>
#include <utility>

using namespace ake;

SceneManager::SceneManager() { }

void SceneManager::update(fpDouble deltaTime) { for(auto& scene : m_scenes) scene->update(deltaTime); }
void SceneManager::render(fpDouble deltaTime) { for(auto& scene : m_scenes) scene->render(deltaTime); }

SceneID SceneManager::newScene(const std::string& name) {
	auto result = m_scenes.insert(std::make_unique<Scene>(SceneManagerProxy(*this), name, [](ake::EntityManager&){ static ake::EntityUID uid = 0; return ++uid; }));
	if (result.first) (*result.second)->m_sceneID = result.first;
	return result.first;
}

Scene& SceneManager::getScene(SceneID sceneID) { return *m_scenes[sceneID]; }
const Scene& SceneManager::getScene(SceneID sceneID) const { return *m_scenes[sceneID]; }

bool SceneManager::destroyScene(SceneID sceneID) { return m_scenes.erase(sceneID); }



SceneManagerProxy::SceneManagerProxy(SceneManager& manager) : m_manager(manager) {}
SceneID SceneManagerProxy::newScene(const std::string& name) { return m_manager.newScene(name); }
Scene& SceneManagerProxy::getScene(SceneID sceneID) { return m_manager.getScene(sceneID); }
const Scene& SceneManagerProxy::getScene(SceneID sceneID) const { return m_manager.getScene(sceneID); }
bool SceneManagerProxy::destroyScene(SceneID sceneID) { return m_manager.destroyScene(sceneID); }
