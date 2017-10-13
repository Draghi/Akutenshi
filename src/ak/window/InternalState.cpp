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

#include <ak/window/InternalState.hpp>
#include <GLFW/glfw3.h>
#include <stddef.h>

namespace ak {
	namespace window {
		namespace internal {
			std::atomic<bool> hasInit = false;
			GLFWwindow* windowHandle = nullptr;

			WindowState windowState;

			ak::container::DoubleBuffer<Action> eventBuffer;
			ak::container::DoubleBuffer<Action> actionBuffer;
		}
	}
}

using namespace ak::window;
using namespace ak::window::internal;

static void glfwPosHandler(GLFWwindow* /*handle*/, int x, int y) {
	eventBuffer.push_back(Action::Position({x, y}));
}

static void glfwSizeHandler(GLFWwindow* /*handle*/, int w, int h) {
	eventBuffer.push_back(Action::WinSize({w, h}));
}

static void glfwCloseHandler(GLFWwindow* /*handle*/) {
	eventBuffer.push_back(Action::Close(true));
}

static void glfwFocusHandler(GLFWwindow* /*handle*/, int state) {
	eventBuffer.push_back(Action::Focus(state == GLFW_TRUE));
}

static void glfwMinimiseHandler(GLFWwindow* /*handle*/, int state) {
	eventBuffer.push_back(Action::Minimise(state == GLFW_TRUE));
}

static void glfwFrameSizeHandler(GLFWwindow* /*handle*/, int w, int h) {
	eventBuffer.push_back(Action::FrameSize({w, h}));
}

void ak::window::internal::registerCallbacks() {
	glfwSetWindowPosCallback(windowHandle, glfwPosHandler);
	glfwSetWindowSizeCallback(windowHandle, glfwSizeHandler);
	glfwSetWindowCloseCallback(windowHandle, glfwCloseHandler);
	glfwSetWindowFocusCallback(windowHandle, glfwFocusHandler);
	glfwSetWindowIconifyCallback(windowHandle, glfwMinimiseHandler);
	glfwSetFramebufferSizeCallback(windowHandle, glfwFrameSizeHandler);
}

void ak::window::internal::processEventBuffer() {
	eventBuffer.swap();

	WindowState newState = windowState;
	eventBuffer.iterate([&](size_t /*index*/, const Action& action){
		switch(action.type) {
			case Title:
				newState.title = action.title;
				break;

			case Position:
				newState.position = action.winCoord1;
				break;

			case WinSize:
				newState.windowSize = action.winCoord1;
				break;

			case WinSizeLimit:
				newState.windowMinSize = action.winCoord1;
				newState.windowMaxSize = action.winCoord2;
				break;

			case WinAspectConstraint:
				newState.windowAspect = action.winCoord1;
				break;

			case FrameSize:
				newState.frameSize = action.frameCoord;
				break;

			case CloseRequest:
				newState.isCloseRequested = action.state;
				break;

			case Restore:
				newState.isMinimised = false;
				break;

			case Minimise:
				newState.isMinimised = true;
				break;

			case Maximise:
				newState.isMinimised = false;
				break;

			case Focus:
				newState.isFocused = action.state;
				break;

			case Fullscreen:
				newState.fullscreenMonitor = action.fullscreenMonitor;
				newState.position = action.winCoord1;
				newState.windowSize = action.winCoord2;
				newState.refreshRate = action.targetFramerate;
				break;

			case Visibility:
				newState.isVisible = action.state;
				break;
		}
	});
	windowState = newState;

}

void ak::window::internal::processActionBuffer() {
	actionBuffer.swap();
	actionBuffer.iterate([&](size_t /*index*/, const Action& action){
		switch(action.type) {
			case Title:
				glfwSetWindowTitle(windowHandle, action.title.c_str());
				break;

			case Position:
				glfwSetWindowPos(windowHandle, action.winCoord1.x, action.winCoord1.y);
				break;

			case WinSize:
				glfwSetWindowSize(windowHandle, action.winCoord1.x, action.winCoord1.y);
				break;

			case WinSizeLimit:
				glfwSetWindowSizeLimits(windowHandle, action.winCoord1.x, action.winCoord1.y, action.winCoord2.x, action.winCoord2.y);
				eventBuffer.push_back(action);
				glfwPollEvents();
				break;

			case WinAspectConstraint:
				glfwSetWindowAspectRatio(windowHandle, action.winCoord1.y, action.winCoord1.x);
				eventBuffer.push_back(action);
				glfwPollEvents();
				break;

			case FrameSize:
				break;

			case CloseRequest:
				glfwSetWindowShouldClose(windowHandle, action.state);
				break;

			case Restore:
				if (action.state) glfwRestoreWindow(windowHandle);
				break;

			case Minimise:
				if (action.state) glfwIconifyWindow(windowHandle);
				break;

			case Maximise:
				if (action.state) glfwMaximizeWindow(windowHandle);
				eventBuffer.push_back(action);
				glfwPollEvents();
				break;

			case Focus:
				if (action.state) glfwFocusWindow(windowHandle);
				break;

			case Fullscreen:
				glfwSetWindowMonitor(windowHandle, static_cast<GLFWmonitor*>(action.fullscreenMonitor.handle), action.winCoord1.x, action.winCoord1.y, action.winCoord2.x, action.winCoord2.y, action.targetFramerate);
				glfwPollEvents();
				eventBuffer.push_back(action);
				break;

			case Visibility:
				if (action.state) glfwShowWindow(windowHandle);
				else glfwHideWindow(windowHandle);
				break;
		}
	});
}
