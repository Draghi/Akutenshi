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

#include <ak/assets/AssetRegistry.hpp>
#include <ak/assets/Convert.hpp>
#include <ak/assets/Image.hpp>
#include <ak/data/PValue.hpp>
#include <ak/data/Serialize.hpp>
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
#include <ak/input/Mouse.hpp>
#include <ak/Log.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/gl/Draw.hpp>
#include <ak/render/gl/RenderTarget.hpp>
#include <ak/render/gl/Textures.hpp>
#include <ak/render/gl/Types.hpp>
#include <ak/render/SceneRendererDefault.hpp>
#include <ak/sound/backend/Backend.hpp>
#include <ak/sound/backend/Types.hpp>
#include <ak/sound/Buffer.hpp>
#include <ak/sound/Decode.hpp>
#include <ak/sound/FIRFilter.hpp>
#include <ak/sound/Types.hpp>
#include <ak/sound/Util.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/thread/CurrentThread.hpp>
#include <ak/util/FPSCounter.hpp>
#include <ak/util/FreqDomainFilter.hpp>
#include <ak/util/String.hpp>
#include <ak/util/Time.hpp>
#include <ak/util/Timer.hpp>
#include <ak/window/Types.hpp>
#include <ak/window/Window.hpp>
#include <ak/window/WindowOptions.hpp>
#include <akgame/CameraControllerBehaviour.hpp>
#include <algorithm>
#include <array>
#include <functional>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>


int akGameMain();

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

	constexpr akSize AUDIO_SAMPLE_RATE = 48000; // The sample rate for audio playback
	constexpr float WAVE_FREQUENCY = 100.f;    // The frequency of the wave to produce
	aks::Buffer sound, sound2;
	std::unordered_map<aks::Channel, aks::Buffer> trainSounds;

	aks::FreqDomainFilter domainFilter;

	aks::backend::init(nullptr, {aks::backend::Format::FPSingle, aks::backend::ChannelMap::Stereo, AUDIO_SAMPLE_RATE},
		[&](void* audioFrames, akSize frameCount, aks::backend::StreamFormat /*streamFormat*/){
			/*if (!sound) return 0u;
			static akSize cFrame = 0;
			auto writtenFrames = sound->sample(static_cast<fpSingle*>(audioFrames), cFrame, frameCount);
			cFrame += writtenFrames;
			return writtenFrames;*/
			if (trainSounds.empty()) return 0u;

			static akSize cFrame = 0;
			akSize writtenFrames = 0;

			std::array<std::vector<fpSingle>, 2> buffer; buffer[0].resize(frameCount, 0.f); buffer[1].resize(frameCount, 0.f);
			//writtenFrames = std::max(writtenFrames, aks::FIRFilter(trainSounds[aks::Channel::Left],  aks::generateLowPassFilterAvg(AUDIO_SAMPLE_RATE)).sample(buffer[0].data(), cFrame, frameCount));
			//writtenFrames = std::max(writtenFrames, aks::FIRFilter(trainSounds[aks::Channel::Right], aks::generateLowPassFilterAvg(800)).sample(buffer[1].data(), cFrame, frameCount));
			//writtenFrames = std::max(writtenFrames, aks::KernalFilter(sound,  aks::generateLowPassFilterAvg(WAVE_FREQUENCY*10)).sample(buffer[0].data(), cFrame, frameCount));
			//writtenFrames = std::max(writtenFrames, aks::KernalFilter(sound2, aks::generateLowPassFilterAvg(WAVE_FREQUENCY*10)).sample(buffer[1].data(), cFrame, frameCount));
			writtenFrames = std::max(writtenFrames, domainFilter.sample(buffer[0].data(), cFrame, frameCount));
			//writtenFrames = std::max(writtenFrames, sound2.sample(buffer[0].data(), cFrame, frameCount));
			if (akw::keyboard().isDown(akin::Key::P)) writtenFrames = std::max(writtenFrames, aks::FIRFilter(trainSounds[aks::Channel::Right], aks::generateLowPassFilterAvg(800)).sample(buffer[1].data(), cFrame, frameCount));
			else if (!akw::keyboard().isDown(akin::Key::O)) writtenFrames = std::max(writtenFrames,trainSounds[aks::Channel::Right].sample(buffer[1].data(), cFrame, frameCount));

			cFrame += writtenFrames;

			for(akSize i = 0; i < writtenFrames; i++) {
				static_cast<fpSingle*>(audioFrames)[i*2 + 0] = akm::clamp(buffer[0][i], -1, 1);
				static_cast<fpSingle*>(audioFrames)[i*2 + 1] = akm::clamp(buffer[1][i], -1, 1);
			}

			return writtenFrames;
		}
	);

	sound  = aks::generateSineWave(WAVE_FREQUENCY,    1.0f);
	sound2 = aks::generateWindowedSinc(800, aks::backend::getDeviceInfo()->streamFormat.sampleRate); // aks::generateSineWave(WAVE_FREQUENCY*10, 1.0f);
	std::vector<uint8> fileData; akfs::CFile("srcdata/test.flac").readAll(fileData);
	trainSounds = aks::decode(fileData);

	domainFilter = aks::FreqDomainFilter(trainSounds[aks::Channel::Left], aks::generateWindowedSinc(800, aks::backend::getDeviceInfo()->streamFormat.sampleRate));

	aks::backend::startDevice();

	akas::convertDirectory(akfs::Path("./srcdata/"));

	startGame();

	log.info("Exiting main function");

	return 0;
}

static void startGame() {
	// constexpr ak::log::Logger log(AK_STRING_VIEW("Main"));

	akas::AssetRegistry assetRegistry(akfs::Path("data/"));
	ake::SceneManager sceneManager;
	auto& scene = sceneManager.getScene(sceneManager.newScene("World"));
	auto& ecs = scene.entities();

	//if (!ecs.registerComponentManager(std::make_unique<ake::SceneGraphManager>())) throw std::runtime_error("Failed to register SceneGraphComponent.");
	if (!ecs.registerComponentManager(std::make_unique<ake::TransformManager>())) throw std::runtime_error("Failed to register TransformComponent.");
	if (!ecs.registerComponentManager(std::make_unique<ake::CameraManager>())) throw std::runtime_error("Failed to register CameraManager.");
	if (!ecs.registerComponentManager(std::make_unique<ake::BehavioursManager>())) throw std::runtime_error("Failed to register BehavioursManager.");

	auto cameraEID = ecs.newEntity("Camera");
	cameraEID.createComponent<ake::Transform>(akm::Vec3{0,0,0});
	cameraEID.createComponent<ake::Camera>();
	cameraEID.createComponent<ake::Behaviours>();
	cameraEID.component<ake::Camera>().setPerspectiveH(akm::degToRad(90), akw::size(), {0.1f, 65525.0f});
	cameraEID.component<ake::Behaviours>().addBehaviour(std::make_unique<akgame::CameraControllerBehaviour>());

	//auto skyboxTexture = resourceManager.loadUniqueTexture("data/textures/cubemap_snowy_street.aktex");

	auto skyboxTexture = std::make_shared<akr::gl::Texture>(akr::gl::TexTarget::TexCubemap);
	akr::gl::setActiveTexUnit(0);
	akr::gl::bindTexture(0, *skyboxTexture);
	{
		auto pX = akas::loadImageAndTransform("data/skybox/winter/pX.jpg", akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte_sRGB, akas::ImageRotation::None, 0, 0, 0, 0, false, false);
		auto pY = akas::loadImageAndTransform("data/skybox/winter/pY.jpg", akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte_sRGB, akas::ImageRotation::None, 0, 0, 0, 0, false, false);
		auto pZ = akas::loadImageAndTransform("data/skybox/winter/pZ.jpg", akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte_sRGB, akas::ImageRotation::None, 0, 0, 0, 0, false, false);
		auto nX = akas::loadImageAndTransform("data/skybox/winter/nX.jpg", akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte_sRGB, akas::ImageRotation::None, 0, 0, 0, 0, false, false);
		auto nY = akas::loadImageAndTransform("data/skybox/winter/nY.jpg", akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte_sRGB, akas::ImageRotation::None, 0, 0, 0, 0, false, false);
		auto nZ = akas::loadImageAndTransform("data/skybox/winter/nZ.jpg", akr::gl::TexFormat::RGBA, akr::gl::TexStorage::Byte_sRGB, akas::ImageRotation::None, 0, 0, 0, 0, false, false);

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

	scene.renderEvent().subscribe([&](ake::SceneRenderEvent& renderEventData){
		fpDouble delta = renderEventData.data();
		static fpSingle time = 0;
		time += delta;

		// Prepare
		akr::gl::setClearColour(0.2f, 0.2f, 0.2f);
		akr::gl::clear();

		worldRenderer.renderScene(scene, delta);

		akr::gl::blitRenderTargetToDisplay(worldRenderer.renderTarget(), {0,0}, worldRenderer.renderTargetSize(), {0,0}, worldRenderer.renderTargetSize(), akr::gl::BlitMask::Colour, akr::gl::FilterType::Nearest);
		akr::gl::bindDisplayRenderTarget();

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
