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

#ifndef AK_ENGINE_SCENEMANAGER_HPP_
#define AK_ENGINE_SCENEMANAGER_HPP_

#include <memory>
#include <string>

#include <ak/container/SlotMap.hpp>
#include <ak/PrimitiveTypes.hpp>

namespace ake {
	class Scene;
} /* namespace ake */

namespace ake {

	using SceneID = akc::SlotID;

	class SceneManager final {
		SceneManager(const SceneManager&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		private:
			akc::SlotMap<std::unique_ptr<Scene>> m_scenes;
		public:
			SceneManager();

			void update(fpDouble deltaTime);
			void render(fpDouble deltaTime);

			SceneID newScene(const std::string& name);

			Scene& getScene(SceneID sceneID);
			const Scene& getScene(SceneID sceneID) const;

			bool destroyScene(SceneID sceneID);
	};
}

#endif /* AK_ENGINE_SCENEMANAGER_HPP_ */
