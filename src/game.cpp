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
#include <ak/container/SparseGrid.hpp>
#include <ak/data/Brotli.hpp>
#include <ak/data/Image.hpp>
#include <ak/data/MsgPack.hpp>
#include <ak/data/PValue.hpp>
#include <ak/engine/components/Behaviours.hpp>
#include <ak/engine/components/Camera.hpp>
#include <ak/engine/components/Transform.hpp>
#include <ak/engine/Config.hpp>
#include <ak/engine/EntityManager.hpp>
#include <ak/engine/Scene.hpp>
#include <ak/engine/SceneManager.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/event/Event.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/Path.hpp>
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
#include <ak/render/gl/Buffers.hpp>
#include <ak/render/DebugDraw.hpp>
#include <ak/render/gl/Draw.hpp>
#include <ak/render/gl/RenderTarget.hpp>
#include <ak/render/gl/Shaders.hpp>
#include <ak/render/gl/Textures.hpp>
#include <ak/render/gl/Types.hpp>
#include <ak/render/gl/VertexArrays.hpp>
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
#include <akgame/CameraControllerBehaviour.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>
#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

int akGameMain();

static void printLogHeader(const ak::log::Logger& logger);
static ak::ScopeGuard startup();

static akr::gl::ShaderProgram buildShaderProgram(const std::vector<std::pair<akr::gl::StageType, std::string>>& stages);
static akr::gl::Texture loadTexture(const akfs::Path& path);
static akr::gl::Texture loadTextureCM(const akfs::Path& path);

template<typename type_t> static type_t readMeshFile(const akfs::Path& filename);

static void startGame();

int akGameMain() {
	constexpr ak::log::Logger log(AK_STRING_VIEW("Main"));
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

	startGame();

	log.info("Exiting main function");

	return 0;
}

static void startGame() {
	//constexpr ak::log::Logger log(AK_STRING_VIEW("Main"));

	ake::SceneManager sceneManager;
	auto& scene = sceneManager.getScene(sceneManager.newScene("World"));
	auto& ecs = scene.entities();

	//if (!ecs.registerComponentManager(std::make_unique<ake::SceneGraphManager>())) throw std::runtime_error("Failed to register SceneGraphComponent.");
	if (!ecs.registerComponentManager(std::make_unique<ake::TransformManager>())) throw std::runtime_error("Failed to register TransformComponent.");
	if (!ecs.registerComponentManager(std::make_unique<ake::CameraManager>())) throw std::runtime_error("Failed to register CameraManager.");
	if (!ecs.registerComponentManager(std::make_unique<ake::BehavioursManager>())) throw std::runtime_error("Failed to register BehavioursManager.");

	auto objID  = ecs.newEntity("TestObject");
	objID.createComponent<ake::Transform>(akm::Vec3(23.5f, 127.5f, 12.5f));

	akc::sparsegrid::SparseGrid<int> octree(akm::Vec3(0,0,0), akm::Vec3(1,1,1));
	octree.insert(0, akm::Vec3(1.5f,    20.5f, 1.5f), akm::Vec3(.5,.5,.5));
	octree.insert(0, akm::Vec3(23.5f,   34.f,  23.f), akm::Vec3(.5,.5,.5));
	octree.insert(0, akm::Vec3(23.5f, 127.5f, 12.5f), akm::Vec3(.5,.5,.5));

	auto   baseEID = ecs.newEntity("Base");               baseEID.createComponent<ake::Transform>(akm::Vec3{0,0,0});
	auto child1EID = ecs.newEntity("Child",   baseEID); child1EID.createComponent<ake::Transform>(akm::Vec3{5,0,0});
	auto child2EID = ecs.newEntity("Child", child1EID); child2EID.createComponent<ake::Transform>(akm::Vec3{2.5,0,0});

	auto cameraEID = ecs.newEntity("Camera");
	cameraEID.createComponent<ake::Transform>(akm::Vec3{0,0,0});
	cameraEID.createComponent<ake::Camera>();
	cameraEID.createComponent<ake::Behaviours>();
	cameraEID.component<ake::Camera>().setPerspectiveH(akm::degToRad(90), akw::size(), {0.1f, 65525.0f});
	cameraEID.component<ake::Behaviours>().addBehaviour(std::make_unique<akgame::CameraControllerBehaviour>());

	// Render target
		akr::gl::RenderTarget renderTarget;
		akr::gl::RenderBuffer /*colourBuffer,*/ depthBuffer;
		akr::gl::Texture colourTex(akr::gl::TexTarget::Tex2D);
		akr::gl::bindTexture(0, colourTex);
		akr::gl::newTexStorage2D(akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte, akw::size().x, akw::size().y, 1);
		akr::gl::newRenderBufferDepthStorage(depthBuffer, akr::gl::DepthStorage::SIZE_32F, akw::size().x, akw::size().y, 0);
		akr::gl::attachColourTexture(renderTarget, 0, colourTex);
		akr::gl::attachDepthBuffer(renderTarget, depthBuffer);

		akr::gl::ShaderProgram shaderPost = buildShaderProgram({
			{akr::gl::StageType::Vertex,   "data/shaders/post.vert"},
			{akr::gl::StageType::Fragment, "data/shaders/post.frag"},
		});

	// Skybox
		akr::gl::ShaderProgram shaderSkybox = buildShaderProgram({
			{akr::gl::StageType::Vertex,   "data/shaders/skybox.vert"},
			{akr::gl::StageType::Fragment, "data/shaders/skybox.frag"},
		});

		akr::gl::Texture texSkybox = loadTextureCM("data/textures/cubemap_snowy_street.aktex");

		akr::gl::VertexArray vaSkybox; {
			vaSkybox.enableVAttrib(0);
			vaSkybox.setVAttribFormat(0, 2, akr::gl::DataType::Single);
		}

		fpSingle arrSkyboxVerts[] = {-1, -1,  1, -1,  1,  1, /** 2 **/  1,  1, -1,  1, -1, -1};
		akr::gl::Buffer vbufSkybox(arrSkyboxVerts, 12*sizeof(fpSingle));
		vaSkybox.bindVertexBuffer(0, vbufSkybox, 2*sizeof(fpSingle));

	// Anim
		// Pipeline
		akr::gl::ShaderProgram shaderMesh = buildShaderProgram({{akr::gl::StageType::Vertex, "data/shaders/main.vert"}, {akr::gl::StageType::Fragment, "data/shaders/main.frag"}});

		// VAO
		akr::gl::VertexArray vaMesh;
		vaMesh.enableVAttribs({0, 1, 2, 3, 4, 5, 6});
		vaMesh.setVAttribFormats({0, 1, 2, 3}, 3, akr::gl::DataType::Single);
		vaMesh.setVAttribFormat(4, 2, akr::gl::DataType::Single);
		vaMesh.setVAttribFormat(5, 4, akr::gl::IDataType::UInt16);
		vaMesh.setVAttribFormat(6, 4, akr::gl::DataType::Single);

		// Mesh
		aka::Mesh mesh = readMeshFile<aka::Mesh>("data/meshes/Human.akmesh");

		// VBO
		akr::gl::Buffer vbufMeshVerts(mesh.vertexData().data(), mesh.vertexData().size()*sizeof(aka::VertexData));
		vaMesh.bindVertexBuffer(0, vbufMeshVerts, sizeof(aka::VertexData), offsetof(aka::VertexData, position));
		vaMesh.bindVertexBuffer(1, vbufMeshVerts, sizeof(aka::VertexData), offsetof(aka::VertexData, tangent));
		vaMesh.bindVertexBuffer(2, vbufMeshVerts, sizeof(aka::VertexData), offsetof(aka::VertexData, bitangent));
		vaMesh.bindVertexBuffer(3, vbufMeshVerts, sizeof(aka::VertexData), offsetof(aka::VertexData, normal));
		vaMesh.bindVertexBuffer(4, vbufMeshVerts, sizeof(aka::VertexData), offsetof(aka::VertexData, texCoord));

		aka::Skeleton skele = readMeshFile<aka::Skeleton>("data/meshes/Human.akskel");
		auto poseData = aka::createPoseData(skele, mesh);
		akr::gl::Buffer vbufMeshPose(poseData.data(), poseData.size()*sizeof(aka::PoseData));
		vaMesh.bindVertexBuffer(5, vbufMeshPose, sizeof(aka::PoseData), offsetof(aka::PoseData, boneIndicies));
		vaMesh.bindVertexBuffer(6, vbufMeshPose, sizeof(aka::PoseData), offsetof(aka::PoseData, boneWeights));

		akr::gl::Buffer ibufMesh(mesh.indexData().data(), mesh.indexData().size()*sizeof(aka::IndexData));
		vaMesh.bindIndexBuffer(ibufMesh);

		akr::gl::Buffer ubufMeshBones(skele.finalTransform().data(), sizeof(akm::Mat4)*skele.finalTransform().size(), akr::gl::BufferHint_Dynamic);

		// Tex
		auto texMeshAlbedo   = loadTexture("data/textures/brick_albedo.aktex");
		auto texMeshNormal   = loadTexture("data/textures/brick_norm.aktex");
		auto texMeshSpecular = loadTexture("data/textures/brick_spec.aktex");

		aka::Animation anim = readMeshFile<aka::Animation>("data/meshes/Human.akanim");
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

	//ake::FPSCamera camera;
	static bool updating = true;
	static bool drawBoxes = true;

	/*ecs.getComponentType<akec::Transform3D>().transformModifiedEvent(child1EID).subscribe([&](akec::TransformModifiedEvent& event){
		camera.position() = ecs.getComponent<akec::Transform3D>(event.data().modifiedEntity()).position();
	});*/

	scene.renderEvent().subscribe([&](ake::SceneRenderEvent& renderEventData){
		akr::gl::bindRenderTarget(renderTarget);

		fpDouble delta = renderEventData.data();

		static fpSingle time = 0;
		time += delta;

		auto skeleBones = skele.bones();
		aka::applyPose(time*4, skeleBones, anim, poseMap);
		auto skeleTransform = aka::calculateFinalTransform(skele.rootID(), skeleBones);
		ubufMeshBones.writeData(skeleTransform.data(), sizeof(akm::Mat4)*skeleTransform.size());

		auto cameraTransform = cameraEID.component<ake::Transform>();

		auto lookPos = cameraTransform.position();
		auto projMatrix = cameraEID.component<ake::Camera>().projectionMatrix();//akm::perspectiveV(1.0472f, akw::size().x, akw::size().y, 0.1f, 65536.0f);
		auto viewMatrix = cameraEID.component<ake::Camera>().viewMatrix();

		// Prepare
		akr::gl::setClearColour(0.2f, 0.2f, 0.2f);
		akr::gl::clear();

		// Skybox
			akr::gl::enableDepthTest(false);
			akr::gl::enableCullFace(false);

			shaderSkybox.setUniform(0, projMatrix);
			shaderSkybox.setUniform(1, viewMatrix);
			shaderSkybox.setUniform(3, 0);

			akr::gl::bindShaderProgram(shaderSkybox);
			akr::gl::bindVertexArray(vaSkybox);
			akr::gl::bindTexture(0, texSkybox);
			akr::gl::draw(akr::gl::DrawType::Triangles, 6);

		// Scene
			akr::gl::enableDepthTest(true);
			akr::gl::enableCullFace(true);

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
			akr::gl::bindBuffer(akr::gl::BufferTarget::UNIFORM, ubufMeshBones, 0);
			akr::gl::bindShaderProgram(shaderMesh);
			akr::gl::bindVertexArray(vaMesh);
			akr::gl::bindTexture(0, texMeshAlbedo);
			akr::gl::bindTexture(1, texMeshNormal);
			akr::gl::bindTexture(2, texMeshSpecular);
			akr::gl::drawIndexed(akr::gl::DrawType::Triangles, akr::gl::IDataType::UInt16, mesh.indexData().size()*3, 0);

		// Test
			akr::gl::enableDepthTest(true);
			akr::gl::enableCullFace(true);

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
				static akm::Vec3 rayPos = lookPos;
				static akm::Vec3 rayDir = cameraTransform.forward();
				if (updating) {
					rayPos = lookPos;
					rayDir = cameraTransform.forward();
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

			// ECS Test
			akrd::pushMatrix(akrd::Matrix::Model);
				akrd::setColour({0.25, 0.25,   1});
				akrd::setMatrix(akrd::Matrix::Model, baseEID.component<ake::Transform>().localToWorld());
				akrd::draw(dlFilledCube);

				akrd::setColour({  1, 0.25, 0.25});
				akrd::setMatrix(akrd::Matrix::Model, child1EID.component<ake::Transform>().localToWorld());
				akrd::draw(dlFilledCube);

				akrd::setColour({0.25,   1, 0.25});
				akrd::setMatrix(akrd::Matrix::Model, child2EID.component<ake::Transform>().localToWorld());
				akrd::draw(dlFilledCube);
			akrd::popMatrix(akrd::Matrix::Model);

		akr::gl::bindDisplayRenderTarget();
		//akr::gl::blitRenderTargetToDisplay(renderTarget, {0,0}, akw::size(), {0,0}, akw::size(), akr::gl::BlitMask::Colour, akr::gl::FilterType::Linear);

		akr::gl::clear(akr::gl::ClearMode::All);
		akr::gl::enableDepthTest(false);
		akr::gl::enableCullFace(false);

		akr::gl::bindShaderProgram(shaderPost);
		shaderPost.setUniform(1, 0);
		shaderPost.setUniform(2, akw::size().x);
		shaderPost.setUniform(3, akw::size().y);
		akr::gl::bindVertexArray(vaSkybox);
		akr::gl::bindTexture(0, colourTex);
		akr::gl::draw(akr::gl::DrawType::Triangles, 6);

		// Finish
		akw::swapBuffer();
	});

	scene.updateEvent().subscribe([&](ake::SceneUpdateEvent& updateEventData){
		fpDouble delta = updateEventData.data();

		static fpSingle time = 0;
		time += delta;

		auto cameraTransform = cameraEID.component<ake::Transform>();

		if (akw::keyboard().wasPressed(akin::Key::H)) updating = !updating;
		if (akw::keyboard().wasPressed(akin::Key::B)) drawBoxes = !drawBoxes;

		if (drawBoxes) baseEID.component<ake::Transform>().setRotation(akm::rotateQ(time, akm::Vec3(0,0,1)));

		if (drawBoxes) child1EID.component<ake::Transform>().setScale(akm::abs(akm::sin(time)));
		if (drawBoxes) child1EID.component<ake::Transform>().setRotation(akm::rotateQ(time*4, akm::Vec3(0,0,1)));

		if (!drawBoxes) child2EID.component<ake::Transform>().setPosition(cameraTransform.forward()*5.f + cameraTransform.position()); //(akm::Vec3(2.5, 0, 0) + akm::Vec3(akm::sin(time)*1.5, 0, 0));
		if (!drawBoxes) child2EID.component<ake::Transform>().setRotation(cameraTransform.rotationQuat());

		if (!drawBoxes) child2EID.component<ake::Transform>().setScale(1.f);

		if (akw::keyboard().wasPressed(akin::Key::J)) child2EID.setParent(baseEID);
		if (akw::keyboard().wasPressed(akin::Key::N)) child2EID.setParent(child1EID);
	});

	fpSingle updateAccum = 0.f;
	fpSingle updateDelta = 1.f/ake::config()["engine"]["ticksPerSecond"].asOrDef<fpSingle>(60.f);
	fpSingle renderDelta = 1.f/static_cast<fpSingle>(akw::currentMonitor().prefVideoMode.refreshRate);

	aku::FPSCounter fps(10);
	aku::FPSCounter tps(10);
	aku::Timer loopTimer;
	//aku::Timer renderTimer;
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
		//renderFunc(fps.update().avgTickDelta());

		std::stringstream sstream;
		sstream << fps.avgTicksPerSecond() << "fps | " << tps.avgTicksPerSecond() << "tps";
		akw::setTitle(sstream.str());

		updateAccum += loopTimer.mark().secsf();
		loopTimer.reset();
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

	startLog.info("Starting ECS system.");
	//if (!ake::initEntityComponentSystem()) throw std::logic_error("Failed to initialize entity component system.");

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

static akr::gl::ShaderProgram buildShaderProgram(const std::vector<std::pair<akr::gl::StageType, std::string>>& stages) {
	akr::gl::ShaderProgram program;

	for(auto& stageInfo : stages) {
		auto shaderFile = akfs::CFile(stageInfo.second, akfs::OpenFlags::In);
		if (!shaderFile) throw std::runtime_error(ak::buildString("buildShaderProgram: Could not open file: ", stageInfo.second));

		std::string source;
		if (!shaderFile.readAllLines(source)) throw std::runtime_error(ak::buildString("buildShaderProgram: Could not read data from file: ", stageInfo.second));

		akr::gl::ShaderStage stage(stageInfo.first);
		if (!stage.attach(source)) throw std::runtime_error(ak::buildString("buildShaderProgram: Could not attach source to shader, see log for more information."));
		if (!stage.compile()) throw std::runtime_error(ak::buildString("buildShaderProgram: Could not compile shader. Error log:\n", stage.compileLog()));

		if (!program.attach(stage)) throw std::runtime_error(ak::buildString("buildShaderProgram: Could not attach shader to program."));
	}

	if (!program.link()) throw std::runtime_error(ak::buildString("buildShaderProgram: Failed to link shader. Error log:\n", program.linkLog()));
	return program;
}

static akr::gl::Texture loadTexture(const akfs::Path& filename) {
	auto textureFile = akfs::CFile(filename, akfs::OpenFlags::In);
	if (!textureFile) throw std::runtime_error(ak::buildString("Failed to open texture: ", filename.str()));

	std::vector<uint8> textureData;
	if (!textureFile.readAll(textureData)) throw std::runtime_error(ak::buildString("Failed to read texture: ", filename.str()));
	textureData = akd::decompressBrotli(textureData);

	akd::PValue textureConfig;
	if (!akd::fromMsgPack(textureConfig, textureData)) throw std::runtime_error(ak::buildString("Failed to parse texture: ", filename.str()));

	std::optional<akr::gl::Texture> tex;

	if (textureConfig["hdr"].asBool()) {
		akd::ImageF32 image;
		if (!akd::deserialize(textureConfig["image"], image)) throw std::runtime_error(ak::buildString("Failed to deserialize texture: ", filename.str()));
		tex = akr::gl::createTex2D(0, akr::gl::TexStorage::Single, image);
		if (!tex) throw std::runtime_error("Could not create texture.");
	} else {
		akd::ImageU8 image;
		if (!akd::deserialize(textureConfig["image"], image)) throw std::runtime_error(ak::buildString("Failed to deserialize texture: ", filename.str()));
		tex = akr::gl::createTex2D(0, akr::gl::TexStorage::Byte, image);
		if (!tex) throw std::runtime_error("Could not create texture.");
	}

	akr::gl::setTexFilters(akr::gl::TexTarget::Tex2D, akr::gl::FilterType::Linear, akr::gl::FilterType::Linear, akr::gl::FilterType::Linear);
	akr::gl::genTexMipmaps(akr::gl::TexTarget::Tex2D);

	return std::move(*tex);
}

static akr::gl::Texture loadTextureCM(const akfs::Path& filename) {
	auto textureFile = akfs::CFile(filename, akfs::OpenFlags::In);
	if (!textureFile) throw std::runtime_error(ak::buildString("Failed to open texture: ", filename.str()));

	std::vector<uint8> textureData;
	if (!textureFile.readAll(textureData)) throw std::runtime_error(ak::buildString("Failed to read texture: ", filename.str()));
	textureData = akd::decompressBrotli(textureData);

	akd::PValue textureConfig;
	if (!akd::fromMsgPack(textureConfig, textureData)) throw std::runtime_error(ak::buildString("Failed to parse texture: ", filename.str()));

	std::optional<akr::gl::Texture> tex;

	if (textureConfig["hdr"].asBool()) {
		akd::ImageF32 image;
		if (!akd::deserialize(textureConfig["image"], image)) throw std::runtime_error(ak::buildString("Failed to deserialize texture: ", filename.str()));
		tex = akr::gl::createTexCubemap(0, akr::gl::TexStorage::Single, image);
		if (!tex) throw std::runtime_error("Could not create texture.");
	} else {
		akd::ImageU8 image;
		if (!akd::deserialize(textureConfig["image"], image)) throw std::runtime_error(ak::buildString("Failed to deserialize texture: ", filename.str()));
		tex = akr::gl::createTexCubemap(0, akr::gl::TexStorage::Byte, image);
		if (!tex) throw std::runtime_error("Could not create texture.");
	}

	akr::gl::setTexFilters(akr::gl::TexTarget::TexCubemap, akr::gl::FilterType::Linear, akr::gl::FilterType::Linear, akr::gl::FilterType::Linear);
	akr::gl::genTexMipmaps(akr::gl::TexTarget::TexCubemap);

	return std::move(*tex);
}

template<typename type_t> static type_t readMeshFile(const akfs::Path& filename) {
	auto file = akfs::CFile(filename, akfs::OpenFlags::In);
	if (!file) throw std::runtime_error(ak::buildString("Failed to open mesh: ", filename.str()));

	std::vector<uint8> compressedData;
	compressedData.resize(file.sizeOnDisk());
	if (!file.read(compressedData.data(), compressedData.size())) throw std::runtime_error(ak::buildString("Failed to read mesh: ", filename.str()));
	auto data = akd::decompressBrotli(compressedData);
	akd::PValue dTree;
	if (!akd::fromMsgPack(dTree, data)) throw std::runtime_error(ak::buildString("Failed to parse mesh: ", filename.str()));

	type_t result;
	if (!akd::deserialize(result, dTree)) throw std::runtime_error(ak::buildString("Failed to deserialize mesh: ", filename.str()));
	return result;
}

static const auto cb = ake::regenerateConfigDispatch().subscribe([](ake::RegenerateConfigEvent& ev){
	auto& config = ev.data()["engine"];
	config["ticksPerSecond"].set<fpSingle>(60.0f);
});
