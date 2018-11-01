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

#include <akrender/window/InternalState.hpp>
#include <akrender/window/Monitor.hpp>
#include <akrender/window/Window.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <atomic>
#include <iterator>
#include <stdexcept>

using namespace akw;
using namespace akw::internal;

static Monitor createMonitorFromGLFWMonitor(GLFWmonitor* monitor);

std::vector<Monitor> akw::getAllWindowMonitors() {
	if (!windowHandle) return {Monitor::NullMonitor()};
	return getMonitorsAt(position(), size());
}

Monitor akw::currentMonitor() {
	if (!windowHandle) return Monitor::NullMonitor();
	return getMonitorsAt(position(), {0,0}).front();
}

Monitor akw::primaryMonitor() {
	auto* monitor = glfwGetPrimaryMonitor();
	if (monitor) return createMonitorFromGLFWMonitor(monitor);
	return Monitor::NullMonitor();
}

std::vector<Monitor> akw::monitors() {
	if (!hasInit) throw std::logic_error("Window system not initialized");

	int monitorCount;
	auto* glfwMonitors = glfwGetMonitors(&monitorCount);
	if (glfwMonitors == nullptr) throw std::runtime_error("Failed to retrieve monitors");

	std::vector<Monitor> result;
	for(int i = 0; i < monitorCount; i++) {
		result.push_back(createMonitorFromGLFWMonitor(glfwMonitors[i]));
	}

	return result;
}

void akw::setGamma(const Monitor* monitor, fpSingle gamma) {
	if (!monitor) return;
	glfwSetGamma(static_cast<GLFWmonitor*>(monitor->handle), gamma);
}

static VideoMode createVideoModeFromGLFWVidMode(const GLFWvidmode* videoMode) {
	return videoMode == nullptr
		? VideoMode::DefaultVideoMode()
		: VideoMode{{videoMode->width, videoMode->height}, videoMode->refreshRate};
}

static Monitor createMonitorFromGLFWMonitor(GLFWmonitor* monitor) {
	Monitor monitorRecord;
	monitorRecord.handle = monitor;
	monitorRecord.name = glfwGetMonitorName(monitor);

	int pX, pY;
	glfwGetMonitorPos(monitor, &pX, &pY);
	monitorRecord.position = WindowCoord(pX, pY);

	int sX, sY;
	glfwGetMonitorPhysicalSize(monitor, &sX, &sY);
	monitorRecord.realSizeMM = RealCoord(sX, sY);

	monitorRecord.prefVideoMode = createVideoModeFromGLFWVidMode(glfwGetVideoMode(monitor));

	int videoModeCount;
	auto* videoModes = glfwGetVideoModes(monitor, &videoModeCount);
	for(int j = 0; j < videoModeCount; j++) {
		auto videoMode = createVideoModeFromGLFWVidMode(&videoModes[j]);
		if (std::find(monitorRecord.videoModes.begin(), monitorRecord.videoModes.end(), videoMode) == monitorRecord.videoModes.end()) {
			monitorRecord.videoModes.push_back(videoMode);
		}
	}

	return monitorRecord;
}

std::vector<Monitor> akw::getMonitorsAt(WindowCoord pos, WindowCoord size) {
	if (!hasInit) return {Monitor::NullMonitor()};

	auto wLeft   = pos.x;
	auto wRight  = pos.x + size.x;
	auto wTop    = pos.y;
	auto wBottom = pos.y + size.y;

	auto allMonitors = monitors();
	std::vector<Monitor> result;
	for(auto iter = allMonitors.begin(); iter != allMonitors.end(); iter++) {
		auto mLeft   = iter->position.x;
		auto mRight  = iter->position.x + iter->prefVideoMode.resolution.x;
		auto mTop    = iter->position.y;
		auto mBottom = iter->position.y + iter->prefVideoMode.resolution.y;

		if ((wRight < mLeft) || (wLeft >= mRight)) continue;
		if ((wBottom < mTop) || (wTop >= mBottom)) continue;

		result.push_back(*iter);
	}

	if (result.size() <= 0) result.push_back(Monitor::NullMonitor());

	return result;
}


