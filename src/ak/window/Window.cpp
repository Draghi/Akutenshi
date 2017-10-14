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

#include <ak/window/Monitor.hpp>
#include <ak/window/Window.hpp>
#include <ak/window/InternalState.hpp>
#include <ak/window/WindowOptions.hpp>
#include <ak/math/Scalar.hpp>
#include <GLFW/glfw3.h>
#include <atomic>
#include <iterator>
#include <vector>

using namespace ak::window;
using namespace ak::window::internal;

#define DEFAULT_BIT_DEPTH_RED   8
#define DEFAULT_BIT_DEPTH_GREEN 8
#define DEFAULT_BIT_DEPTH_BLUE  8

void ak::window::init() {
	if (hasInit.exchange(true)) return;
	glfwInit();
}

void ak::window::shutdown() {
	if (!hasInit.exchange(false)) return;
	windowHandle = nullptr;
	glfwTerminate();
}


bool ak::window::open(const WindowOptions& options) {
	if (!hasInit) return false;
	if (windowHandle) return false;

	glfwDefaultWindowHints();

	glfwWindowHint(GLFW_RESIZABLE,    options.resizable());
	glfwWindowHint(GLFW_VISIBLE,      options.visible());
	glfwWindowHint(GLFW_DECORATED,    options.decorated());
	glfwWindowHint(GLFW_FOCUSED,      GLFW_TRUE);
	glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
	glfwWindowHint(GLFW_FLOATING,     options.alwaysOnTop());
	glfwWindowHint(GLFW_MAXIMIZED,    options.maximised());

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, options.glMajor());
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, options.glMinor());

	glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, options.glCore());
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, options.glForwardCompat());
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, options.glDebugContext());

	glfwWindowHint(GLFW_STEREO, options.glStereo());
	glfwWindowHint(GLFW_DOUBLEBUFFER, options.glDoubleBuffer());
	glfwWindowHint(GLFW_SAMPLES, options.glMSAA());

	glfwWindowHint(GLFW_RED_BITS,     DEFAULT_BIT_DEPTH_RED);
	glfwWindowHint(GLFW_GREEN_BITS,   DEFAULT_BIT_DEPTH_GREEN);
	glfwWindowHint(GLFW_BLUE_BITS,    DEFAULT_BIT_DEPTH_BLUE);
	glfwWindowHint(GLFW_REFRESH_RATE, options.videoMode().refreshRate);
	glfwWindowHint(GLFW_SRGB_CAPABLE, options.glsRGB());

	auto fsMonitor = options.fullscreen() ? static_cast<GLFWmonitor*>(options.targetMonitor().handle) : nullptr;
	windowHandle = glfwCreateWindow(options.videoMode().resolution.x, options.videoMode().resolution.y, options.title().c_str(), fsMonitor, nullptr);
	if (!windowHandle) return false;

	registerCallbacks();

	glfwMakeContextCurrent(windowHandle);

	glfwSwapInterval(options.glVSync() == VSync::NONE ? 0 : (options.glVSync() == VSync::FULL ? 1 : (options.glVSync() == VSync::HALF ? 2 : -1)));

	if (!options.fullscreen()) {
		auto pos = options.position();
		if (options.centerOnMonitor()) {
			auto monitor = options.targetMonitor().handle ? options.targetMonitor() : getMonitorsAt(pos)[0];

			pos.x += monitor.position.x + ak::math::max(monitor.prefVideoMode.resolution.x/2 - options.videoMode().resolution.x/2, 0);
			pos.y += monitor.position.y + ak::math::max(monitor.prefVideoMode.resolution.y/2 - options.videoMode().resolution.y/2, 0);
		}
		glfwSetWindowPos(windowHandle, pos.x, pos.y);
	}

	if (options.minimised()) glfwIconifyWindow(windowHandle);
	if (!options.visible()) glfwHideWindow(windowHandle);

	windowState.title = options.title();
	windowState.position = options.position();
	windowState.windowSize = options.videoMode().resolution;
	glfwGetFramebufferSize(windowHandle, &windowState.frameSize.x, &windowState.frameSize.y);
	windowState.isDecorated = options.decorated();
	windowState.isMinimised = options.minimised();
	windowState.isVisible = options.visible();
	windowState.isResizable = options.resizable();
	windowState.isAlwaysOnTop = options.alwaysOnTop();
	windowState.refreshRate = options.videoMode().refreshRate;

	ak::window::pollEvents();

	return true;
}

bool ak::window::close() {
	if (!windowHandle) return false;
	glfwDestroyWindow(windowHandle);
	windowHandle = nullptr;
	return true;
}

void ak::window::pollEvents() {
	processActionBuffer();
	glfwPollEvents();
	processEventBuffer();
}

bool ak::window::swapBuffer() {
	if (!windowHandle) return false;
	glfwSwapBuffers(windowHandle);
	return true;
}

std::string ak::window::title() { return windowState.title; }

WindowCoord ak::window::pos() { return windowState.position; }

WindowCoord ak::window::winSize() { return windowState.windowSize; }
WindowCoord ak::window::winSizeMin() { return windowState.windowMinSize; }
WindowCoord ak::window::winSizeMax() { return windowState.windowMaxSize; }
WindowCoord ak::window::winAspectConstraint() { return windowState.windowAspect; }

FrameCoord ak::window::frameSize() { return windowState.frameSize; }
int ak::window::refreshRate() { return windowState.refreshRate; }

bool ak::window::isCloseRequested() { return windowState.isCloseRequested; }

bool ak::window::isFocused() { return windowState.isFocused; }
bool ak::window::isMinimised() { return windowState.isMinimised; }
bool ak::window::isFullscreen() { return windowState.isFullscreen; }
bool ak::window::isVisisble() { return windowState.isVisible; }

bool ak::window::isAlwaysOnTop() { return windowState.isAlwaysOnTop; }
bool ak::window::isDecorated() { return windowState.isDecorated; }
bool ak::window::isResizable() { return windowState.isResizable; }



void ak::window::setTitle(const std::string& title) {
	actionBuffer.push_back(Action::Title(title));
}

void ak::window::setPos(WindowCoord nPos) {
	actionBuffer.push_back(Action::Position(nPos));
}

void ak::window::setWinSize(WindowCoord size) {
	actionBuffer.push_back(Action::WinSize(size));
}

void ak::window::setWinSizeLimits(WindowCoord minSize, WindowCoord maxSize) {
	actionBuffer.push_back(Action::WinSizeLimit(minSize, maxSize));
}

void ak::window::setWinAspectConstraint(WindowCoord aspectSize) {
	actionBuffer.push_back(Action::AspectConstraint(aspectSize));
}

void ak::window::setCloseRequest(bool state) {
	actionBuffer.push_back(Action::Close(state));
}

void ak::window::restore() {
	actionBuffer.push_back(Action::Restore(true));
}

void ak::window::minimise() {
	actionBuffer.push_back(Action::Minimise(true));
}

void ak::window::maximise() {
	actionBuffer.push_back(Action::Maximise(true));
}

void ak::window::setFullscreen(Monitor targetMonitor, WindowCoord pos, WindowCoord size, int frameRate) {
	actionBuffer.push_back(Action::Fullscreen(targetMonitor, pos, size, frameRate));
}

void ak::window::setVisibility(bool state) {
	actionBuffer.push_back(Action::Visibility(state));
}