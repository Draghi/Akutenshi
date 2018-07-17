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

#ifndef AK_RENDER_SCENERENDERER_HPP_
#define AK_RENDER_SCENERENDERER_HPP_

#include <ak/PrimitiveTypes.hpp>

namespace ake {
	class Scene;
}

namespace akr {
	namespace gl {
		class RenderTarget;
	}
}

namespace akr {

	class SceneRenderer {
		SceneRenderer(const SceneRenderer&) = delete;
		SceneRenderer& operator=(const SceneRenderer&) = delete;
		protected:
			SceneRenderer() = default;

		public:
			virtual ~SceneRenderer() = default;

			virtual void renderScene(const ake::Scene& scene, fpDouble delta) = 0;
			virtual const akr::gl::RenderTarget& renderTarget() = 0;
	};

}

#endif
