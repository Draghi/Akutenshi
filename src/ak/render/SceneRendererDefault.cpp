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

#include <ak/render/SceneRendererDefault.hpp>

#include <unordered_map>
#include <utility>
#include <vector>

#include <ak/container/SlotMap.hpp>
#include <ak/engine/components/Camera.hpp>
#include <ak/engine/EntityManager.hpp>
#include <ak/engine/Scene.hpp>
#include <ak/render/gl/Buffers.hpp>
#include <ak/render/gl/Draw.hpp>
#include <ak/render/gl/RenderTarget.hpp>
#include <ak/render/gl/Shaders.hpp>
#include <ak/render/gl/Textures.hpp>
#include <ak/render/gl/Types.hpp>
#include <ak/render/gl/Util.hpp>
#include <ak/render/gl/VertexArrays.hpp>
#include <ak/window/Window.hpp>

namespace akr {
	struct SceneRendererDefault_Data {
		akm::Vec2 size;
		gl::RenderTarget finalRenderTarget;
		gl::RenderBuffer finalColourBuffer;

		struct {
			gl::ShaderProgram shader;
			std::shared_ptr<gl::Texture> texture;
			gl::VertexArray vertexArray;
		} skybox;

	};
}

using namespace akr;

static void renderSkybox(const ake::Camera& camera, SceneRendererDefault_Data& data);

SceneRendererDefault::SceneRendererDefault(std::shared_ptr<gl::Texture> skyboxTexture) {
	m_data = new SceneRendererDefault_Data();

	// Final render
	m_data->size = akw::size();
	gl::newRenderBufferColourStorage(m_data->finalColourBuffer, gl::TexFormat::RGBA, gl::TexStorage::Byte_sRGB, static_cast<uint32>(m_data->size.x), static_cast<uint32>(m_data->size.y), 0);
	gl::attachColourBuffer(m_data->finalRenderTarget, 0, m_data->finalColourBuffer);

	// Skybox
	m_data->skybox.shader = akr::gl::util::buildShaderProgram({
		{akr::gl::StageType::Vertex,   "data/shaders/skybox/skybox.vert"},
		{akr::gl::StageType::Fragment, "data/shaders/skybox/skybox.frag"},
	});

	m_data->skybox.texture = skyboxTexture;

	m_data->skybox.vertexArray.enableVAttrib(0);
	m_data->skybox.vertexArray.setVAttribFormat(0, 2, akr::gl::DataType::Single); {
		fpSingle arrSkyboxVerts[] = {-1, -1,  1, -1,  1,  1, /** 2 **/  1,  1, -1,  1, -1, -1};
		akr::gl::Buffer vbufSkybox(arrSkyboxVerts, 12*sizeof(fpSingle));
		m_data->skybox.vertexArray.bindVertexBuffer(0, vbufSkybox, 2*sizeof(fpSingle));
	}
}

SceneRendererDefault::~SceneRendererDefault() {
	delete m_data;
}



void SceneRendererDefault::renderScene(const ake::Scene& scene, fpDouble /*delta*/) {
	for(auto& camera : scene.entities().componentManager<ake::Camera>().components()) {
		renderSkybox(camera.second, *m_data);
	}
}

const akr::gl::RenderTarget& SceneRendererDefault::renderTarget() {
	return m_data->finalRenderTarget;
}

akm::Vec2 SceneRendererDefault::renderTargetSize() {
	return m_data->size;
}

static void renderSkybox(const ake::Camera& camera, SceneRendererDefault_Data& data) {
	gl::setViewport(camera.viewOffset()*data.size, camera.viewSize()*data.size);
	gl::bindRenderTarget(data.finalRenderTarget);
	akr::gl::enableDepthTest(false);
	akr::gl::enableCullFace(false);

	data.skybox.shader.setUniform(0, camera.projectionMatrix());
	data.skybox.shader.setUniform(1, camera.viewMatrix());
	data.skybox.shader.setUniform(3, 0);

	akr::gl::bindShaderProgram(data.skybox.shader);
	akr::gl::bindVertexArray(data.skybox.vertexArray);
	akr::gl::bindTexture(0, *data.skybox.texture);
	akr::gl::draw(akr::gl::DrawType::Triangles, 6);
}
