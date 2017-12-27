#include <ak/data/Brotli.hpp>
#include <ak/data/Json.hpp>
#include <ak/data/MPac.hpp>
#include <ak/data/PValue.hpp>
#include <ak/engine/Camera.hpp>
#include <ak/engine/Config.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/event/Subscription.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/filesystem/TextureLoader.hpp>
#include <ak/input/Keyboard.hpp>
#include <ak/input/Keys.hpp>
#include <ak/input/Mouse.hpp>
#include <ak/Log.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/math/Vector.hpp>
#include <ak/Macros.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/Buffer.hpp>
#include <ak/render/Draw.hpp>
#include <ak/render/Pipeline.hpp>
#include <ak/render/Primitives.hpp>
#include <ak/render/Texture.hpp>
#include <ak/render/Types.hpp>
#include <ak/render/VertexMapping.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/thread/CurrentThread.hpp>
#include <ak/util/FPSCounter.hpp>
#include <ak/util/Time.hpp>
#include <ak/util/Timer.hpp>
#include <ak/window/Monitor.hpp>
#include <ak/window/Types.hpp>
#include <ak/window/Window.hpp>
#include <ak/window/WindowOptions.hpp>
#include <glm/detail/func_matrix.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


#if defined(__linux)
#define BACKWARD_HAS_BFD 1
#include <signal.h>
#ifndef SIGUNUSED
#define SIGUNUSED 31
#endif
#include "backward.hpp"
namespace backward { static backward::SignalHandling sh; }
#endif

#include <ak/animation/Animation.hpp>
#include <ak/animation/Mesh.hpp>
#include <ak/animation/Skeleton.hpp>
#include <ak/animation/Model.hpp>

static void printLogHeader(const ak::log::Logger& logger);
static ak::ScopeGuard startup();

static void createShaderProgram(akr::Pipeline& pipeline);


int main() {
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

	aka::Mesh mesh;

	auto file = akfs::open(akfs::SystemFolder::appData, "meshes/TestMesh.mpac.br", akfs::OpenFlags::In);
	if (file) {
		std::vector<uint8> compressedData;
		compressedData.resize(file.sizeOnDisk());
		file.read(compressedData.data(), compressedData.size());

		auto data = akd::decompressFromBrotli(compressedData);

		akd::PValue dTree;
		akd::deserializeFromMPac(dTree, data);
		log.info("Json Parse: ", akd::serializeAsJson(dTree, true));

		akd::deserialize(mesh, dTree);
		log.info("Verts: ", mesh.verts().size());
		log.info("Norms: ", mesh.norms().size());
		log.info("UVs: ", mesh.uvs().size());
		log.info("Faces: ", mesh.faces().size());
	}

	akw::setCursorMode(akw::CursorMode::Captured);

	// Setup Render
		// Init
		akr::init();

		akr::enableDepthTest(true);
		akr::enableCullFace(true);
		akr::setClearColour(0.2f, 0.2f, 0.2f);

		// Pipeline
		akr::Pipeline pipeline;
		createShaderProgram(pipeline);
		akr::setActivePipeline(pipeline);

		// VAO
		akr::VertexMapping vMapping;
		akr::bind(vMapping);

		// VBO
		akr::Buffer vBuf;
		akr::bind(akr::BufferTarget::VARRYING, vBuf);
		auto vData = mesh.buildBuffer(true, true, true);//akr::genCubeMesh();
		akr::setData(akr::BufferTarget::VARRYING, vData.data(), static_cast<uint32>(vData.size()));
		akr::mapVertexBufferF(0, 3, akr::DataType::FPSingle, false,                  0, 8*sizeof(fpSingle));
		akr::mapVertexBufferF(1, 3, akr::DataType::FPSingle, false, 3*sizeof(fpSingle), 8*sizeof(fpSingle));
		akr::mapVertexBufferF(2, 2, akr::DataType::FPSingle, false, 6*sizeof(fpSingle), 8*sizeof(fpSingle));

		// Tex
		akr::Texture tex = akfs::loadTexture2D(akfs::SystemFolder::appData, "textures/testTexture2D.json").texture;


	// Setup Finish

	// Skybox


		akr::Pipeline skyboxProgram;
		akr::PipelineStage cubeVStage(akr::StageType::Vertex);
		std::string source;
		akfs::open(akfs::SystemFolder::appData, "shaders/skybox.vert", akfs::OpenFlags::In).readLine(source, false, {});
		cubeVStage.attach(source);
		cubeVStage.compile();
		skyboxProgram.attach(cubeVStage);

		akr::PipelineStage cubeFStage(akr::StageType::Fragment);
		akfs::open(akfs::SystemFolder::appData, "shaders/skybox.frag", akfs::OpenFlags::In).readLine(source, false, {});
		cubeFStage.attach(source);
		cubeFStage.compile();
		skyboxProgram.attach(cubeFStage);

		skyboxProgram.link();

		akr::Texture cubeMap = akfs::loadTextureCubemap(akfs::SystemFolder::appData, "textures/testCubeMap.json").texture;

		akr::VertexMapping vSkybox;
		akr::bind(vSkybox);

		akr::Buffer vSkyboxBuffer;
		akr::bind(akr::BufferTarget::VARRYING, vSkyboxBuffer);
		fpSingle vData2[] = {
			-1, -1,
			 1, -1,
			 1,  1,

			 1,  1,
			-1,  1,
			-1, -1,
		};
		akr::setData(akr::BufferTarget::VARRYING, vData2, 12);
		akr::mapVertexBufferF(0, 2, akr::DataType::FPSingle);

	/*akm::Vec3 lookPos(0, 0, 0);
	akm::SphericalCoord_t<fpSingle> lookOrient;*/

	ake::FPSCamera camera;

	auto renderFunc = [&](fpSingle /*delta*/){
		static aku::FPSCounter fps;

		auto lookPos = camera.position();
		auto lookRot = akm::mat4_cast(camera.oritentation());

		// Prepare
		akr::clear();

		// Skybox
		akr::setActivePipeline(skyboxProgram);
		akr::enableDepthTest(false);
		akr::enableCullFace(false);

		akr::setUniform(0, akm::perspective<fpSingle>(1.0472f, akw::size().x/static_cast<fpSingle>(akw::size().y), 0.1f, 100.0f));
		akr::setUniform(1, akm::transpose(lookRot)*akm::translate(-lookPos));

		akr::bind(0, cubeMap);
		akr::setUniform(3, 0);

		akr::bind(vSkybox);
		akr::draw(akr::DrawType::Triangles, 6);

		// Scene
		akr::setActivePipeline(pipeline);
		akr::enableDepthTest(true);
		akr::enableCullFace(true);

		// Setup Cube
		akr::bind(vMapping);
		akr::bind(0, tex);
		akr::setUniform(0, akm::perspective<fpSingle>(1.0472f, akw::size().x/static_cast<fpSingle>(akw::size().y), 0.1f, 100.0f));
		akr::setUniform(1, akm::transpose(lookRot)*akm::translate(-lookPos));

		akr::setUniform(3, 0);

		// Draw Controlled Cube
		akr::setUniform(2, akm::translate(akm::Vec3(0, 0, 5)));
		akr::draw(akr::DrawType::Triangles, 36);

		// Draw Floor
		for(int i = 0; i < 32; i++) for(int j = 0; j < 32; j++) {
			akr::setUniform(2, akm::translate(akm::Vec3(i-16, -2, j)));
			akr::draw(akr::DrawType::Triangles, 36);
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

static void startupLog() {
	ak::log::startProcessing();
	//ak::log::enableFileOutput();
}

static void startupWindow() {
	akw::init();
}

static ak::ScopeGuard startup() {
	constexpr ak::log::Logger startLog(AK_STRING_VIEW("Start"));

	akt::current().setName("Main");

	startLog.info("Loading engine config.");
	startupConfig();

	startLog.info("Starting log system.");
	startupLog();

	startLog.info("Starting window system.");
	startupWindow();

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

static void createShaderProgram(akr::Pipeline& pipeline) {
	akr::PipelineStage vertStage(akr::StageType::Vertex);
	std::string source;
	akfs::open(akfs::SystemFolder::appData, "shaders/main.vert", akfs::OpenFlags::In).readLine(source, false, {});
	vertStage.attach(source);
	vertStage.compile();
	pipeline.attach(vertStage);

	akr::PipelineStage fragStage(akr::StageType::Fragment);
	akfs::open(akfs::SystemFolder::appData, "shaders/main.frag", akfs::OpenFlags::In).readLine(source, false, {});
	fragStage.attach(source);
	fragStage.compile();
	pipeline.attach(fragStage);

	pipeline.link();
}

static const auto cb = ake::regenerateConfigDispatch().subscribe([](ake::RegenerateConfigEvent& ev){
	auto& config = ev.data()["engine"];
	config["ticksPerSecond"].set<fpSingle>(60.0f);
});
