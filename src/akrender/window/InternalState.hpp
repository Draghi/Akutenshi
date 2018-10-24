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

#ifndef AK_WINDOW_INTERNALSTATE_HPP_
#define AK_WINDOW_INTERNALSTATE_HPP_

#include <akengine/thread/DoubleBuffer.hpp>
#include <akrender/window/Monitor.hpp>
#include <akrender/window/Types.hpp>
#include <atomic>
#include <string>

struct GLFWmonitor;

struct GLFWwindow;

namespace akw {
	namespace internal {

		extern std::atomic<bool> hasInit;
		extern GLFWwindow* windowHandle;

		struct WindowState {
			std::string title;

			WindowCoord position= {0,0};

			WindowCoord windowSize = {0,0};

			WindowCoord windowMinSize = {0,0};
			WindowCoord windowMaxSize = {0,0};
			WindowCoord windowAspect  = {0,0};

			FrameCoord frameSize = {0,0};
			int refreshRate = 0;

			bool isCloseRequested = false;

			bool isFocused = false;
			bool isMinimised = false;
			bool isFullscreen = false;
			bool isVisible = false;

			bool isAlwaysOnTop = false;
			bool isDecorated = false;
			bool isResizable = false;

			Monitor fullscreenMonitor = Monitor::NullMonitor();
			Monitor currentMonitor = Monitor::NullMonitor();

			CursorMode cursorMode = CursorMode::Normal;
		};

		extern WindowState windowState;

		enum class ActionType {
			Title,

			Position,
			WinSize,

			WinSizeLimit,
			WinAspectConstraint,

			FrameSize,

			CloseRequest,

			Restore,
			Minimise,
			Maximise,
			Focus,

			Fullscreen,
			Visibility,

			CursorMode
		};

		struct Action {
			ActionType type;

			std::string title;

			WindowCoord winCoord1;
			WindowCoord winCoord2;
			FrameCoord frameCoord;
			bool state;

			Monitor fullscreenMonitor;
			int targetFramerate;

			CursorMode cursorMode;

			static Action Title(const std::string& title) { return {ActionType::Title, title, {0,0}, {0,0}, {0,0}, false, Monitor::NullMonitor(), 0, CursorMode::Normal}; }

			static Action Position(WindowCoord pos) { return {ActionType::Position, "", pos, {0,0}, {0,0}, false, Monitor::NullMonitor(), 0, CursorMode::Normal}; }
			static Action WinSize(WindowCoord size) { return {ActionType::WinSize, "", size, {0,0}, {0,0}, false, Monitor::NullMonitor(), 0, CursorMode::Normal}; }

			static Action WinSizeLimit(WindowCoord minSize, WindowCoord maxSize) { return {ActionType::WinSizeLimit, "", minSize, maxSize, {0,0}, false, Monitor::NullMonitor(), 0, CursorMode::Normal}; }
			static Action AspectConstraint(WindowCoord aspectConstraint) { return {ActionType::WinAspectConstraint, "", aspectConstraint, {0,0}, {0,0}, false, Monitor::NullMonitor(), 0, CursorMode::Normal}; }

			static Action FrameSize(FrameCoord frameSize) { return {ActionType::FrameSize, "", {0,0}, {0,0}, frameSize, false, Monitor::NullMonitor(), 0, CursorMode::Normal}; }

			static Action Close(bool state) { return {ActionType::CloseRequest, "", {0,0}, {0,0}, {0,0}, state, Monitor::NullMonitor(), 0, CursorMode::Normal}; }

			static Action Restore(bool state) { return {ActionType::Restore, "", {0,0}, {0,0}, {0,0}, state, Monitor::NullMonitor(), 0, CursorMode::Normal}; }
			static Action Minimise(bool state) { return {ActionType::Minimise, "", {0,0}, {0,0}, {0,0}, state, Monitor::NullMonitor(), 0, CursorMode::Normal}; }
			static Action Maximise(bool state) { return {ActionType::Maximise, "", {0,0}, {0,0}, {0,0}, state, Monitor::NullMonitor(), 0, CursorMode::Normal}; }
			static Action Focus(bool state) { return {ActionType::Focus, "", {0,0}, {0,0}, {0,0}, state, Monitor::NullMonitor(), 0, CursorMode::Normal}; }

			static Action Fullscreen(Monitor monitor, WindowCoord pos, WindowCoord size, int frameRate) { return {ActionType::Fullscreen, "", pos, size, {0,0}, false, monitor, frameRate, CursorMode::Normal}; }
			static Action Visibility(bool state) { return {ActionType::Visibility, "", {0,0}, {0,0}, {0,0}, state, Monitor::NullMonitor(), 0, CursorMode::Normal}; }

			static Action CursorMode(CursorMode mode) { return {ActionType::CursorMode, "", {0,0}, {0,0}, {0,0}, false, Monitor::NullMonitor(), 0, mode}; }
		};

		extern akt::DoubleBuffer<Action> eventBuffer;
		extern akt::DoubleBuffer<Action> actionBuffer;

		void registerCallbacks();
		void processEventBuffer();
		void processActionBuffer();
	}
}

#endif
