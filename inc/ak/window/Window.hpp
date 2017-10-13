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

#ifndef AK_WINDOW_WINDOW_HPP_
#define AK_WINDOW_WINDOW_HPP_

#include <ak/data/PValue.hpp>
#include <string>

namespace ak {
	namespace window {
		struct FrameCoord;
		struct WindowCoord;
	}
}

namespace ak {
	namespace window {
		struct Monitor;
		class WindowOptions;
	}
}

namespace ak {
	namespace window {

		void init();
		void shutdown();

		bool open(const WindowOptions& options);
		bool close();

		void pollEvents();
		bool swapBuffer();

		std::string title();

		WindowCoord pos();

		WindowCoord winSize();
		WindowCoord winSizeMin();
		WindowCoord winSizeMax();
		WindowCoord winAspectConstraint();

		FrameCoord frameSize();
		int refreshRate();

		bool isCloseRequested();

		bool isFocused();
		bool isMinimised();
		bool isFullscreen();
		bool isVisisble();

		bool isAlwaysOnTop();
		bool isDecorated();
		bool isResizable();



		void setTitle(const std::string& title);

		void setPos(WindowCoord nPos);

		void setWinSize(WindowCoord size);
		void setWinSizeLimits(WindowCoord minSize, WindowCoord maxSize);
		void setWinAspectConstraint(WindowCoord aspectSize);

		void setCloseRequest(bool state);

		void restore();
		void minimise();
		void maximise();

		void setFullscreen(Monitor targetMonitor, WindowCoord pos, WindowCoord size, int frameRate);
		void setVisibility(bool state);

	}
}

#endif
