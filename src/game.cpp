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

#include <ak/animation/Fwd.hpp>
#include <ak/animation/Mesh.hpp>
#include <ak/animation/MeshPoseData.hpp>
#include <ak/animation/Skeleton.hpp>
#include <ak/animation/Type.hpp>
#include <ak/animation/Serialize.hpp>
#include <ak/container/SlotMap.hpp>
#include <ak/container/SparseGrid.hpp>
#include <ak/data/Brotli.hpp>
#include <ak/data/Image.hpp>
#include <ak/data/MsgPack.hpp>
#include <ak/data/PValue.hpp>
#include <ak/engine/Camera.hpp>
#include <ak/engine/Config.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/event/Subscription.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/input/Keyboard.hpp>
#include <ak/input/Keys.hpp>
#include <ak/input/Mouse.hpp>
#include <ak/Log.hpp>
#include <ak/math/Matrix.hpp>
#include <ak/math/Quaternion.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/math/Types.hpp>
#include <ak/Macros.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/Buffers.hpp>
#include <ak/render/DebugDraw.hpp>
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
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <experimental/filesystem>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
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

static void startGame();

int akGameMain() {
	constexpr ak::log::Logger log(AK_STRING_VIEW("Main"));
	auto shutdownScope = startup();

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

	startGame();

	log.info("Exiting main function");

	return 0;
}

static void startGame() {
	constexpr ak::log::Logger log(AK_STRING_VIEW("Main"));

	akc::sparsegrid::SparseGrid<int> octree(akm::Vec3(0,0,0), akm::Vec3(1,1,1));
	octree.insert(0, akm::Vec3(1.5f, 20.5f, 1.5f), akm::Vec3(.5, .5, .5));
	octree.insert(0, akm::Vec3(23.5f, 34.f, 23.f), akm::Vec3(.5,.5,.5));
	octree.insert(0, akm::Vec3(23.5f, 255.5f, 12.5f), akm::Vec3(.5,.5,.5));

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

	// Anim
		// Pipeline
		akr::ShaderProgram shaderMesh = buildShaderProgram({{akr::StageType::Vertex, "shaders/main.vert"}, {akr::StageType::Fragment, "shaders/main.frag"}});

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
		aka::AnimPoseMap poseMap(skele, anim);

	// Display Lists
		akrd::DisplayList dlFilledCube;
		akrd::DisplayList dlLineCube;

		{
			akm::Vec3 verts[] = {{-0.5, -0.5, -0.5},{-0.5, -0.5,  0.5},{-0.5,  0.5, -0.5},{-0.5,  0.5,  0.5},{ 0.5, -0.5, -0.5},{ 0.5, -0.5,  0.5},{ 0.5,  0.5, -0.5},{ 0.5,  0.5,  0.5}};
			akm::Vec3 fv0[] = {verts[1], verts[3], verts[2], verts[0]}, fv1[] = {verts[0], verts[2], verts[6], verts[4]};
			akm::Vec3 fv2[] = {verts[4], verts[6], verts[7], verts[5]}, fv3[] = {verts[5], verts[7], verts[3], verts[1]};
			akm::Vec3 fv4[] = {verts[1], verts[0], verts[4], verts[5]}, fv5[] = {verts[2], verts[3], verts[7], verts[6]};

			dlFilledCube.begin(akrd::Primitive::Triangles)
				.addVertexPoly(4, fv0, {1, 0, 0})
				.addVertexPoly(4, fv1, {0, 0, 1})
				.addVertexPoly(4, fv2, {1, 0, 0})
				.addVertexPoly(4, fv3, {0, 0, 1})
				.addVertexPoly(4, fv4, {0, 1, 0})
				.addVertexPoly(4, fv5, {0, 1, 0})
			.end();

			dlLineCube.begin(akrd::Primitive::Lines)
				.addVertex(verts[1]).addVertex(verts[3])
				.addVertex(verts[2]).addVertex(verts[0])
				.addVertex(verts[4]).addVertex(verts[6])
				.addVertex(verts[7]).addVertex(verts[5])
				.addVertexPoly(4, fv4)
				.addVertexPoly(4, fv5)
			.end();
		}

	ake::FPSCamera camera;
	static bool updating = true;
	static bool drawBoxes = true;

	auto projMatrix = akm::perspectiveV(1.0472f, akw::size().x, akw::size().y, 0.1f, 65536.0f);
	auto renderFunc = [&](fpSingle delta){
		static fpSingle time = 0;
		time += delta;

		auto skeleBones = skele.bones();
		aka::applyPose(time*4, skeleBones, anim, poseMap);
		auto skeleTransform = aka::calculateFinalTransform(skele.rootID(), skeleBones);
		ubufMeshBones.writeData(skeleTransform.data(), sizeof(akm::Mat4)*skeleTransform.size());

		auto lookPos = camera.position();
		auto lookRot = akm::mat4_cast(camera.oritentation());

		auto viewMatrix = akm::transpose(lookRot)*akm::translate(-lookPos);

		// Prepare
		akr::setClearColour(0.2f, 0.2f, 0.2f);
		akr::clear();

		// Skybox
			akr::enableDepthTest(false);
			akr::enableCullFace(false);

			shaderSkybox.setUniform(0, projMatrix);
			shaderSkybox.setUniform(1, viewMatrix);
			shaderSkybox.setUniform(3, 0);

			akr::bindShaderProgram(shaderSkybox);
			akr::bindVertexArray(vaSkybox);
			akr::bindTexture(0, texSkybox);
			akr::draw(akr::DrawType::Triangles, 6);

		// Scene
			akr::enableDepthTest(true);
			akr::enableCullFace(true);

			// Setup Shader
			shaderMesh.setUniform(0, projMatrix);
			shaderMesh.setUniform(1, viewMatrix);
			shaderMesh.setUniform(2, akm::translate(akm::Vec3(0, -1.2f, 1)));
			shaderMesh.setUniform(3, 0);
			shaderMesh.setUniform(4, lookPos);
			shaderMesh.setUniform(5, 1);
			shaderMesh.setUniform(6, 2);
			shaderMesh.setUniform(7, skele.getIndexedBone(skele.rootID()).data.nodeMatrix);

			// Draw Anim
			akr::bindBuffer(akr::BufferTarget::UNIFORM, ubufMeshBones, 0);
			akr::bindShaderProgram(shaderMesh);
			akr::bindVertexArray(vaMesh);
			akr::bindTexture(0, texMeshAlbedo);
			akr::bindTexture(1, texMeshNormal);
			akr::bindTexture(2, texMeshSpecular);
			akr::drawIndexed(akr::DrawType::Triangles, akr::IDataType::UInt16, mesh.indexData().size()*3, 0);

		// Test
			akr::enableDepthTest(true);
			akr::enableCullFace(true);

			akrd::setMatrix(akrd::Matrix::Projection, projMatrix);
			akrd::setMatrix(akrd::Matrix::View, viewMatrix);

			// Sparse Grid
				akrd::setColour({1,1,1});
				akrd::pushMatrix(akrd::Matrix::Model, akm::translate(akm::Vec3(0,0,0)+octree.gridBounds()/2.f) * akm::scale(octree.gridBounds()));
					akrd::draw(dlLineCube);
				akrd::popMatrix(akrd::Matrix::Model);

				octree.iterate([&](akm::Vec3 pos, akm::Vec3 boxSize){
					akrd::pushMatrix(akrd::Matrix::Model, akm::translate(pos+boxSize/2.f) * akm::scale(boxSize));
						akrd::draw(dlLineCube);
					akrd::popMatrix(akrd::Matrix::Model);
				});

			// Raycast
				static akm::Vec3 rayPos = camera.position();
				static akm::Vec3 rayDir = camera.forward();
				if (updating) {
					rayPos = camera.position();
					rayDir = camera.forward();
				} else {
					akrd::setColour({0,0,0});
					akrd::DisplayList().begin(akrd::Primitive::Lines)
						.addVertex(rayPos).addVertex(rayPos + rayDir*32.f)
					.end().draw();
				}

				akrd::pushMatrix(akrd::Matrix::Model);
					octree.castLine(rayPos, rayDir, 32, [&](akm::Vec3 pos, akm::Vec3 enterPos, akm::Vec3 exitPos){
							if (drawBoxes) {
								akrd::setColour({1,1,1});
								akrd::setMatrix(akrd::Matrix::Model, akm::translate(pos + octree.cellBounds()/2.f) * akm::scale(octree.cellBounds()));
								akrd::draw(dlFilledCube);
							}

							akrd::setColour({0.25,0.25,1});
							akrd::setMatrix(akrd::Matrix::Model, akm::translate(enterPos) * akm::scale(akm::Vec3{0.05, 0.05, 0.05}));
							akrd::draw(dlFilledCube);

							akrd::setColour({1,0.25,0.25});
							akrd::setMatrix(akrd::Matrix::Model, akm::translate(exitPos) * akm::scale(akm::Vec3{0.05, 0.05, 0.05}));
							akrd::draw(dlFilledCube);
						return true;
					});
				akrd::popMatrix(akrd::Matrix::Model);

		// Finish
		akw::swapBuffer();
	};

	auto updateFunc = [&](fpSingle delta){
		akw::pollEvents();
		akw::mouse().update();
		akw::keyboard().update();

		static fpSingle time = 0;
		time += delta;

		if (akw::keyboard().wasPressed(akin::Key::LALT)) akw::setCursorMode(akw::cursorMode() == akw::CursorMode::Captured ? akw::CursorMode::Normal : akw::CursorMode::Captured);


		if (akw::cursorMode() == akw::CursorMode::Captured) {
			auto mPos = akw::mouse().deltaPosition();
			camera.lookLR(akm::degToRad( mPos.x)/20.0f*(camera.up().y >=0 ? 1 : -1));
			camera.lookUD(akm::degToRad(-mPos.y)/20.0f);
		}

		fpSingle moveSpeed = (akw::keyboard().isDown(akin::Key::LSHIFT) ? 40 : 5)*delta;

		if (akw::keyboard().isDown(akin::Key::W)) camera.moveForward( moveSpeed);
		if (akw::keyboard().isDown(akin::Key::S)) camera.moveForward(-moveSpeed);
		if (akw::keyboard().isDown(akin::Key::D)) camera.moveRight( moveSpeed);
		if (akw::keyboard().isDown(akin::Key::A)) camera.moveRight(-moveSpeed);
		if (akw::keyboard().isDown(akin::Key::R)) camera.moveUp( moveSpeed);
		if (akw::keyboard().isDown(akin::Key::F)) camera.moveUp(-moveSpeed);

		if (akw::keyboard().wasPressed(akin::Key::H)) updating = !updating;
		if (akw::keyboard().wasPressed(akin::Key::B)) drawBoxes = !drawBoxes;
	};

	fpSingle updateAccum = 0;
	fpSingle updateDelta = 1/ake::config()["engine"]["ticksPerSecond"].asOrDef<fpSingle>(60.0f);
	fpSingle renderDelta = 1/static_cast<fpSingle>(akw::currentMonitor().prefVideoMode.refreshRate);

	aku::FPSCounter fps;
	aku::FPSCounter tps;
	aku::Timer loopTimer;
	while(!akw::closeRequested()) {
		loopTimer.reset();

		while(updateAccum >= updateDelta) {
			aku::Timer updateTimer;

			updateFunc(updateDelta);
			tps.update();

			if (updateTimer.mark().secsf() > updateDelta) updateAccum = akm::mod(updateAccum, updateDelta);

			updateAccum -= updateDelta;
		}

		renderFunc(renderDelta);
		fps.update();

		std::stringstream sstream;
		sstream << fps.fps() << "fps | " << tps.fps() << "tps";
		akw::setTitle(sstream.str());

		updateAccum += loopTimer.mark().secsf();
	}
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
