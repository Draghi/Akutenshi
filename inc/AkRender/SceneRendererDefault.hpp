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

#ifndef AK_RENDER_SCENERENDERERDEFAULT_HPP_
#define AK_RENDER_SCENERENDERERDEFAULT_HPP_

#include <AkCommon/PrimitiveTypes.hpp>
#include <AkEngine/scene/Scene.hpp>
#include <AkMath/Types.hpp>
#include <AkRender/gl/RenderTarget.hpp>
#include <AkRender/gl/Textures.hpp>
#include <AkRender/SceneRenderer.hpp>
#include <memory>

namespace akr {

	struct SceneRendererDefault_Data;

	class SceneRendererDefault : public SceneRenderer {
		private:
			SceneRendererDefault_Data* m_data;

		public:
			SceneRendererDefault(std::shared_ptr<gl::Texture> skyboxTexture);
			~SceneRendererDefault() override;

			void renderScene(const ake::Scene& scene, fpDouble /*delta*/) override;
			const akr::gl::RenderTarget& renderTarget() override;
			akm::Vec2 renderTargetSize();
	};

}

#endif /* AK_RENDER_SCENERENDERERDEFAULT_HPP_ */
