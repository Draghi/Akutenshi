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

#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/thread/DoubleBuffer.hpp>
#include <akinput/keyboard/EventKeyboard.hpp>
#include <akinput/keyboard/Keys.hpp>
#include <akinput/mouse/Buttons.hpp>
#include <akinput/mouse/EventMouse.hpp>
#include <akinput/Types.hpp>
#include <akmath/Vector.hpp>
#include <akrender/window/InternalState.hpp>
#include <akrender/window/Monitor.hpp>
#include <akrender/window/Types.hpp>
#include <akrender/window/Window.hpp>
#include <glm/vec2.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <atomic>

using namespace akr::win;
using namespace akr::win::internal;

#define DEFAULT_BIT_DEPTH_RED   8
#define DEFAULT_BIT_DEPTH_GREEN 8
#define DEFAULT_BIT_DEPTH_BLUE  8

static akin::EventMouse mouseInst;
static akin::EventKeyboard keyboardInst;

static akin::Action convertGLFWActionToAK(int action) {
	if (action == GLFW_PRESS) return akin::Action::Pressed;
	if (action == GLFW_RELEASE) return akin::Action::Released;
	if (action == GLFW_REPEAT) return akin::Action::Repeat;
	return akin::Action::None;
}

void akr::win::init() {
	if (hasInit.exchange(true)) return;
	glfwInit();
}

void akr::win::shutdown() {
	if (!hasInit.exchange(false)) return;
	windowHandle = nullptr;
	glfwTerminate();
}


bool akr::win::open(const WindowOptions& options) {
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

	glfwWindowHint(GLFW_STEREO, options.glStereoBuffer());
	glfwWindowHint(GLFW_DOUBLEBUFFER, options.glDoubleBuffer());
	glfwWindowHint(GLFW_SAMPLES, options.glMSAA());

	glfwWindowHint(GLFW_RED_BITS,     DEFAULT_BIT_DEPTH_RED);
	glfwWindowHint(GLFW_GREEN_BITS,   DEFAULT_BIT_DEPTH_GREEN);
	glfwWindowHint(GLFW_BLUE_BITS,    DEFAULT_BIT_DEPTH_BLUE);
	glfwWindowHint(GLFW_REFRESH_RATE, options.videoMode().refreshRate);
	glfwWindowHint(GLFW_SRGB_CAPABLE, options.glSRGB());

	auto fsMonitor = options.fullscreen() ? static_cast<GLFWmonitor*>(options.targetMonitor().handle) : nullptr;
	windowHandle = glfwCreateWindow(
		static_cast<int>(options.videoMode().resolution.x),
		static_cast<int>(options.videoMode().resolution.y),
		options.title().c_str(),
		fsMonitor,
		nullptr
	);

	if (!windowHandle) return false;

	registerCallbacks();

	glfwMakeContextCurrent(windowHandle);

	glfwSwapInterval(options.glVSync() == VSync::NONE ? 0 : (options.glVSync() == VSync::FULL ? 1 : (options.glVSync() == VSync::HALF ? 2 : -1)));

	if (!options.fullscreen()) {
		auto pos = options.position();
		if (options.centerOnMonitor()) {
			auto monitor = options.targetMonitor().handle ? options.targetMonitor() : getMonitorsAt(pos)[0];

			pos += monitor.position + akm::max(monitor.prefVideoMode.resolution/2.0f - options.videoMode().resolution/2.0f, akm::Vec2(0, 0));
		}
		glfwSetWindowPos(windowHandle, static_cast<int>(pos.x), static_cast<int>(pos.y));
	}

	if (options.minimised()) glfwIconifyWindow(windowHandle);
	if (!options.visible()) glfwHideWindow(windowHandle);

	windowState.title = options.title();
	windowState.position = options.position();
	windowState.windowSize = options.videoMode().resolution;

	int sX, sY;
	glfwGetFramebufferSize(windowHandle, &sX, &sY);
	windowState.frameSize = FrameCoord(sX, sY);


	windowState.isDecorated = options.decorated();
	windowState.isMinimised = options.minimised();
	windowState.isVisible = options.visible();
	windowState.isResizable = options.resizable();
	windowState.isAlwaysOnTop = options.alwaysOnTop();
	windowState.refreshRate = options.videoMode().refreshRate;

	glfwSetMouseButtonCallback(windowHandle, [](GLFWwindow* /*window*/, int buttonCode, int action, int /*mods*/){
		mouseInst.onButtonEvent(akin::ButtonEventData{static_cast<akin::Button>(buttonCode), convertGLFWActionToAK(action), &mouseInst});
	});

	glfwSetCursorPosCallback(windowHandle, [](GLFWwindow* /*window*/, double x, double y) {
		auto size = akr::win::size();
		mouseInst.onMoveEvent(akin::MoveEventData{
			{x, size.y - y},
			akm::Vec2{x, size.y - y} - mouseInst.position(),
			&mouseInst
		});
	});

	glfwSetScrollCallback(windowHandle, [](GLFWwindow* /*window*/, double x, double y) {
		mouseInst.onScrollEvent(akin::ScrollEventData{
			std::max(0, static_cast<int32>( y)),
			std::max(0, static_cast<int32>(-y)),
			std::max(0, static_cast<int32>( x)),
			std::max(0, static_cast<int32>(-x)),
			&mouseInst
		});
	});


	glfwSetKeyCallback(windowHandle, [](GLFWwindow* /*window*/, int keycode, int /*scancode*/, int action, int /*mods*/) {
		keyboardInst.onKeyEvent(akin::KeyEventData{static_cast<akin::Key>(keycode), convertGLFWActionToAK(action), &keyboardInst});
	});


	akr::win::pollEvents();

	return true;
}

bool akr::win::close() {
	if (!windowHandle) return false;
	glfwDestroyWindow(windowHandle);
	windowHandle = nullptr;
	return true;
}

void akr::win::pollEvents() {
	processActionBuffer();
	glfwPollEvents();
	processEventBuffer();
}

bool akr::win::swapBuffer() {
	if (!windowHandle) return false;
	glfwSwapBuffers(windowHandle);
	return true;
}

std::string akr::win::title() { return windowState.title; }

WindowCoord akr::win::position() { return windowState.position; }

WindowCoord akr::win::size() { return windowState.windowSize; }
WindowCoord akr::win::sizeMin() { return windowState.windowMinSize; }
WindowCoord akr::win::sizeMax() { return windowState.windowMaxSize; }
WindowCoord akr::win::aspectConstraint() { return windowState.windowAspect; }

FrameCoord akr::win::frameSize() { return windowState.frameSize; }
int akr::win::refreshRate() { return windowState.refreshRate; }

bool akr::win::closeRequested() { return windowState.isCloseRequested; }

bool akr::win::focused() { return windowState.isFocused; }
bool akr::win::minimised() { return windowState.isMinimised; }
bool akr::win::fullscreen() { return windowState.isFullscreen; }
bool akr::win::visible() { return windowState.isVisible; }

bool akr::win::alwaysOnTop() { return windowState.isAlwaysOnTop; }
bool akr::win::decorated() { return windowState.isDecorated; }
bool akr::win::resizable() { return windowState.isResizable; }

CursorMode akr::win::cursorMode() { return windowState.cursorMode; }


void akr::win::setTitle(const std::string& title) {
	actionBuffer.push_back(Action::Title(title));
}

void akr::win::setPosition(WindowCoord nPos) {
	actionBuffer.push_back(Action::Position(nPos));
}

void akr::win::setSize(WindowCoord size) {
	actionBuffer.push_back(Action::WinSize(size));
}

void akr::win::setSizeLimit(WindowCoord minSize, WindowCoord maxSize) {
	actionBuffer.push_back(Action::WinSizeLimit(minSize, maxSize));
}

void akr::win::setAspectConstraint(WindowCoord aspectSize) {
	actionBuffer.push_back(Action::AspectConstraint(aspectSize));
}

void akr::win::setCloseFlag(bool state) {
	actionBuffer.push_back(Action::Close(state));
}

void akr::win::restore() {
	actionBuffer.push_back(Action::Restore(true));
}

void akr::win::minimise() {
	actionBuffer.push_back(Action::Minimise(true));
}

void akr::win::maximise() {
	actionBuffer.push_back(Action::Maximise(true));
}

void akr::win::setFullscreen(Monitor targetMonitor, WindowCoord pos, WindowCoord size, int frameRate) {
	actionBuffer.push_back(Action::Fullscreen(targetMonitor, pos, size, frameRate));
}

void akr::win::setVisibility(bool state) {
	actionBuffer.push_back(Action::Visibility(state));
}

void akr::win::setCursorMode(CursorMode mode) {
	actionBuffer.push_back(Action::CursorMode(mode));
}


akin::Mouse& akr::win::mouse() {
	return mouseInst;
}

akin::Keyboard& akr::win::keyboard() {
	return keyboardInst;
}
