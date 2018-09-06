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
#include <ak/container/RingBuffer.hpp>
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
#include <ak/filesystem/Filesystem.hpp>
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
#include <ak/sound/Decode.hpp>
#include <ak/sound/FilterFIRFreq.hpp>
#include <ak/sound/FilterVolume.hpp>
#include <ak/sound/hrtf/HRTFFilterLookup.hpp>
#include <ak/sound/MixerBasic.hpp>
#include <ak/sound/SamplerBuffer.hpp>
#include <ak/sound/Types.hpp>
#include <ak/sound/Util.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/thread/CurrentThread.hpp>
#include <ak/util/FPSCounter.hpp>
#include <ak/util/Memory.hpp>
#include <ak/util/String.hpp>
#include <ak/util/Time.hpp>
#include <ak/util/Timer.hpp>
#include <ak/window/Types.hpp>
#include <ak/window/Window.hpp>
#include <ak/window/WindowOptions.hpp>
#include <akgame/CameraControllerBehaviour.hpp>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>


int akGameMain();

static void printLogHeader(const akl::Logger& logger);
static ak::ScopeGuard startup();

static void startGame();

static akc::RingBuffer<fpSingle> audioChannelLeft(1), audioChannelRight(1);

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

	aks::backend::init(nullptr, {aks::backend::Format::FPSingle, aks::backend::ChannelMap::Stereo, 48000},
		[&](void* audioFrames, akSize frameCount, aks::backend::StreamFormat /*streamFormat*/){
			//static akSize cFrame = 0;
			//akSize writtenFrames = 0;

			//std::array<std::vector<fpSingle>, 2> buffer{std::vector<fpSingle>(frameCount, 0.f), std::vector<fpSingle>(frameCount, 0.f)};
			//writtenFrames = std::max(writtenFrames, leftMixer.sample(buffer[0].data(), cFrame, frameCount));
			//writtenFrames = std::max(writtenFrames, rightMixer.sample(buffer[1].data(), cFrame, frameCount));
			//cFrame += writtenFrames;

			//for(akSize i = 0; i < writtenFrames; i++) {
				//static_cast<fpSingle*>(audioFrames)[i*2 + 0] = akm::clamp(buffer[0][i], -1, 1);
				//static_cast<fpSingle*>(audioFrames)[i*2 + 1] = akm::clamp(buffer[1][i], -1, 1);
			//}

			/*static akSize frameReadOffset = 0;
			akSize writtenFrames = 0;
			auto lock = audioQueueLock.lock();
			while((!audioQueue.empty()) && (writtenFrames < frameCount)) {
				auto& audioFrame = audioQueue.front();
				auto& leftBuffer = audioFrame.at(aks::Channel::Left);
				auto& rightBuffer = audioFrame.at(aks::Channel::Right);

				if (leftBuffer.size() != rightBuffer.size()) throw std::logic_error("Submitted audio frames must be the same size.");

				static_cast<fpSingle*>(audioFrames)[writtenFrames*2 + 0] = -0.1f;
				static_cast<fpSingle*>(audioFrames)[writtenFrames*2 + 1] = -0.1f;

				for(akSize oIndex = writtenFrames + 1; (frameReadOffset < rightBuffer.size()) && (writtenFrames < frameCount); oIndex++, frameReadOffset++, writtenFrames++) {
					static_cast<fpSingle*>(audioFrames)[oIndex*2 + 0] = akm::clamp( leftBuffer[frameReadOffset], -1, 1);
					static_cast<fpSingle*>(audioFrames)[oIndex*2 + 1] = akm::clamp(rightBuffer[frameReadOffset], -1, 1);
				}

				if ((frameReadOffset < rightBuffer.size()) && (writtenFrames >= frameCount)) break;
				frameReadOffset = 0;
				audioQueue.pop_front();
			}*/

			std::array<std::vector<fpSingle>, 2> buffer{std::vector<fpSingle>(frameCount, 0.f), std::vector<fpSingle>(frameCount, 0.f)};
			akSize leftRead  =  audioChannelLeft.read(buffer[0].data(), frameCount);
			akSize rightRead = audioChannelRight.read(buffer[1].data(), frameCount);
			if (leftRead != rightRead) throw std::logic_error("Inconsistent audio channel read.");

			akl::Logger("").info("Req: ", frameCount, " Recv: ", leftRead);

			for(akSize i = 0; i < leftRead; i++) {
				if (i == 0) {
					static_cast<fpSingle*>(audioFrames)[i*2 + 0] = -0.1;
					static_cast<fpSingle*>(audioFrames)[i*2 + 1] = -0.1;
				} else {
					static_cast<fpSingle*>(audioFrames)[i*2 + 0] = akm::clamp(buffer[0][i], -1, 1);
					static_cast<fpSingle*>(audioFrames)[i*2 + 1] = akm::clamp(buffer[1][i], -1, 1);
				}
			}

			return leftRead;
		}
	);

	audioChannelLeft  = akc::RingBuffer<fpSingle>(aks::backend::getDeviceInfo()->streamFormat.sampleRate * 4);
	audioChannelRight = akc::RingBuffer<fpSingle>(aks::backend::getDeviceInfo()->streamFormat.sampleRate * 4);

	aks::backend::startDevice();

	akas::convertDirectory(akfs::Path("./srcdata/"));

	startGame();

	log.info("Exiting main function");

	return 0;
}

static aks::MixerBasic leftMixer, rightMixer;

static aks::hrtf::HRTFFilterLookup leftHRTFLookup;
static aks::hrtf::HRTFFilterLookup rightHRTFLookup;

static aks::SamplerBuffer sineWave;
static aks::FilterFIRFreq leftHRTF, rightHRTF;
static aks::FilterVolume leftVolume, rightVolume;

static void startGame() {
	// constexpr ak::log::Logger log(AK_STRING_VIEW("Main"));

/*	static auto trainSound = aks::decode(akfs::CFile("data/test.flac").readAll(), true);
	static auto volTrainSoundLeft = aks::FilterVolume(trainSound.at(aks::Channel::Left), 0.25f);
	static auto volTrainSoundRight = aks::FilterVolume(trainSound.at(aks::Channel::Right), 1.f);
	leftMixer.addSource(volTrainSoundLeft);
	rightMixer.addSource(volTrainSoundRight);*/

	sineWave = aks::generateSineWave(100, 1.f);
	leftVolume  = aks::FilterVolume(leftHRTF, 0.125f/8.f);
	rightVolume = aks::FilterVolume(rightHRTF, 0.125f/8.f);

	static const std::string hrtfDir = "IRC_1002_C";
	akfs::iterateDirectory(akfs::Path("data/")/hrtfDir, [&](const akfs::Path& path, bool isDir) {
		if (isDir) return true;

		std::string filename = akfs::Path(path).clearExtension().filename();
		if (filename.find(hrtfDir) != 0) {
			akl::Logger("").info("Skipping: ", path.filename());
			return true;
		}

		std::string nameData = filename.substr(hrtfDir.size() + 1) + '_';

		auto radius    = std::strtol(nameData.data() + nameData.find_first_of('R') + 1, nullptr, 10);
		auto azimuth   = std::strtol(nameData.data() + nameData.find_first_of('T') + 1, nullptr, 10);
		auto elevation = std::strtol(nameData.data() + nameData.find_first_of('P') + 1, nullptr, 10);

		auto data = aks::decode(akfs::CFile(path).readAll(), true);
		leftHRTFLookup.addEntryByAngle(
			aku::make_unique<aks::Sampler, aks::SamplerBuffer>(data.at(aks::Channel::Left)),
			azimuth,
			elevation
		);

		rightHRTFLookup.addEntryByAngle(
			aku::make_unique<aks::Sampler, aks::SamplerBuffer>(data.at(aks::Channel::Right)),
			azimuth,
			elevation
		);

		return true;
	}, false);

	leftMixer.addSource(leftVolume);
	rightMixer.addSource(rightVolume);


	akas::AssetRegistry assetRegistry(akfs::Path("data/"));
	ake::SceneManager sceneManager;
	auto& scene = sceneManager.getScene(sceneManager.newScene("World"));
	auto& ecs = scene.entities();

	if (!ecs.registerComponentManager(std::make_unique<ake::TransformManager>()))  throw std::runtime_error("Failed to register TransformComponent.");
	if (!ecs.registerComponentManager(std::make_unique<ake::CameraManager>()))     throw std::runtime_error("Failed to register CameraManager.");
	if (!ecs.registerComponentManager(std::make_unique<ake::BehavioursManager>())) throw std::runtime_error("Failed to register BehavioursManager.");

	auto cameraEID = ecs.newEntity("Camera");
	cameraEID.createComponent<ake::Transform>(akm::Vec3{0,0,0});
	cameraEID.createComponent<ake::Camera>();
	cameraEID.createComponent<ake::Behaviours>();
	cameraEID.component<ake::Camera>().setPerspectiveH(akm::degToRad(90), akw::size(), {0.1f, 65525.0f});
	cameraEID.component<ake::Behaviours>().addBehaviour(std::make_unique<akgame::CameraControllerBehaviour>());

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

		akm::Vec3 pos = {akm::sin(time/5.f), 0, akm::cos(time/5.f)};

		auto leftFilter = leftHRTFLookup.findClosestFiltersByPos(pos, 1).front();
		auto rightFilter = rightHRTFLookup.findClosestFiltersByPos(pos, 1).front();
		 leftHRTF = aks::FilterFIRFreq(sineWave, *(leftFilter.second));
		rightHRTF = aks::FilterFIRFreq(sineWave, *(rightFilter.second));


		static akSize cFrame = 0;
		akSize expectedFrames = time * aks::backend::getDeviceInfo()->streamFormat.sampleRate;
		if (cFrame < expectedFrames) {
			akSize framesToBuffer = (expectedFrames - cFrame);
			akSize writtenFrames = std::numeric_limits<akSize>::max();

			std::array<std::vector<fpSingle>, 2> buffer{std::vector<fpSingle>(framesToBuffer, 0.f), std::vector<fpSingle>(framesToBuffer, 0.f)};
			writtenFrames = std::min(writtenFrames,  leftMixer.sample(buffer[0].data(), cFrame, buffer[0].size()));
			writtenFrames = std::min(writtenFrames, rightMixer.sample(buffer[1].data(), cFrame, buffer[1].size()));

			 audioChannelLeft.write(buffer[0].data(), writtenFrames);
			audioChannelRight.write(buffer[1].data(), writtenFrames);

			cFrame += writtenFrames;
		} else akl::Logger("Up to date").info("");


		/*{
			auto lock = audioQueueLock.lock();
			audioQueue.push_back(std::unordered_map<aks::Channel, std::vector<fpSingle>>{
				{aks::Channel::Left,  std::move(buffer[0])},
				{aks::Channel::Right, std::move(buffer[1])},
			});
		}*/

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
