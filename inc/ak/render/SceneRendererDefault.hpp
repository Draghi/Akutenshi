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

#ifndef AK_RENDER_SCENERENDERERDEFAULT_HPP_
#define AK_RENDER_SCENERENDERERDEFAULT_HPP_

#include <ak/container/SlotMap.hpp>
#include <ak/engine/components/Camera.hpp>
#include <ak/engine/EntityManager.hpp>
#include <ak/engine/Scene.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/gl/Draw.hpp>
#include <ak/render/gl/RenderTarget.hpp>
#include <ak/render/gl/Textures.hpp>
#include <ak/render/SceneRenderer.hpp>
#include <unordered_map>
#include <utility>

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
