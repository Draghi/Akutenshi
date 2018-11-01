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

#include <akasset/Convert.hpp>
#include <akcommon/FPSCounter.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/Timer.hpp>
#include <akengine/data/PValue.hpp>
#include <akengine/data/Serialize.hpp>
#include <akengine/debug/Log.hpp>
#include <akengine/entity/Config.hpp>
#include <akengine/event/Dispatcher.hpp>
#include <akengine/filesystem/Path.hpp>
#include <akengine/scene/Scene.hpp>
#include <akengine/scene/SceneManager.hpp>
#include <akgame/game.hpp>
#include <akinput/keyboard/Keyboard.hpp>
#include <akinput/keyboard/Keys.hpp>
#include <akinput/mouse/Buttons.hpp>
#include <akinput/mouse/Mouse.hpp>
#include <akmath/Scalar.hpp>
#include <akrender/gl/Draw.hpp>
#include <akrender/window/Types.hpp>
#include <akrender/window/Window.hpp>
#include <akrender/window/WindowOptions.hpp>
#include <sstream>
#include <stdexcept>

void akg::startup(const akl::Logger& log) {
	log.info("Starting window system."); {
		akr::win::init();

		if (ake::config().exists("window")) {
			if (!akr::win::open(akd::deserialize<akr::win::WindowOptions>(ake::config()["window"]))) throw std::runtime_error("Failed to open window");
		} else {
			auto defaultWindowOptions = akr::win::WindowOptions().glVSync(akr::win::VSync::FULL);
			if (!akr::win::open(defaultWindowOptions)) throw std::runtime_error("Failed to open window");
		}

		akr::win::setCursorMode(akr::win::CursorMode::Captured);
	}

	log.info("Starting OpenGL system."); {
		akr::gl::init();
	}

	log.info("Converting resources."); {
		aka::convertDirectory(akfs::Path("./srcdata/"));
	}
}

void akg::cleanup(const akl::Logger& log) {
	log.info("Cleaning up window system."); {
		akr::win::shutdown();
	}
}

static void setupGame(ake::Scene& scene);

void akg::runGame() {
	ake::SceneManager sceneManager;
	auto& scene = sceneManager.getScene(sceneManager.newScene("World"));

	setupGame(scene);

	fpSingle updateAccum = 0.f;
	fpSingle updateDelta = 1.f/ake::config()["engine"]["ticksPerSecond"].asOrDef<fpSingle>(60.f);
	//fpSingle renderDelta = 1.f/static_cast<fpSingle>(akw::currentMonitor().prefVideoMode.refreshRate);

	while(!akr::win::closeRequested()) {
		static akc::FPSCounter fps(60), tps(60);
		static akc::Timer loopTimer;

		while(updateAccum >= updateDelta) {
			akc::Timer updateTimer;

			akr::win::pollEvents();
			akr::win::mouse().update();
			akr::win::keyboard().update();

			sceneManager.update(updateDelta);

			tps.update();
			if (updateTimer.mark().secsf() > updateDelta) updateAccum = akm::mod(updateAccum, updateDelta);
			updateAccum -= updateDelta;
		}

		sceneManager.render(fps.update().avgTickDelta());

		std::stringstream sstream;
		sstream << fps.avgTicksPerSecond() << "fps | " << tps.avgTicksPerSecond() << "tps";
		akr::win::setTitle(sstream.str());

		updateAccum += loopTimer.mark().secsf();
		loopTimer.reset();
	}
}

static void setupGame(ake::Scene& scene) {

	scene.renderEvent().subscribe([&](ake::SceneRenderEvent& renderEventData) {
		akr::gl::setClearColour(0.2f, 0.2f, 0.2f);
		akr::gl::clear();
		akr::win::swapBuffer();
	});

	scene.updateEvent().subscribe([&](ake::SceneUpdateEvent& updateEventData){
		if (akr::win::mouse().wasReleased(akin::Button::Left)) akr::win::setCursorMode(akr::win::CursorMode::Captured);
		if (akr::win::keyboard().wasReleased(akin::Key::LALT)) akr::win::setCursorMode(akr::win::CursorMode::Normal);

		akr::win::keyboard().update();
		akr::win::mouse().update();
		akr::win::pollEvents();
	});

}

static const auto cb = ake::regenerateConfigDispatch().subscribe([](ake::RegenerateConfigEvent& ev){
	auto& config = ev.data()["engine"];
	config["ticksPerSecond"].set<fpSingle>(60.0f);
});
