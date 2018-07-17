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

#ifndef AK_WINDOW_WINDOW_HPP_
#define AK_WINDOW_WINDOW_HPP_

#include <string>

#include <ak/window/Types.hpp>

namespace akw {
	struct Monitor;
	class WindowOptions;
}

namespace akin {
	class Keyboard;
	class Mouse;
}

namespace akw {

	void init();
	void shutdown();

	bool open(const WindowOptions& options);
	bool close();

	void pollEvents();
	bool swapBuffer();

	std::string title();

	WindowCoord position();

	WindowCoord size();
	WindowCoord sizeMin();
	WindowCoord sizeMax();
	WindowCoord aspectConstraint();

	FrameCoord frameSize();
	int refreshRate();

	bool closeRequested();

	bool focused();
	bool minimised();
	bool fullscreen();
	bool visible();

	bool alwaysOnTop();
	bool decorated();
	bool resizable();

	void setTitle(const std::string& title);

	void setPosition(WindowCoord nPos);

	void setSize(WindowCoord size);
	void setSizeLimit(WindowCoord minSize, WindowCoord maxSize);
	void setAspectConstraint(WindowCoord aspectSize);

	void setCloseFlag(bool state);

	void restore();
	void minimise();
	void maximise();

	void setFullscreen(Monitor targetMonitor, WindowCoord pos, WindowCoord size, int frameRate);
	void setVisibility(bool state);

	void setCursorMode(CursorMode mode);
	CursorMode cursorMode();

	akin::Mouse& mouse();
	akin::Keyboard& keyboard();

}

#endif
