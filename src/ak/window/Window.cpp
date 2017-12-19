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

#include <ak/input/Buttons.hpp>
#include <ak/input/EventInput.hpp>
#include <ak/input/Keys.hpp>
#include <ak/input/Types.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/thread/DoubleBuffer.hpp>
#include <ak/window/InternalState.hpp>
#include <ak/window/Monitor.hpp>
#include <ak/window/Window.hpp>
#include <ak/window/WindowOptions.hpp>
#include <glm/detail/func_common.hpp>
#include <glm/detail/type_vec2.hpp>
#include <GLFW/glfw3.h>
#include <atomic>

namespace ak {
	namespace input {
		class Keyboard;
	}
}

using namespace akw;
using namespace akw::internal;

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

void akw::init() {
	if (hasInit.exchange(true)) return;
	glfwInit();
}

void akw::shutdown() {
	if (!hasInit.exchange(false)) return;
	windowHandle = nullptr;
	glfwTerminate();
}


bool akw::open(const WindowOptions& options) {
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

			pos += monitor.position + akm::cMax(monitor.prefVideoMode.resolution/2.0f - options.videoMode().resolution/2.0f, akm::Vec2(0, 0));
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
		auto size = akw::size();
		mouseInst.onMoveEvent(akin::MoveEventData{
			{x, size.y - y},
			akm::Vec2{x, size.y - y} - mouseInst.position(),
			&mouseInst
		});
	});

	glfwSetScrollCallback(windowHandle, [](GLFWwindow* /*window*/, double x, double y) {
		mouseInst.onScrollEvent(akin::ScrollEventData{
			static_cast<int32>(akm::max(0.0,  y)),
			static_cast<int32>(akm::max(0.0, -y)),
			static_cast<int32>(akm::max(0.0,  x)),
			static_cast<int32>(akm::max(0.0, -x)),
			&mouseInst
		});
	});


	glfwSetKeyCallback(windowHandle, [](GLFWwindow* /*window*/, int keycode, int /*scancode*/, int action, int /*mods*/) {
		keyboardInst.onKeyEvent(akin::KeyEventData{static_cast<akin::Key>(keycode), convertGLFWActionToAK(action), &keyboardInst});
	});


	akw::pollEvents();

	return true;
}

bool akw::close() {
	if (!windowHandle) return false;
	glfwDestroyWindow(windowHandle);
	windowHandle = nullptr;
	return true;
}

void akw::pollEvents() {
	processActionBuffer();
	glfwPollEvents();
	processEventBuffer();
}

bool akw::swapBuffer() {
	if (!windowHandle) return false;
	glfwSwapBuffers(windowHandle);
	return true;
}

std::string akw::title() { return windowState.title; }

WindowCoord akw::position() { return windowState.position; }

WindowCoord akw::size() { return windowState.windowSize; }
WindowCoord akw::sizeMin() { return windowState.windowMinSize; }
WindowCoord akw::sizeMax() { return windowState.windowMaxSize; }
WindowCoord akw::aspectConstraint() { return windowState.windowAspect; }

FrameCoord akw::frameSize() { return windowState.frameSize; }
int akw::refreshRate() { return windowState.refreshRate; }

bool akw::closeRequested() { return windowState.isCloseRequested; }

bool akw::focused() { return windowState.isFocused; }
bool akw::minimised() { return windowState.isMinimised; }
bool akw::fullscreen() { return windowState.isFullscreen; }
bool akw::visible() { return windowState.isVisible; }

bool akw::alwaysOnTop() { return windowState.isAlwaysOnTop; }
bool akw::decorated() { return windowState.isDecorated; }
bool akw::resizable() { return windowState.isResizable; }

CursorMode akw::cursorMode() { return windowState.cursorMode; }


void akw::setTitle(const std::string& title) {
	actionBuffer.push_back(Action::Title(title));
}

void akw::setPosition(WindowCoord nPos) {
	actionBuffer.push_back(Action::Position(nPos));
}

void akw::setSize(WindowCoord size) {
	actionBuffer.push_back(Action::WinSize(size));
}

void akw::setSizeLimit(WindowCoord minSize, WindowCoord maxSize) {
	actionBuffer.push_back(Action::WinSizeLimit(minSize, maxSize));
}

void akw::setAspectConstraint(WindowCoord aspectSize) {
	actionBuffer.push_back(Action::AspectConstraint(aspectSize));
}

void akw::setCloseFlag(bool state) {
	actionBuffer.push_back(Action::Close(state));
}

void akw::restore() {
	actionBuffer.push_back(Action::Restore(true));
}

void akw::minimise() {
	actionBuffer.push_back(Action::Minimise(true));
}

void akw::maximise() {
	actionBuffer.push_back(Action::Maximise(true));
}

void akw::setFullscreen(Monitor targetMonitor, WindowCoord pos, WindowCoord size, int frameRate) {
	actionBuffer.push_back(Action::Fullscreen(targetMonitor, pos, size, frameRate));
}

void akw::setVisibility(bool state) {
	actionBuffer.push_back(Action::Visibility(state));
}

void akw::setCursorMode(CursorMode mode) {
	actionBuffer.push_back(Action::CursorMode(mode));
}


akin::Mouse& akw::mouse() {
	return mouseInst;
}

akin::Keyboard& akw::keyboard() {
	return keyboardInst;
}
