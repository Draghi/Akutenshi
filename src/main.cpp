#include <ak/data/PValue.hpp>
#include <ak/engine/Config.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/input/Keyboard.hpp>
#include <ak/input/Keys.hpp>
#include <ak/input/Mouse.hpp>
#include <ak/Log.hpp>
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
#include <ak/window/Types.hpp>
#include <ak/window/Window.hpp>
#include <ak/window/WindowOptions.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <iomanip>
#include <sstream>
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

static void printLogHeader(const ak::log::Logger& logger);
static ak::ScopeGuard startup();

static void createShaderProgram(akr::Pipeline& pipeline);
static void createTexture();


int main() {
	constexpr ak::log::Logger log(AK_STRING_VIEW("Main"));
	auto shutdownScope = startup();

	log.info("Engine started.");
	printLogHeader(log);

	if (ak::engine::config().exists("window")) {
		ak::window::open(ak::data::deserialize<ak::window::WindowOptions>(ak::engine::config()["window"]));
	} else {
		auto defaultWindowOptions = ak::window::WindowOptions().glVSync(ak::window::VSync::FULL);
		ak::window::open(defaultWindowOptions);
	}

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
		auto vData = akr::genCubeMesh();
		akr::setData(ak::render::BufferTarget::VARRYING, vData.data(), static_cast<uint32>(vData.size()));
		akr::mapVertexBufferF(0, 3, ak::render::DataType::FPSingle, false,                  0, 8*sizeof(fpSingle));
		akr::mapVertexBufferF(1, 3, ak::render::DataType::FPSingle, false, 3*sizeof(fpSingle), 8*sizeof(fpSingle));
		akr::mapVertexBufferF(2, 2, ak::render::DataType::FPSingle, false, 6*sizeof(fpSingle), 8*sizeof(fpSingle));

		// Tex
		akr::Texture tex(akr::TexTarget::Tex2D);
		akr::setActiveTextureUnit(0);
		akr::bind(0, tex);
		createTexture();
	// Setup Finish

	while(!ak::window::closeRequested()) {
		// Input Start
			ak::window::pollEvents();
			ak::window::mouse().update();
			ak::window::keyboard().update();

			static fpSingle x = 0, y = 0, z = 0;
			if (akw::keyboard().isDown(akin::Key::W)) y += 1/60.0;
			if (akw::keyboard().isDown(akin::Key::A)) x -= 1/60.0;
			if (akw::keyboard().isDown(akin::Key::S)) y -= 1/60.0;
			if (akw::keyboard().isDown(akin::Key::D)) x += 1/60.0;
			if (akw::keyboard().isDown(akin::Key::R)) z += 1/60.0;
			if (akw::keyboard().isDown(akin::Key::F)) z -= 1/60.0;
		// Input End

		// Render Start
			// Prepare
			akr::clear();
			akr::setActivePipeline(pipeline);

			// Setup Cube
			akr::bind(vMapping);
			akr::setUniform(0, akm::perspective<fpSingle>(1.0472f, akw::size().x/static_cast<fpSingle>(akw::size().y), 0.1f, 100.0f));
			akr::setUniform(2, 0);

			// Draw Controlled Cube
			akr::setUniform(1, akm::translate(akm::Vec3(x, y, z)));
			akr::draw(akr::DrawType::Triangles, 36);

			// Draw Floor
			for(int i = 0; i < 32; i++) for(int j = 0; j < 32; j++) {
				akr::setUniform(1, akm::translate(akm::Vec3(i-16, -2, j)));
				akr::draw(akr::DrawType::Triangles, 36);
			}

			// Finish
			ak::window::swapBuffer();
		// Render End
	}

	log.info("Exiting main function");
}

static void printLogHeader(const ak::log::Logger& logger) {
	auto utc = ak::util::utcTimestamp();

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
	if (!ak::engine::loadConfig()) {
		log.warn("Failed to load config. Attempting to regenerate.");
		ak::engine::regenerateConfig();
		if (!ak::engine::saveConfig()) log.warn("Failed to save new config.");
	}
}

static void startupLog() {
	ak::log::startProcessing();
	//ak::log::enableFileOutput();
}

static void startupWindow() {
	ak::window::init();
}

static ak::ScopeGuard startup() {
	constexpr ak::log::Logger startLog(AK_STRING_VIEW("Start"));

	ak::thread::current().setName("Main");

	startLog.info("Loading engine config.");
	startupConfig();

	startLog.info("Starting log system.");
	startupLog();

	startLog.info("Starting window system.");
	startupWindow();

	return [](){
		constexpr ak::log::Logger stopLog(AK_STRING_VIEW("Stop"));

		stopLog.info("Saving config.");
		if (!ak::engine::saveConfig()) stopLog.warn("Failed to save config.");

		stopLog.info("Flushing log system.");
		ak::log::stopProcessing();
		ak::log::processMessageQueue();
		ak::log::processMessageQueue();
	};
}

static void createShaderProgram(akr::Pipeline& pipeline) {
	akr::PipelineStage vertStage(ak::render::StageType::Vertex);
	std::string source;
	akfs::open(akfs::SystemFolder::appData, "shaders/main.vert", akfs::OpenFlags::In).readLine(source, false, {});
	vertStage.attach(source);
	vertStage.compile();
	pipeline.attach(vertStage);

	akr::PipelineStage fragStage(ak::render::StageType::Fragment);
	akfs::open(akfs::SystemFolder::appData, "shaders/main.frag", akfs::OpenFlags::In).readLine(source, false, {});
	fragStage.attach(source);
	fragStage.compile();
	pipeline.attach(fragStage);

	pipeline.link();
}

static void createTexture() {
	const fpSingle texData[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
	};

	akr::setTextureData2D(akr::TexFormat::RGB, 2, 2, texData);
	akr::setTextureFilters(akr::TexTarget::Tex2D, akr::FilterType::Nearest, akr::FilterType::Nearest);
	akr::setTextureClamping(akr::TexTarget::Tex2D, akr::ClampDir::S, akr::ClampType::Edge);
	akr::setTextureClamping(akr::TexTarget::Tex2D, akr::ClampDir::T, akr::ClampType::Edge);
}
