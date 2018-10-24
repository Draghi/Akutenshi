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


#include <akasset/AssetRegistry.hpp>
#include <akasset/Convert.hpp>
#include <akasset/Image.hpp>
#include <akasset/Material.hpp>
#include <akasset/Mesh.hpp>
#include <akasset/Serialize.hpp>
#include <akcommon/FPSCounter.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/ScopeGuard.hpp>
#include <akcommon/String.hpp>
#include <akcommon/Time.hpp>
#include <akcommon/Timer.hpp>
#include <akengine/component/Behaviours.hpp>
#include <akengine/component/Camera.hpp>
#include <akengine/data/Json.hpp>
#include <akengine/data/MsgPack.hpp>
#include <akengine/data/PValue.hpp>
#include <akengine/data/Serialize.hpp>
#include <akengine/debug/Log.hpp>
#include <akengine/entity/Config.hpp>
#include <akengine/entity/Entity.hpp>
#include <akengine/entity/EntityManager.hpp>
#include <akengine/event/Dispatcher.hpp>
#include <akengine/event/Event.hpp>
#include <akengine/filesystem/Path.hpp>
#include <akengine/scene/Scene.hpp>
#include <akengine/scene/SceneManager.hpp>
#include <akengine/thread/CurrentThread.hpp>
#include <akgame/CameraControllerBehaviour.hpp>
#include <akinput/keyboard/Keyboard.hpp>
#include <akinput/mouse/Mouse.hpp>
#include <akmath/Matrix.hpp>
#include <akmath/Scalar.hpp>
#include <akmath/Types.hpp>
#include <akrender/gl/Buffers.hpp>
#include <akrender/gl/Draw.hpp>
#include <akrender/gl/RenderTarget.hpp>
#include <akrender/gl/Shaders.hpp>
#include <akrender/gl/Textures.hpp>
#include <akrender/gl/Types.hpp>
#include <akrender/gl/Util.hpp>
#include <akrender/gl/VertexArrays.hpp>
#include <akrender/SceneRendererDefault.hpp>
#include <akrender/window/Types.hpp>
#include <akrender/window/Window.hpp>
#include <akrender/window/WindowOptions.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec2.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <vector>

static void printLogHeader(const akl::Logger& logger);
static ak::ScopeGuard startup();

static void startGame();

int akGameMain() {
	constexpr akl::Logger log(AK_STRING_VIEW("Main"));
	auto shutdownScope = startup();

	log.info("Engine started.");
	printLogHeader(log);

	if (ake::config().exists("window")) {
		if (!akw::open(akd::deserialize<akw::WindowOptions>(ake::config()["window"]))) throw std::runtime_error("Failed to open window");
	} else {
		auto defaultWindowOptions = akw::WindowOptions().glVSync(akw::VSync::FULL);
		if (!akw::open(defaultWindowOptions)) throw std::runtime_error("Failed to open window");
	}

	akw::setCursorMode(akw::CursorMode::Captured);
	akr::gl::init();


	aka::convertDirectory(akfs::Path("./srcdata/"));

	startGame();

	log.info("Exiting main function");

	return 0;
}

struct MaterialUBO final {
	akm::Vec4 baseColour;
	fpSingle metallicFactor;
	fpSingle roughnessFactor;
	akm::Vec3 emmisiveFactor;

	uint32 baseCoordSet;
	uint32 metalRoughnessCoordSet;
	uint32 normalCoordSet;
	uint32 occlusionCoordSet;
	uint32 emissiveCoordSet;

	uint32 alphaMode;
	fpSingle alphaCutoff;
};

static void startGame() {
	// constexpr ak::log::Logger log(AK_STRING_VIEW("Main"));

	aka::AssetRegistry assetRegistry(akfs::Path("data/"));
	ake::SceneManager sceneManager;
	auto& scene = sceneManager.getScene(sceneManager.newScene("World"));
	auto& ecs = scene.entities();

	//if (!ecs.registerComponentManager(std::make_unique<ake::TransformManager>()))  throw std::runtime_error("Failed to register TransformComponent.");
	if (!ecs.registerComponentManager(std::make_unique<ake::CameraManager>()))     throw std::runtime_error("Failed to register CameraManager.");
	if (!ecs.registerComponentManager(std::make_unique<ake::BehavioursManager>())) throw std::runtime_error("Failed to register BehavioursManager.");

	auto cameraEID = ecs.newEntity("Camera");
	//cameraEID.createComponent<ake::Transform>(akm::Vec3{0,0,0});
	cameraEID.createComponent<ake::Camera>();
	cameraEID.createComponent<ake::Behaviours>();
	cameraEID.component<ake::Camera>().setPerspectiveH(akm::degToRad(90), akw::size(), {0.1f, 256.0f});
	cameraEID.component<ake::Behaviours>().addBehaviour(std::make_unique<akgame::CameraControllerBehaviour>());

	auto skyboxTexture = std::make_shared<akr::gl::Texture>(akr::gl::TexTarget::TexCubemap);
	akr::gl::setActiveTexUnit(0);
	akr::gl::bindTexture(0, *skyboxTexture);
	{
		auto pX = aka::loadImageAndTransform("data/skybox/winter/pX.jpg", akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte_sRGB, aka::ImageRotation::None, 0, 0, 0, 0, false, false);
		auto pY = aka::loadImageAndTransform("data/skybox/winter/pY.jpg", akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte_sRGB, aka::ImageRotation::None, 0, 0, 0, 0, false, false);
		auto pZ = aka::loadImageAndTransform("data/skybox/winter/pZ.jpg", akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte_sRGB, aka::ImageRotation::None, 0, 0, 0, 0, false, false);
		auto nX = aka::loadImageAndTransform("data/skybox/winter/nX.jpg", akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte_sRGB, aka::ImageRotation::None, 0, 0, 0, 0, false, false);
		auto nY = aka::loadImageAndTransform("data/skybox/winter/nY.jpg", akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte_sRGB, aka::ImageRotation::None, 0, 0, 0, 0, false, false);
		auto nZ = aka::loadImageAndTransform("data/skybox/winter/nZ.jpg", akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte_sRGB, aka::ImageRotation::None, 0, 0, 0, 0, false, false);

		if ((pX->width() != pY->width()) || (pY->width() != pZ->width()) || (pZ->width() != nX->width()) || (nX->width() != nY->width()) || (nY->width() != nZ->width()) ||
			(pX->height() != pY->height()) || (pY->height() != pZ->height()) || (pZ->height() != nX->height()) || (nX->height() != nY->height()) || (nY->height() != nZ->height())) {
			throw std::runtime_error("Cubemap images don't have matching sizes!");
		}

		akr::gl::newTexStorageCubemap(akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte, pX->width(), pY->height(), akr::gl::calcTexMaxMipmaps(pY->width(), pY->height()));

		akr::gl::loadTexDataCubemap(akr::gl::CubemapTarget::PosX, 0, akr::gl::TexFormat::RGBA, akr::gl::DataType::UInt8, pX->data(), pX->width(), pX->height(), 0, 0);
		akr::gl::loadTexDataCubemap(akr::gl::CubemapTarget::PosY, 0, akr::gl::TexFormat::RGBA, akr::gl::DataType::UInt8, pY->data(), pY->width(), pY->height(), 0, 0);
		akr::gl::loadTexDataCubemap(akr::gl::CubemapTarget::PosZ, 0, akr::gl::TexFormat::RGBA, akr::gl::DataType::UInt8, pZ->data(), pZ->width(), pZ->height(), 0, 0);
		akr::gl::loadTexDataCubemap(akr::gl::CubemapTarget::NegX, 0, akr::gl::TexFormat::RGBA, akr::gl::DataType::UInt8, nX->data(), nX->width(), nX->height(), 0, 0);
		akr::gl::loadTexDataCubemap(akr::gl::CubemapTarget::NegY, 0, akr::gl::TexFormat::RGBA, akr::gl::DataType::UInt8, nY->data(), nY->width(), nY->height(), 0, 0);
		akr::gl::loadTexDataCubemap(akr::gl::CubemapTarget::NegZ, 0, akr::gl::TexFormat::RGBA, akr::gl::DataType::UInt8, nZ->data(), nZ->width(), nZ->height(), 0, 0);
	}
	akr::gl::setTexFilters(akr::gl::TexTarget::TexCubemap, akr::gl::FilterType::Linear, akr::gl::MipFilterType::Linear, akr::gl::FilterType::Linear);
	akr::gl::genTexMipmaps(akr::gl::TexTarget::TexCubemap);

	akr::SceneRendererDefault worldRenderer(skyboxTexture);

/*	akr::gl::RenderBuffer depthRB;
	akr::gl::newRenderBufferDepthStorage(depthRB, akr::gl::DepthStorage::SIZE_32F, akw::frameSize().x, akw::frameSize().y, 0);

	akr::gl::Texture texClusterID(akr::gl::TexTarget::Tex2D);
	akr::gl::setActiveTexUnit(0);
	akr::gl::bindTexture(0, texClusterID);
	akr::gl::newTexStorage2D(akr::gl::TexFormat::RGB, akr::gl::TexStorage::Single, akw::frameSize().x, akw::frameSize().y, 1);

	akr::gl::RenderTarget lightRT;
	akr::gl::attachDepthBuffer(lightRT, depthRB);
	akr::gl::attachColourTexture(lightRT, 0, texClusterID);*/

	// ////////////////// //
	// //  Model Data  // //
	// ////////////////// //

	aka::Mesh humanMeshData = [&]{
		auto rawData = akd::fromMsgPackFile("data/human/meshes/BaseMesh.akmesh", true);
		return akd::deserialize<aka::Mesh>(rawData);
	}();

	aka::Material humanMaterialData = [&]{
		auto rawData = akd::fromJsonFile(assetRegistry.tryGetAssetInfoBySUID(humanMeshData.primitives[0].materialAssetID)->second.clearExtension());
		return akd::deserialize<aka::Material>(rawData);
	}();

	aka::Image humanTextureData = *aka::loadImageAndTransform(assetRegistry.tryGetAssetInfoBySUID(humanMaterialData.baseTexture->imgAssetID)->second.clearExtension(), akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte, aka::ImageRotation::None, 0, 0, 0, 0, false, true);

	// //////////////////// //
	// //  Model Upload  // //
	// //////////////////// //

	akr::gl::VertexArray humanMeshVAO;
	humanMeshVAO.enableVAttribs({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, true);

	akr::gl::Buffer vertexSurfaceDataBuffer(humanMeshData.primitives[0].surfaceData.data(), humanMeshData.primitives[0].surfaceData.size()*sizeof(aka::VertexSurfaceData));
	humanMeshVAO.setVAttribFormats({0, 1, 2, 3}, 3, akr::gl::DataType::Single, false, 0);
	humanMeshVAO.bindVertexBuffer(0, vertexSurfaceDataBuffer, sizeof(aka::VertexSurfaceData), offsetof(aka::VertexSurfaceData, position));
	humanMeshVAO.bindVertexBuffer(1, vertexSurfaceDataBuffer, sizeof(aka::VertexSurfaceData), offsetof(aka::VertexSurfaceData, tangent));
	humanMeshVAO.bindVertexBuffer(2, vertexSurfaceDataBuffer, sizeof(aka::VertexSurfaceData), offsetof(aka::VertexSurfaceData, bitangent));
	humanMeshVAO.bindVertexBuffer(3, vertexSurfaceDataBuffer, sizeof(aka::VertexSurfaceData), offsetof(aka::VertexSurfaceData, normal));

	akr::gl::Buffer skinningDataBuffer(humanMeshData.primitives[0].skinningData.data(), humanMeshData.primitives[0].skinningData.size()*sizeof(aka::VertexWeightData));
	humanMeshVAO.setVAttribFormat(4, 4, akr::gl::DataType::UInt32, false, 0);
	humanMeshVAO.setVAttribFormat(5, 4, akr::gl::DataType::Single, false, 0);
	humanMeshVAO.bindVertexBuffer(4, skinningDataBuffer, sizeof(aka::VertexSurfaceData), offsetof(aka::VertexWeightData, bones));
	humanMeshVAO.bindVertexBuffer(5, skinningDataBuffer, sizeof(aka::VertexSurfaceData), offsetof(aka::VertexWeightData, weights));

	std::vector<akm::Vec2> texCoordData;
	akSize texCoordCount = std::max(std::max(humanMeshData.primitives[0].texCoordData[0].size(), humanMeshData.primitives[0].texCoordData[1].size()), std::max(humanMeshData.primitives[0].texCoordData[2].size(), humanMeshData.primitives[0].texCoordData[3].size()));
	texCoordData.reserve(texCoordCount*4);
	for(akSize i = 0; i < texCoordCount; i++) {
		texCoordData.push_back(i < humanMeshData.primitives[0].texCoordData[0].size() ? humanMeshData.primitives[0].texCoordData[0][i] : akm::Vec2(0,0));
		texCoordData.push_back(i < humanMeshData.primitives[0].texCoordData[1].size() ? humanMeshData.primitives[0].texCoordData[1][i] : akm::Vec2(0,0));
		texCoordData.push_back(i < humanMeshData.primitives[0].texCoordData[2].size() ? humanMeshData.primitives[0].texCoordData[2][i] : akm::Vec2(0,0));
		texCoordData.push_back(i < humanMeshData.primitives[0].texCoordData[3].size() ? humanMeshData.primitives[0].texCoordData[3][i] : akm::Vec2(0,0));
	}

	akr::gl::Buffer texCoordDataBuffer(texCoordData.data(), texCoordData.size()*sizeof(akm::Vec2));
	humanMeshVAO.setVAttribFormats({6, 7, 8, 9}, 2, akr::gl::DataType::Single, false, 0);
	humanMeshVAO.bindVertexBuffer(6, texCoordDataBuffer, sizeof(akm::Vec2)*4, sizeof(akm::Vec2)*0);
	humanMeshVAO.bindVertexBuffer(7, texCoordDataBuffer, sizeof(akm::Vec2)*4, sizeof(akm::Vec2)*1);
	humanMeshVAO.bindVertexBuffer(8, texCoordDataBuffer, sizeof(akm::Vec2)*4, sizeof(akm::Vec2)*2);
	humanMeshVAO.bindVertexBuffer(9, texCoordDataBuffer, sizeof(akm::Vec2)*4, sizeof(akm::Vec2)*3);

	akr::gl::Buffer colourDataBuffer(humanMeshData.primitives[0].colourData.data(), humanMeshData.primitives[0].colourData.size()*sizeof(akm::Vec4));
	humanMeshVAO.setVAttribFormat(10, 4, akr::gl::DataType::Single, false, 0);
	humanMeshVAO.bindVertexBuffer(10, colourDataBuffer, sizeof(akm::Vec4), 0);

	akr::gl::Buffer ibufMesh(humanMeshData.primitives[0].indexData.data(), humanMeshData.primitives[0].indexData.size()*sizeof(uint16));
	humanMeshVAO.bindIndexBuffer(ibufMesh);

	MaterialUBO materialUBO {
		humanMaterialData.baseColour,
		humanMaterialData.metallicFactor,
		humanMaterialData.roughnessFactor,
		humanMaterialData.emmisiveFactor,

		humanMaterialData.baseTexture->texCoordIndex, 0, 0, 0, 0,

		static_cast<uint32>(humanMaterialData.alphaMode),
		humanMaterialData.alphaCutoff
	};

	akr::gl::Buffer materialUniformBuffer(&materialUBO, sizeof(MaterialUBO));

	akr::gl::Texture humanTexture = *akr::gl::createTex2D(0, humanTextureData.format(), humanTextureData.storage(), akr::gl::DataType::UInt8, humanTextureData.data(), humanTextureData.width(), humanTextureData.height(), 1);

	// //////////////////// //
	// //  Depth Prepass // //
	// //////////////////// //
	akr::gl::ShaderProgram depthPrepassShader = akr::gl::util::buildShaderProgram({
		{akr::gl::StageType::Vertex,   "data/shaders/prepass/prepass.vert"},
		{akr::gl::StageType::Fragment, "data/shaders/prepass/prepass.frag"},
	});

	akr::gl::RenderBuffer depthRenderBuffer;
	akr::gl::newRenderBufferDepthStorage(depthRenderBuffer, akr::gl::DepthStorage::SIZE_32F, akw::frameSize().x, akw::frameSize().y, 0);

	akr::gl::Texture depthTexture(akr::gl::TexTarget::Tex2D);
	akr::gl::setActiveTexUnit(0);
	akr::gl::bindTexture(0, depthTexture);
	akr::gl::newTexStorage2D(akr::gl::TexFormat::RGB, akr::gl::TexStorage::Single, akw::frameSize().x, akw::frameSize().y, 1);

	akr::gl::RenderTarget depthRenderTarget;
	akr::gl::attachDepthBuffer(depthRenderTarget, depthRenderBuffer);
	akr::gl::attachColourTexture(depthRenderTarget, 0, depthTexture);

	// ///////////////////////// //
	// //  Mark Clusters Pass // //
	// ///////////////////////// //
	akr::gl::ShaderProgram markClustersShader = akr::gl::util::buildShaderProgram({
		{akr::gl::StageType::Vertex,   "data/shaders/clustered/markClusters.vert"},
		{akr::gl::StageType::Fragment, "data/shaders/clustered/markClusters.frag"},
	});

	scene.renderEvent().subscribe([&](ake::SceneRenderEvent& renderEventData){
		fpDouble delta = renderEventData.data();
		static fpSingle time = 0;
		time += delta;

		akr::gl::enableCullFace(true);
		akr::gl::enableDepthTest(true);

		akr::gl::setClearColour(0.2f, 0.2f, 0.2f);
		akr::gl::clear();

		depthPrepassShader.setUniform(0, cameraEID.component<ake::Camera>().projectionMatrix());
		depthPrepassShader.setUniform(1, akm::inverse(cameraEID.component<ake::Camera>().projectionMatrix()));

		depthPrepassShader.setUniform(2, cameraEID.component<ake::Camera>().viewMatrix());
		depthPrepassShader.setUniform(3, akm::inverse(cameraEID.component<ake::Camera>().viewMatrix()));

		depthPrepassShader.setUniform(11, akm::Vec2(cameraEID.component<ake::Camera>().planeNear(), cameraEID.component<ake::Camera>().planeFar()));

		akr::gl::bindBuffer(akr::gl::BufferTarget::Uniform, materialUniformBuffer, 0);
		akr::gl::bindVertexArray(humanMeshVAO);

		akr::gl::bindRenderTarget(depthRenderTarget);
		akr::gl::bindShaderProgram(depthPrepassShader);
		akr::gl::bindTexture(0, humanTexture);
		depthPrepassShader.setUniform(6, 0);
		for(akSize i = 0; i < 10; i++) {
			for(akSize j = 0; j < 10; j++) {
				depthPrepassShader.setUniform(4, (akm::scale({-50, -50, -50}) * akm::translate({i*.33f, 0, j*.33f})));
				depthPrepassShader.setUniform(5, akm::inverse(akm::scale({-50, -50, -50}) * akm::translate({i*.33f, 0, j*.33f})));

				akr::gl::drawIndexed(akr::gl::DrawType::Triangles, akr::gl::IDataType::UInt16, humanMeshData.primitives[0].indexData.size()*3, 0);
			}
		}

		akr::gl::blitRenderTargetToDisplay(depthRenderTarget, {0,0}, akw::frameSize(), {0,0}, akw::frameSize(), akr::gl::BlitMask::Colour, akr::gl::FilterType::Nearest);

		// Prepare
/*		akr::gl::setClearColour(0.2f, 0.2f, 0.2f);
		akr::gl::clear();

		worldRenderer.renderScene(scene, delta);

		akr::gl::blitRenderTargetToDisplay(worldRenderer.renderTarget(), {0,0}, worldRenderer.renderTargetSize(), {0,0}, worldRenderer.renderTargetSize(), akr::gl::BlitMask::Colour, akr::gl::FilterType::Nearest);
		akr::gl::bindDisplayRenderTarget();*/

		// Finish
		akw::swapBuffer();
	});

	scene.updateEvent().subscribe([&](ake::SceneUpdateEvent& updateEventData){
		fpDouble delta = updateEventData.data();
		static fpSingle time = 0;
		time += delta;

	});

	fpSingle updateAccum = 0.f;
	fpSingle updateDelta = 1.f/ake::config()["engine"]["ticksPerSecond"].asOrDef<fpSingle>(60.f);
	//fpSingle renderDelta = 1.f/static_cast<fpSingle>(akw::currentMonitor().prefVideoMode.refreshRate);

	aku::FPSCounter fps(60), tps(60);
	aku::Timer loopTimer;
	while(!akw::closeRequested()) {
		while(updateAccum >= updateDelta) {
			aku::Timer updateTimer;

			akw::pollEvents();
			akw::mouse().update();
			akw::keyboard().update();

			sceneManager.update(updateDelta);

			tps.update();
			if (updateTimer.mark().secsf() > updateDelta) updateAccum = akm::mod(updateAccum, updateDelta);
			updateAccum -= updateDelta;
		}

		sceneManager.render(fps.update().avgTickDelta());

		std::stringstream sstream;
		sstream << fps.avgTicksPerSecond() << "fps | " << tps.avgTicksPerSecond() << "tps";
		akw::setTitle(sstream.str());

		updateAccum += loopTimer.mark().secsf();
		loopTimer.reset();
	}
}

static void printLogHeader(const akl::Logger& logger) {
	auto utc = aku::utcTimestamp();
	std::stringstream dateStream; dateStream << std::put_time(&utc.ctime, "%Y-%m-%d");
	std::stringstream timeStream; timeStream << std::put_time(&utc.ctime, "%H:%M:%S") << "." << std::setw(3) << std::setfill('0') << utc.milliseconds;
	logger.raw(R"(+-----------------------------------------------------------------------------+)", '\n',
		       R"(|     _____    __               __                            __       __     |)", '\n',
		       R"(|    // __ \  || |             || |                          || |     ((_)    |)", '\n',
			   R"(|   || |_|| | || | __  __  __  || |_   ____   __ ___   ____  || |___   __     |)", '\n',
			   R"(|   ||  __  | || |/ / || ||| | || __| // _ \ || '_  \ // __| || '_  \ || |    |)", '\n',
			   R"(|   || | || | ||   <  || |_| | || |_ ||  __/ || | | | \\__ \ || ||| | || |    |)", '\n',
			   R"(|   ||_| ||_| ||_|\_\ \\___,_|  \\__| \\___| ||_| |_| ||___/ ||_|||_| ||_|    |)", '\n',
			   R"(|  _______________________________________________________________________    |)", '\n',
			   R"(| /\                                                                      \   |)", '\n',
			   R"(| \ \               Game Engine - github.com/draghi/akutenshi              \  |)", '\n',
			   R"(|  \ \______________________________________________________________________\ |)", '\n',
			   R"(|   \/______________________________________________________________________/ |)", '\n',
			   R"(+-----------------------------------------------------------------------------+)", '\n',
			   R"(| Engine started on )", dateStream.str(),  " at ", timeStream.str(), "                                |",  '\n',
			   R"(+-----------------------------------------------------------------------------+)", '\n');
}

static void startupConfig() {
	constexpr akl::Logger log(AK_STRING_VIEW("Config"));

	auto result = ake::loadConfig();
	if (result == ake::ConfigLoadResult::CannotOpen) {
		log.warn("Failed to load config. Attempting to regenerate.");
		ake::regenerateConfig();
		if (!ake::saveConfig()) log.warn("Failed to save new config.");
	} else if (result == ake::ConfigLoadResult::CannotRead) {
		log.fatal("Cannot read config. Fix permissions or delete to attempt regeneration.");
		throw std::runtime_error("Startup error, cannot read config.");
	} else if (result == ake::ConfigLoadResult::CannotParse) {
		log.fatal("Cannot parse config. Fix format or delete to attempt regeneration.");
		throw std::runtime_error("Startup error, cannot parse config.");
	}
}

static ak::ScopeGuard startup() {
	constexpr akl::Logger startLog(AK_STRING_VIEW("Start"));

	akt::current().setName("Main");

	startLog.info("Loading engine config.");
	startupConfig();

	startLog.info("Starting log system.");
	akl::startProcessing();
	//ak::log::enableFileOutput();

	startLog.info("Starting ECS system.");
	//if (!ake::initEntityComponentSystem()) throw std::logic_error("Failed to initialize entity component system.");

	startLog.info("Starting window system.");
	akw::init();

	return [](){
		constexpr akl::Logger stopLog(AK_STRING_VIEW("Stop"));

		stopLog.info("Saving config.");
		if (!ake::saveConfig()) stopLog.warn("Failed to save config.");

		stopLog.info("Flushing log system.");
		akl::stopProcessing();
		akl::processMessageQueue();
		akl::processMessageQueue();
	};
}

static const auto cb = ake::regenerateConfigDispatch().subscribe([](ake::RegenerateConfigEvent& ev){
	auto& config = ev.data()["engine"];
	config["ticksPerSecond"].set<fpSingle>(60.0f);
});
