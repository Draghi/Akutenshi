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
#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/String.hpp>
#include <akcommon/Timer.hpp>
#include <akengine/Config.hpp>
#include <akengine/data/Hash.hpp>
#include <akengine/data/PValue.hpp>
#include <akengine/data/Serialize.hpp>
#include <akengine/debug/Log.hpp>
#include <akengine/ecs/Component.hpp>
#include <akengine/ecs/Entity.hpp>
#include <akengine/ecs/Registry.hpp>
#include <akengine/ecs/Types.hpp>
#include <akengine/event/Dispatcher.hpp>
#include <akengine/filesystem/Path.hpp>
#include <akgame/game.hpp>
#include <akrender/gl/Draw.hpp>
#include <akrender/window/Types.hpp>
#include <akrender/window/Window.hpp>
#include <akrender/window/WindowOptions.hpp>
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

static void setupGame(/*ake::Scene& scene*/);

class TestComponent1 : public akecs::Component {
	public:
		static constexpr std::string_view COMPONENT_NAME = AK_STRING_VIEW("TestComponent1");
		static constexpr akecs::ComponentTypeUID COMPONENT_UID = akd::hash32FNV1A<char>(COMPONENT_NAME.data(), COMPONENT_NAME.size());

		TestComponent1(akecs::BaseRegistry&, akecs::EntityRef) {}
		virtual ~TestComponent1() = default;

		TestComponent1(const TestComponent1&) {}
		TestComponent1& operator=(const TestComponent1&) { return *this; }
};

class TestComponent2 : public akecs::Component {
	public:
		static constexpr std::string_view COMPONENT_NAME = AK_STRING_VIEW("TestComponent2");
		static constexpr akecs::ComponentTypeUID COMPONENT_UID = akd::hash32FNV1A<char>(COMPONENT_NAME.data(), COMPONENT_NAME.size());

		TestComponent2(akecs::BaseRegistry&, akecs::EntityRef) {}


};

class TestComponent3 : public akecs::Component {
	public:
		static constexpr std::string_view COMPONENT_NAME = AK_STRING_VIEW("TestComponent3");
		static constexpr akecs::ComponentTypeUID COMPONENT_UID = akd::hash32FNV1A<char>(COMPONENT_NAME.data(), COMPONENT_NAME.size());
};

void akg::runGame() {
	akl::Logger("Info").info(sizeof(akecs::Registry<TestComponent1>));
	akl::Logger("Info").info(sizeof(akecs::Registry<TestComponent1, TestComponent2>));
	akl::Logger("Info").info(sizeof(akecs::Registry<TestComponent1, TestComponent2, TestComponent3>));


	akc::Timer timer;

	akSize totalMS = 0;
	for(akSize j = 0; j < 100; j++) {
		akecs::Registry<TestComponent1, TestComponent2> a;
		a.reserveEntities(10000000);
		for(akSize i = 0; i < 10000000; i++) a.create();
		akSize timeMS = timer.markAndReset().msecs();
		totalMS += timeMS;
		akl::Logger("ecs").info("Time taken to create 10,000,000: ", timeMS, "ms");
	}

	akl::Logger("ecs").info("Average time taken to create 10,000,000: ", totalMS/100, "ms");

	// for(akSize i = 0; i < 10000000; i++) a.destroy(entities[i]);
	// akl::Logger("ecs").info("Time taken to destroy 10,000,000: ", timer.markAndReset().msecs(), "ms");

	/*for(akSize i = 0; i < 1000000; i++) {
		auto entity = a.create();
		if (!a.   attach<TestComponent1>(entity)) akl::Logger("Info").info(   "attach 1 fail");
		if (!a.component<TestComponent1>(entity)) akl::Logger("Info").info("component 1 fail");
		if (!a.   detach<TestComponent1>(entity)) akl::Logger("Info").info(   "detach 1 fail");
		a.destroy(entity);
	}*/


/*	ake::SceneManager sceneManager;
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
	}*/
}

static void setupGame(/*ake::Scene& scene*/) {

/*	scene.renderEvent().subscribe([&](ake::SceneRenderEvent& renderEventData) {
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
	});*/

}

static const auto cb = ake::regenerateConfigDispatch().subscribe([](ake::RegenerateConfigEvent& ev){
	auto& config = ev.data()["engine"];
	config["ticksPerSecond"].set<fpSingle>(60.0f);
});
