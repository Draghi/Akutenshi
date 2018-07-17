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

#include <ak/window/InternalState.hpp>

#include <glm/detail/type_vec2.hpp>
#include <GLFW/glfw3.h>

#include <ak/PrimitiveTypes.hpp>

namespace akw {
	namespace internal {
		std::atomic<bool> hasInit = false;
		GLFWwindow* windowHandle = nullptr;

		WindowState windowState;

		akt::DoubleBuffer<Action> eventBuffer;
		akt::DoubleBuffer<Action> actionBuffer;
	}
}

using namespace akw;
using namespace akw::internal;

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

void akw::internal::registerCallbacks() {
	glfwSetWindowPosCallback(windowHandle, glfwPosHandler);
	glfwSetWindowSizeCallback(windowHandle, glfwSizeHandler);
	glfwSetWindowCloseCallback(windowHandle, glfwCloseHandler);
	glfwSetWindowFocusCallback(windowHandle, glfwFocusHandler);
	glfwSetWindowIconifyCallback(windowHandle, glfwMinimiseHandler);
	glfwSetFramebufferSizeCallback(windowHandle, glfwFrameSizeHandler);
}

void akw::internal::processEventBuffer() {
	eventBuffer.swap();

	WindowState newState = windowState;
	eventBuffer.iterate([&](akSize /*index*/, const Action& action){
		switch(action.type) {
			case ActionType::Title:
				newState.title = action.title;
				break;

			case ActionType::Position:
				newState.position = action.winCoord1;
				break;

			case ActionType::WinSize:
				newState.windowSize = action.winCoord1;
				break;

			case ActionType::WinSizeLimit:
				newState.windowMinSize = action.winCoord1;
				newState.windowMaxSize = action.winCoord2;
				break;

			case ActionType::WinAspectConstraint:
				newState.windowAspect = action.winCoord1;
				break;

			case ActionType::FrameSize:
				newState.frameSize = action.frameCoord;
				break;

			case ActionType::CloseRequest:
				newState.isCloseRequested = action.state;
				break;

			case ActionType::Restore:
				newState.isMinimised = false;
				break;

			case ActionType::Minimise:
				newState.isMinimised = true;
				break;

			case ActionType::Maximise:
				newState.isMinimised = false;
				break;

			case ActionType::Focus:
				newState.isFocused = action.state;
				break;

			case ActionType::Fullscreen:
				newState.fullscreenMonitor = action.fullscreenMonitor;
				newState.position = action.winCoord1;
				newState.windowSize = action.winCoord2;
				newState.refreshRate = action.targetFramerate;
				break;

			case ActionType::Visibility:
				newState.isVisible = action.state;
				break;

			case ActionType::CursorMode:
				newState.cursorMode = action.cursorMode;
				break;
		}
	});
	windowState = newState;

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-conversion"
void akw::internal::processActionBuffer() {
	actionBuffer.swap();
	actionBuffer.iterate([&](akSize /*index*/, const Action& action){
		switch(action.type) {
			case ActionType::Title:
				glfwSetWindowTitle(windowHandle, action.title.c_str());
				break;

			case ActionType::Position:
				glfwSetWindowPos(windowHandle, action.winCoord1.x, action.winCoord1.y);
				break;

			case ActionType::WinSize:
				glfwSetWindowSize(windowHandle, action.winCoord1.x, action.winCoord1.y);
				break;

			case ActionType::WinSizeLimit:
				glfwSetWindowSizeLimits(windowHandle, action.winCoord1.x, action.winCoord1.y, action.winCoord2.x, action.winCoord2.y);
				eventBuffer.push_back(action);
				glfwPollEvents();
				break;

			case ActionType::WinAspectConstraint:
				glfwSetWindowAspectRatio(windowHandle, action.winCoord1.y, action.winCoord1.x);
				eventBuffer.push_back(action);
				glfwPollEvents();
				break;

			case ActionType::FrameSize:
				break;

			case ActionType::CloseRequest:
				glfwSetWindowShouldClose(windowHandle, action.state);
				break;

			case ActionType::Restore:
				if (action.state) glfwRestoreWindow(windowHandle);
				break;

			case ActionType::Minimise:
				if (action.state) glfwIconifyWindow(windowHandle);
				break;

			case ActionType::Maximise:
				if (action.state) glfwMaximizeWindow(windowHandle);
				eventBuffer.push_back(action);
				glfwPollEvents();
				break;

			case ActionType::Focus:
				if (action.state) glfwFocusWindow(windowHandle);
				break;

			case ActionType::Fullscreen:
				glfwSetWindowMonitor(windowHandle, static_cast<GLFWmonitor*>(action.fullscreenMonitor.handle), action.winCoord1.x, action.winCoord1.y, action.winCoord2.x, action.winCoord2.y, action.targetFramerate);
				glfwPollEvents();
				eventBuffer.push_back(action);
				break;

			case ActionType::Visibility:
				if (action.state) glfwShowWindow(windowHandle);
				else glfwHideWindow(windowHandle);
				break;

			case ActionType::CursorMode:
				switch(action.cursorMode) {
					case CursorMode::Normal:   glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);   break;
					case CursorMode::Captured: glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED); break;
					case CursorMode::Hidden:   glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);   break;
				}
				eventBuffer.push_back(action);
				break;
		}
	});
}
#pragma clang diagnostic pop
