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

#ifndef AK_ENGINE_SCENEMANAGERPROXY_HPP_
#define AK_ENGINE_SCENEMANAGERPROXY_HPP_

#include <AkCommon/SlotMap.hpp>
#include <string>

namespace ake {

	using SceneID = akc::SlotID;

	class Scene;
	class SceneManager;

	class SceneManagerProxy final {
		private:
			SceneManager& m_manager;
		public:
			SceneManagerProxy(SceneManager& manager);

			SceneID newScene(const std::string& name);

			Scene& getScene(SceneID sceneID);
			const Scene& getScene(SceneID sceneID) const;

			bool destroyScene(SceneID sceneID);
	};

}

#include <AkEngine/scene/SceneManager.hpp>

#endif /* AK_ENGINE_SCENEMANAGERPROXY_HPP_ */
