#include <ak/data/JsonParser.hpp>
#include <ak/data/PValue.hpp>
#include <ak/engine/Config.hpp>
#include <ak/engine/Startup.hpp>
#include <ak/log/Logger.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/time/FPSCounter.hpp>
#include <ak/window/Monitor.hpp>
#include <ak/window/Types.hpp>
#include <ak/window/Window.hpp>
#include <ak/window/WindowOptions.hpp>
#include <iterator>
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

int main() {
	ak::log::Logger log("PValueTest");

	auto engineShutdownGuard = ak::engine::startup(ak::data::PValue());
	ak::thread::current().sleep(100000);

	if (ak::engine::config().exists("window")) {
		ak::window::open(ak::window::WindowOptions::deserialize(ak::engine::config()["window"]));
	} else {
		auto defaultWindowOptions = ak::window::WindowOptions();
		defaultWindowOptions.serialize(ak::engine::config()["window"]);
		ak::window::open(defaultWindowOptions);
	}


	ak::time::FPSCounter fpsCounter;
	while(!ak::window::isCloseRequested()) {

		std::stringstream sstream;
		{
			auto monitors = ak::window::getAllWindowMonitors();
			sstream << (ak::window::isFocused() ? "*" : "?") << " <" << fpsCounter.fps() << "> "<< ak::window::pos().x << "," << ak::window::pos().y << " {" << ak::window::winSize().x << "x" << ak::window::winSize().y << "}@" << ak::window::refreshRate() << " | ";
			for(auto iter = monitors.begin(); iter != monitors.end(); iter++) sstream << iter->name << " {" << iter->prefVideoMode.resolution.x << "x" << iter->prefVideoMode.resolution.y << "}, ";
		}
		ak::window::setTitle(sstream.str());

		ak::window::swapBuffer();
		ak::window::pollEvents();

		fpsCounter.update();
	}
}
