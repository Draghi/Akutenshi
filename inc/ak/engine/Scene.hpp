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

#ifndef AK_ENGINE_SCENE_HPP_
#define AK_ENGINE_SCENE_HPP_

#include <ak/engine/EntityManager.hpp>
#include <ak/engine/SceneManagerProxy.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/event/Event.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <functional>
#include <string>

namespace ake {
	AK_DEFINE_EVENT(SceneUpdateEvent, fpDouble, false);
	AK_DEFINE_EVENT(SceneRenderEvent, fpDouble, false);

	class SceneManager;
	class SceneManagerProxy;

	class Scene final {
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		friend SceneManager;
		private:
			SceneManagerProxy m_sceneManager;
			EntityManager m_entityManager;

			bool m_isActive, m_shouldRender;
			akev::Dispatcher<SceneUpdateEvent> m_updateDispatch;
			akev::Dispatcher<SceneRenderEvent> m_renderDispatch;

			std::string m_name;
			SceneID m_sceneID;

		protected:
			bool update(fpDouble deltaTime);
			bool render(fpDouble deltaTime);

		public:
			Scene(SceneManagerProxy sceneManager, const std::string name, std::function<EntityUIDGenerator_f> entityUIDGenerator);

			EntityManager& entities();
			const EntityManager& entities() const;

			 akev::DispatcherProxy<SceneUpdateEvent> updateEvent();
			 akev::DispatcherProxy<SceneRenderEvent> renderEvent();

			 SceneManagerProxy& sceneManager();
			 const SceneManagerProxy& sceneManager() const;

			 const std::string& name() const;
			 SceneID id() const;
	};


}



#endif /* AK_ENGINE_SCENE_HPP_ */
