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

#include <ak/container/Octree.hpp>
#include <ak/container/SlotMap.hpp>
#include <ak/animation/Fwd.hpp>
#include <ak/animation/Mesh.hpp>
#include <ak/animation/MeshPoseData.hpp>
#include <ak/animation/Skeleton.hpp>
#include <ak/animation/Serialize.hpp>
#include <ak/animation/Type.hpp>
#include <ak/data/Brotli.hpp>
#include <ak/data/Image.hpp>
#include <ak/data/MsgPack.hpp>
#include <ak/data/PValue.hpp>
#include <ak/engine/Camera.hpp>
#include <ak/engine/Config.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/input/Keyboard.hpp>
#include <ak/input/Keys.hpp>
#include <ak/input/Mouse.hpp>
#include <ak/Log.hpp>
#include <ak/math/Matrix.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/math/Vector.hpp>
#include <ak/Macros.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/Buffers.hpp>
#include <ak/render/Draw.hpp>
#include <ak/render/Shaders.hpp>
#include <ak/render/Textures.hpp>
#include <ak/render/Types.hpp>
#include <ak/render/VertexArrays.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/String.hpp>
#include <ak/thread/CurrentThread.hpp>
#include <ak/util/FPSCounter.hpp>
#include <ak/util/Time.hpp>
#include <ak/util/Timer.hpp>
#include <ak/window/Monitor.hpp>
#include <ak/window/Types.hpp>
#include <ak/window/Window.hpp>
#include <ak/window/WindowOptions.hpp>
#include <cstddef>
#include <iomanip>
#include <iosfwd>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <experimental/filesystem>
#include <string>
#include <utility>
#include <vector>

int akGameMain();

static void printLogHeader(const ak::log::Logger& logger);
static ak::ScopeGuard startup();

static akr::ShaderProgram buildShaderProgram(const std::vector<std::pair<akr::StageType, std::string>>& stages);
static akr::Texture loadTexture(akfs::SystemFolder folderType, const stx::filesystem::path& path);
static akr::Texture loadTextureCM(akfs::SystemFolder folder, const stx::filesystem::path& path);

template<typename type_t> static type_t readMeshFile(const std::string& filename);

int akGameMain() {
	constexpr ak::log::Logger log(AK_STRING_VIEW("Main"));
	auto shutdownScope = startup();

/*	akc::SlotMap<uint64> map;
	auto printContents = [&]{
		std::stringstream sstream;
		for(auto iter = map.begin(); iter != map.end(); iter++) sstream << *iter << ", ";
		log.info("Contents: ", sstream.str());
	};

	akc::SlotID keys[10];
	for(auto i = 0; i < 10; i++) {
		keys[i] = map.insert(i);
		log.info("Added value (", i, ") to map at (", keys[i].index, ":", keys[i].generation, ")");
		printContents();
	}


	for(auto i = 0; i < 10; i++) {
		//log.info("Removing value at (", keys[i].index, ":", keys[i].generation, ")");
		map.erase(keys[i]);

		auto ret = map.insert(i);
		//log.info("Added value (", i, ") to map at (", ret.index, ":", ret.generation, ")");
		keys[i] = ret;
		printContents();
	}

	return 0;*/

	auto printDepth = [&](uint8 depth, akc::OctreeLocation loc) {
		uint32 depthInfo = loc.getIndexForDepth(depth);
		log.info("Depth: ", static_cast<uint32>(depth), " Val: ", depthInfo, " X:", (depthInfo >> 2) & 0x01, " Y:", (depthInfo >> 1) & 0x01, " Z:", (depthInfo >> 0) & 0x01);
	};

	akc::OctreeLocation loc(akm::Vec3(65535/856.0f * 634, 65535/284.0f * 142, 65535/867.0f * 534));
	for(auto i = 0u; i < 16; i++) printDepth(i, loc);
	return 0;

	log.info("Engine started.");
	printLogHeader(log);

	if (ake::config().exists("window")) {
		akw::open(akd::deserialize<akw::WindowOptions>(ake::config()["window"]));
	} else {
		auto defaultWindowOptions = akw::WindowOptions().glVSync(akw::VSync::FULL);
		akw::open(defaultWindowOptions);
	}

	akw::setCursorMode(akw::CursorMode::Captured);
	akr::init();

	// Skybox
		akr::ShaderProgram shaderSkybox = buildShaderProgram({
			{akr::StageType::Vertex, "shaders/skybox.vert"},
			{akr::StageType::Fragment, "shaders/skybox.frag"},
		});

		akr::Texture texSkybox = loadTextureCM(akfs::SystemFolder::appData, "textures/cubemap_snowy_street.aktex");

		akr::VertexArray vaSkybox; {
			vaSkybox.enableVAttrib(0);
			vaSkybox.setVAttribFormat(0, 2, akr::DataType::Single);
		}

		fpSingle arrSkyboxVerts[] = {-1, -1,  1, -1,  1,  1, /** 2 **/  1,  1, -1,  1, -1, -1};
		akr::Buffer vbufSkybox(arrSkyboxVerts, 12*sizeof(fpSingle));
		vaSkybox.bindVertexBuffer(0, vbufSkybox, 2*sizeof(fpSingle));

	// Setup Render
		// Pipeline
		akr::ShaderProgram shaderMesh = buildShaderProgram({
			{akr::StageType::Vertex, "shaders/main.vert"},
			{akr::StageType::Fragment, "shaders/main.frag"},
		});

		// VAO
		akr::VertexArray vaMesh;
		vaMesh.enableVAttribs({0, 1, 2, 3, 4, 5, 6});
		vaMesh.setVAttribFormats({0, 1, 2, 3}, 3, akr::DataType::Single);
		vaMesh.setVAttribFormat(4, 2, akr::DataType::Single);
		vaMesh.setVAttribFormat(5, 4, akr::IDataType::UInt16);
		vaMesh.setVAttribFormat(6, 4, akr::DataType::Single);

		// Mesh
		aka::Mesh mesh = readMeshFile<aka::Mesh>("meshes/Human.akmesh");

		// VBO
		akr::Buffer vbufMeshVerts(mesh.vertexData().data(), mesh.vertexData().size()*sizeof(aka::VertexData));
		vaMesh.bindVertexBuffer(0, vbufMeshVerts, sizeof(aka::VertexData), offsetof(aka::VertexData, position));
		vaMesh.bindVertexBuffer(1, vbufMeshVerts, sizeof(aka::VertexData), offsetof(aka::VertexData, tangent));
		vaMesh.bindVertexBuffer(2, vbufMeshVerts, sizeof(aka::VertexData), offsetof(aka::VertexData, bitangent));
		vaMesh.bindVertexBuffer(3, vbufMeshVerts, sizeof(aka::VertexData), offsetof(aka::VertexData, normal));
		vaMesh.bindVertexBuffer(4, vbufMeshVerts, sizeof(aka::VertexData), offsetof(aka::VertexData, texCoord));

		aka::Skeleton skele = readMeshFile<aka::Skeleton>("meshes/Human.akskel");
		auto poseData = aka::createPoseData(skele, mesh);
		akr::Buffer vbufMeshPose(poseData.data(), poseData.size()*sizeof(aka::PoseData));
		vaMesh.bindVertexBuffer(5, vbufMeshPose, sizeof(aka::PoseData), offsetof(aka::PoseData, boneIndicies));
		vaMesh.bindVertexBuffer(6, vbufMeshPose, sizeof(aka::PoseData), offsetof(aka::PoseData, boneWeights));

		akr::Buffer ibufMesh(mesh.indexData().data(), mesh.indexData().size()*sizeof(aka::IndexData));
		vaMesh.bindIndexBuffer(ibufMesh);

		akr::Buffer ubufMeshBones(skele.finalTransform().data(), sizeof(akm::Mat4)*skele.finalTransform().size(), akr::BufferHint_Dynamic);

		// Tex
		auto texMeshAlbedo   = loadTexture(akfs::SystemFolder::appData, "textures/brick_albedo.aktex");
		auto texMeshNormal   = loadTexture(akfs::SystemFolder::appData, "textures/brick_norm.aktex");
		auto texMeshSpecular = loadTexture(akfs::SystemFolder::appData, "textures/brick_spec.aktex");

		aka::Animation anim = readMeshFile<aka::Animation>("meshes/Human.akanim");
	// Setup Finish

	ake::FPSCamera camera;

	aka::AnimPoseMap poseMap(skele, anim);

	auto renderFunc = [&](fpSingle delta){
		static aku::FPSCounter fps;
		static fpSingle time = 0;
		time += delta;

		auto skeleBones = skele.bones();
		aka::applyPose(time*4, skeleBones, anim, poseMap);
		auto skeleTransform = aka::calculateFinalTransform(skele.rootID(), skeleBones);
		ubufMeshBones.writeData(skeleTransform.data(), sizeof(akm::Mat4)*skeleTransform.size());

		auto lookPos = camera.position();
		auto lookRot = akm::mat4_cast(camera.oritentation());

		// Prepare
		akr::setClearColour(0.2f, 0.2f, 0.2f);
		akr::clear();

		// Skybox
			akr::enableDepthTest(false);
			akr::enableCullFace(false);

			shaderSkybox.setUniform(0, akm::perspective<fpSingle>(1.0472f, akw::size().x/static_cast<fpSingle>(akw::size().y), 0.1f, 100.0f));
			shaderSkybox.setUniform(1, akm::transpose(lookRot)*akm::translate(-lookPos));
			shaderSkybox.setUniform(3, 0);

			akr::bindShaderProgram(shaderSkybox);
			akr::bindVertexArray(vaSkybox);
			akr::bindTexture(0, texSkybox);
			akr::draw(akr::DrawType::Triangles, 6);

		// Scene
			akr::enableDepthTest(true);
			akr::enableCullFace(true);

			// Setup Shader
			shaderMesh.setUniform(0, akm::perspective<fpSingle>(1.0472f, akw::size().x/static_cast<fpSingle>(akw::size().y), 0.1f, 100.0f));
			shaderMesh.setUniform(1, akm::transpose(lookRot)*akm::translate(-lookPos));
			shaderMesh.setUniform(2, akm::translate(akm::Vec3(0, 0, 5)));
			shaderMesh.setUniform(3, 0);
			shaderMesh.setUniform(4, lookPos);
			shaderMesh.setUniform(5, 1);
			shaderMesh.setUniform(6, 2);
			shaderMesh.setUniform(7, skele.getIndexedBone(skele.rootID()).data.nodeMatrix);

			// Draw Controlled Cube
			akr::bindBuffer(akr::BufferTarget::UNIFORM, ubufMeshBones, 0);
			akr::bindShaderProgram(shaderMesh);
			akr::bindVertexArray(vaMesh);
			akr::bindTexture(0, texMeshAlbedo);
			akr::bindTexture(1, texMeshNormal);
			akr::bindTexture(2, texMeshSpecular);
			akr::drawIndexed(akr::DrawType::Triangles, akr::IDataType::UInt16, mesh.indexData().size()*3, 0);

		// Draw Floor
		for(int i = 0; i < 32; i++) for(int j = 0; j < 32; j++) {
			shaderMesh.setUniform(2, akm::translate(akm::Vec3(i-16, -2, j)));
			akr::drawIndexed(akr::DrawType::Triangles, akr::IDataType::UInt16, mesh.indexData().size()*3, 0);
		}

		// Finish
		akw::swapBuffer();

		fps.update();
		std::stringstream sstream;
		sstream << fps.fps() << "fps";
		akw::setTitle(sstream.str());

	};

	auto updateFunc = [&](fpSingle delta){
		akw::pollEvents();
		akw::mouse().update();
		akw::keyboard().update();

		if (akw::keyboard().wasPressed(akin::Key::LALT)) akw::setCursorMode(akw::cursorMode() == akw::CursorMode::Captured ? akw::CursorMode::Normal : akw::CursorMode::Captured);


		if (akw::cursorMode() == akw::CursorMode::Captured) {
			auto mPos = akw::mouse().deltaPosition();
			camera.lookLR(akm::degToRad( mPos.x)/20.0f*(camera.up().y >=0 ? 1 : -1));
			camera.lookUD(akm::degToRad(-mPos.y)/20.0f);
		}

		if (akw::keyboard().isDown(akin::Key::W)) camera.moveForward( 1*delta);
		if (akw::keyboard().isDown(akin::Key::S)) camera.moveForward(-1*delta);
		if (akw::keyboard().isDown(akin::Key::D)) camera.moveRight( 1*delta);
		if (akw::keyboard().isDown(akin::Key::A)) camera.moveRight(-1*delta);
		if (akw::keyboard().isDown(akin::Key::R)) camera.moveUp( 1*delta);
		if (akw::keyboard().isDown(akin::Key::F)) camera.moveUp(-1*delta);
	};

	fpSingle updateAccum = 0;
	fpSingle updateDelta = 1/ake::config()["engine"]["ticksPerSecond"].asOrDef<fpSingle>(60.0f);
	fpSingle renderDelta = 1/static_cast<fpSingle>(akw::currentMonitor().prefVideoMode.refreshRate);

	aku::Timer timer;
	while(!akw::closeRequested()) {
		timer.mark();
		updateAccum += timer.nsecs()*1.0e-9;
		timer.reset();

		while(updateAccum >= updateDelta) {
			updateAccum -= updateDelta;
			updateFunc(updateDelta);
		}

		renderFunc(renderDelta);

	}

	log.info("Exiting main function");

	return 0;
}

static void printLogHeader(const ak::log::Logger& logger) {
	auto utc = aku::utcTimestamp();

	std::stringstream dateStream;
	dateStream << std::put_time(&utc.ctime, "%Y-%m-%d");

	std::stringstream timeStream;
	timeStream << std::put_time(&utc.ctime, "%H:%M:%S") << "." << std::setw(3) << std::setfill('0') << utc.milliseconds;

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
	constexpr ak::log::Logger log(AK_STRING_VIEW("Config"));

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
	constexpr ak::log::Logger startLog(AK_STRING_VIEW("Start"));

	akt::current().setName("Main");

	startLog.info("Loading engine config.");
	startupConfig();

	startLog.info("Starting log system.");
	ak::log::startProcessing();
	//ak::log::enableFileOutput();

	startLog.info("Starting window system.");
	akw::init();

	return [](){
		constexpr ak::log::Logger stopLog(AK_STRING_VIEW("Stop"));

		stopLog.info("Saving config.");
		if (!ake::saveConfig()) stopLog.warn("Failed to save config.");

		stopLog.info("Flushing log system.");
		ak::log::stopProcessing();
		ak::log::processMessageQueue();
		ak::log::processMessageQueue();
	};
}

static akr::ShaderProgram buildShaderProgram(const std::vector<std::pair<akr::StageType, std::string>>& stages) {
	akr::ShaderProgram program;

	for(auto& stageInfo : stages) {
		auto shaderFile = akfs::open(akfs::SystemFolder::appData, stageInfo.second, akfs::OpenFlags::In);
		if (!shaderFile) throw std::runtime_error(ak::buildString("buildShaderProgram: Could not open file: ", stageInfo.second));

		std::string source;
		if (!shaderFile.readAllLines(source)) throw std::runtime_error(ak::buildString("buildShaderProgram: Could not read data from file: ", stageInfo.second));

		akr::ShaderStage stage(stageInfo.first);
		if (!stage.attach(source)) throw std::runtime_error(ak::buildString("buildShaderProgram: Could not attach source to shader, see log for more information."));
		if (!stage.compile()) throw std::runtime_error(ak::buildString("buildShaderProgram: Could not compile shader. Error log:\n", stage.compileLog()));

		if (!program.attach(stage)) throw std::runtime_error(ak::buildString("buildShaderProgram: Could not attach shader to program."));
	}

	if (!program.link()) throw std::runtime_error(ak::buildString("buildShaderProgram: Failed to link shader. Error log:\n", program.linkLog()));
	return program;
}

static akr::Texture loadTexture(akfs::SystemFolder folder, const stx::filesystem::path& path) {
	auto textureFile = akfs::open(folder, path, akfs::OpenFlags::In);
	if (!textureFile) throw std::runtime_error("Could not open texture file.");

	std::vector<uint8> textureData;
	if (!textureFile.readAll(textureData)) throw std::runtime_error("Could not read texture file.");
	textureData = akd::decompressBrotli(textureData);

	akd::PValue textureConfig;
	if (!akd::fromMsgPack(textureConfig, textureData)) throw std::runtime_error("Could not parse texture file.");

	std::optional<akr::Texture> tex;

	if (textureConfig["hdr"].asBool()) {
		akd::ImageF32 image;
		if (!akd::deserialize(textureConfig["image"], image)) throw std::runtime_error("Could not deserialize hdr image file");
		tex = akr::createTex2D(0, akr::TexStorage::Single, image);
		if (!tex) throw std::runtime_error("Could not create texture.");
	} else {
		akd::ImageU8 image;
		if (!akd::deserialize(textureConfig["image"], image)) throw std::runtime_error("Could not deserialize ldr image file");
		tex = akr::createTex2D(0, akr::TexStorage::Byte, image);
		if (!tex) throw std::runtime_error("Could not create texture.");
	}

	akr::setTexFilters(akr::TexTarget::Tex2D, akr::FilterType::Linear, akr::FilterType::Linear, akr::FilterType::Linear);
	akr::genTexMipmaps(akr::TexTarget::Tex2D);

	return std::move(*tex);
}

static akr::Texture loadTextureCM(akfs::SystemFolder folder, const stx::filesystem::path& path) {
	auto textureFile = akfs::open(folder, path, akfs::OpenFlags::In);
	if (!textureFile) throw std::runtime_error("Could not open texture file.");

	std::vector<uint8> textureData;
	if (!textureFile.readAll(textureData)) throw std::runtime_error("Could not read texture file.");
	textureData = akd::decompressBrotli(textureData);

	akd::PValue textureConfig;
	if (!akd::fromMsgPack(textureConfig, textureData)) throw std::runtime_error("Could not parse texture file.");

	std::optional<akr::Texture> tex;

	if (textureConfig["hdr"].asBool()) {
		akd::ImageF32 image;
		if (!akd::deserialize(textureConfig["image"], image)) throw std::runtime_error("Could not deserialize image file");
		tex = akr::createTexCubemap(0, akr::TexStorage::Single, image);
		if (!tex) throw std::runtime_error("Could not create texture.");
	} else {
		akd::ImageU8 image;
		if (!akd::deserialize(textureConfig["image"], image)) throw std::runtime_error("Could not deserialize image file");
		tex = akr::createTexCubemap(0, akr::TexStorage::Byte, image);
		if (!tex) throw std::runtime_error("Could not create texture.");
	}

	akr::setTexFilters(akr::TexTarget::TexCubemap, akr::FilterType::Linear, akr::FilterType::Linear, akr::FilterType::Linear);
	akr::genTexMipmaps(akr::TexTarget::TexCubemap);

	return std::move(*tex);
}

template<typename type_t> static type_t readMeshFile(const std::string& filename) {
	auto file = akfs::open(akfs::SystemFolder::appData, filename, akfs::OpenFlags::In);
	if (file) {
		std::vector<uint8> compressedData;
		compressedData.resize(file.sizeOnDisk());
		if (!file.read(compressedData.data(), compressedData.size())) throw std::runtime_error("Failed to read file.");
		auto data = akd::decompressBrotli(compressedData);
		akd::PValue dTree;
		if (!akd::fromMsgPack(dTree, data)) throw std::runtime_error("Failed to parse file.");

		type_t result;
		if (!akd::deserialize(result, dTree)) throw std::runtime_error("Failed to deserialize file.");
		return result;
	} else {
		throw std::runtime_error("Failed to load mesh.");
	}
}

static const auto cb = ake::regenerateConfigDispatch().subscribe([](ake::RegenerateConfigEvent& ev){
	auto& config = ev.data()["engine"];
	config["ticksPerSecond"].set<fpSingle>(60.0f);
});
