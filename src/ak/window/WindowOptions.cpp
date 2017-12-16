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

#include <ak/engine/Config.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/event/Subscription.hpp>
#include <ak/window/WindowOptions.hpp>
#include <algorithm>
#include <iterator>
#include <vector>

using namespace akw;

static Monitor findMonitorByName(const std::vector<Monitor>& allMonitors, const std::string& name) {
	auto iter = std::find_if(allMonitors.begin(), allMonitors.end(), [&](const Monitor& monitor){ return monitor.name == name; });
	if (iter == allMonitors.end()) return Monitor::NullMonitor();
	if (std::find_if(iter + 1, allMonitors.end(), [&](const Monitor& monitor){ return monitor.name == name; }) != allMonitors.end()) return Monitor::NullMonitor();
	return *iter;
}

static Monitor findTargetMonitor(const std::string& name, WindowCoord pos) {
	auto monitorByName = findMonitorByName(monitors(), name);
	if (monitorByName != Monitor::NullMonitor()) return monitorByName;

	auto monitorsAt = akw::getMonitorsAt(pos, {0,0});
	if (monitorsAt.size() == 1) {
		if (monitorsAt.front() != Monitor::NullMonitor()) return monitorsAt.front();
		return primaryMonitor();
	}

	monitorByName = findMonitorByName(monitorsAt, name);
	if (monitorByName != Monitor::NullMonitor()) return monitorByName;
	return primaryMonitor();
}

template<> akw::WindowOptions akd::deserialize<akw::WindowOptions>(const akd::PValue& root) {
	WindowCoord pos = {root["position"]["x"].as<int>(), root["position"]["y"].as<int>()};
	WindowCoord winSize = {root["videoMode"]["resolution"]["x"].as<int>(), root["videoMode"]["resolution"]["y"].as<int>()};
	VideoMode videoMode = {winSize, root["videoMode"]["refreshRate"].as<int>()};

	auto targetMonitor = findTargetMonitor(root["targetMonitor"]["name"].as<std::string>(), {root["targetMonitor"]["position"]["x"].as<int>(), root["targetMonitor"]["position"]["y"].as<int>()});

	VSync vsyncType = vsyncType = VSync::NONE;
	auto vsyncStr = root["glVSync"].as<std::string>();
	if (vsyncStr == "None") vsyncType = VSync::NONE;
	else if (vsyncStr == "Full") vsyncType = VSync::FULL;
	else if (vsyncStr == "Half") vsyncType = VSync::HALF;
	else if (vsyncStr == "Adaptive") vsyncType = VSync::ADAPTIVE;

	return WindowOptions()
		.position(pos)
		.title(root["title"].as<std::string>())

		.fullscreen(root["fullscreen"].as<bool>())
		.centerOnMonitor(root["centerOnMonitor"].as<bool>())

		.targetMonitor(targetMonitor)
		.videoMode(videoMode)

		.resizable(root["resizable"].as<bool>())
		.visible(root["visible"].as<bool>())
		.decorated(root["decorated"].as<bool>())
		.alwaysOnTop(root["alwaysOnTop"].as<bool>())
		.maximised(root["maximised"].as<bool>())
		.minimised(root["minimised"].as<bool>())

		.glVSync(vsyncType)

		.glVersion(root["glMajor"].as<int>(), root["glMinor"].as<int>())
		.glCore(root["glCore"].as<bool>())
		.glForwardCompat(root["glForwardCompat"].as<bool>())
		.glDebugContext(root["glDebugContext"].as<bool>())
		.glStereoBuffer(root["glStereoBuffer"].as<bool>())
		.glSRGB(root["glSRGB"].as<bool>())
		.glDoubleBuffer(root["glDoubleBuffer"].as<bool>())
		.glMSAA(root["glMSAA"].as<uint8>());
}

template<> void akd::serialize<akw::WindowOptions>(akd::PValue& root, const akw::WindowOptions& val) {
	root["position"]["x"].set<int>(val.position().x);
	root["position"]["y"].set<int>(val.position().y);

	root["title"].set<std::string>(val.title());

	root["fullscreen"].set<bool>(val.fullscreen());
	root["centerOnMonitor"].set<bool>(val.centerOnMonitor());

	root["targetMonitor"]["name"].set<std::string>(val.targetMonitor().name);
	root["targetMonitor"]["position"]["x"].set<int>(val.targetMonitor().position.x);
	root["targetMonitor"]["position"]["y"].set<int>(val.targetMonitor().position.y);

	root["videoMode"]["refreshRate"].set<int>(val.videoMode().refreshRate);
	root["videoMode"]["resolution"]["x"].set<int>(val.videoMode().resolution.x);
	root["videoMode"]["resolution"]["y"].set<int>(val.videoMode().resolution.y);

	root["resizable"].set<bool>(val.resizable());
	root["visible"].set<bool>(val.visible());
	root["decorated"].set<bool>(val.decorated());
	root["alwaysOnTop"].set<bool>(val.alwaysOnTop());
	root["maximised"].set<bool>(val.maximised());
	root["minimised"].set<bool>(val.minimised());

	switch(val.glVSync()) {
		case VSync::NONE:      root["glVSync"].set<std::string>("None"); break;
		case VSync::FULL:      root["glVSync"].set<std::string>("Full"); break;
		case VSync::HALF:      root["glVSync"].set<std::string>("Half"); break;
		case VSync::ADAPTIVE:  root["glVSync"].set<std::string>("Adaptive"); break;
	}

	root["glMajor"].set<int>(val.glMajor());
	root["glMinor"].set<int>(val.glMinor());
	root["glCore"].set<bool>(val.glCore());
	root["glForwardCompat"].set<bool>(val.glForwardCompat());
	root["glDebugContext"].set<bool>(val.glDebugContext());
	root["glStereoBuffer"].set<bool>(val.glStereoBuffer());
	root["glSRGB"].set<bool>(val.glSRGB());
	root["glDoubleBuffer"].set<bool>(val.glDoubleBuffer());
	root["glMSAA"].set<int>(val.glMSAA());
}

static akev::SubscriberID windowSInitRegenerateConfigHook = ake::regenerateConfigDispatch().subscribe([](ake::RegenerateConfigEvent& event){
	akd::serialize<akw::WindowOptions>(event.data()["window"], akw::WindowOptions());
});
