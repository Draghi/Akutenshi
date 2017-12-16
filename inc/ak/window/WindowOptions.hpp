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

#ifndef AK_WINDOW_WINDOWOPTIONS_HPP_
#define AK_WINDOW_WINDOWOPTIONS_HPP_

#include <ak/data/PValue.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/window/Monitor.hpp>
#include <ak/window/Types.hpp>
#include <string>

namespace akw {

	enum class VSync {
		NONE,
		FULL,
		HALF,
		ADAPTIVE,
	};

	class WindowOptions final {
		private:
			WindowCoord m_offsetPosition = {0, 0};
			std::string m_title = "Akutenshi";

			Monitor m_targetMonitor = primaryMonitor();
			VideoMode m_videoMode = VideoMode::DefaultVideoMode();

			bool m_fullscreen = false, m_centerOnMonitor = true;

			bool m_isResizable = false, m_isVisible = true,  m_isDecorated = true;
			bool m_alwaysOnTop = false, m_minimised = false, m_maximised   = false;

			int m_glMajor = 3, m_glMinor = 3;
			bool m_glCore = true, m_glForward = true, m_glDebug = false;
			bool m_glStereo = false, m_glSRGB = true, m_glDoubleBuffer = true;
			uint8 m_glMSSA = 4;
			VSync m_glVSync = VSync::NONE;

		public:
			WindowOptions& position(WindowCoord val) { m_offsetPosition = val; return *this; }
			WindowOptions& title(const std::string& val) { m_title = val; return *this; }

			WindowOptions& fullscreen(bool val) { m_fullscreen = val; return *this; }
			WindowOptions& centerOnMonitor(bool val) { m_centerOnMonitor = val; return *this; }

			WindowOptions& targetMonitor(const Monitor& val) { m_targetMonitor = val; return *this; }
			WindowOptions& videoMode(const VideoMode& val) { m_videoMode = val; return *this; }

			WindowOptions& resizable(bool val) { m_isResizable = val; return *this; }
			WindowOptions& visible(bool val) { m_isVisible = val; return *this; }
			WindowOptions& decorated(bool val) { m_isDecorated = val; return *this; }
			WindowOptions& alwaysOnTop(bool val) { m_alwaysOnTop = val; return *this; }
			WindowOptions& maximised(bool val) { m_maximised = val; return *this; }
			WindowOptions& minimised(bool val) { m_minimised = val; return *this; }


			WindowOptions& glVersion(int major, int minor) { m_glMajor = major; m_glMinor = minor; return *this; }
			WindowOptions& glCore(bool val) { m_glCore = val; return *this; }
			WindowOptions& glForwardCompat(bool val) { m_glForward = val; return *this; }
			WindowOptions& glDebugContext(bool val) { m_glDebug = val; return *this; }
			WindowOptions& glStereoBuffer(bool val) { m_glStereo = val; return *this; }
			WindowOptions& glSRGB(bool val) { m_glSRGB = val; return *this; }
			WindowOptions& glDoubleBuffer(bool val) { m_glDoubleBuffer = val; return *this; }
			WindowOptions& glMSAA(uint8 val) { m_glMSSA = val; return *this; }
			WindowOptions& glVSync(VSync val) { m_glVSync = val; return *this; }

			WindowCoord position() const {return m_offsetPosition; }
			const std::string& title() const { return m_title; }

			bool fullscreen() const { return m_fullscreen; }
			bool centerOnMonitor() const { return m_centerOnMonitor; }

			const Monitor& targetMonitor() const { return m_targetMonitor; }
			const VideoMode& videoMode() const { return m_videoMode; }

			bool resizable() const { return m_isResizable; }
			bool visible() const { return m_isVisible; }
			bool decorated() const { return m_isDecorated; }
			bool alwaysOnTop() const { return m_alwaysOnTop; }
			bool maximised() const { return m_maximised; }
			bool minimised() const { return m_minimised; }

			int glMajor() const { return m_glMajor; }
			int glMinor() const { return m_glMinor; }
			bool glCore() const { return m_glCore; }
			bool glForwardCompat() const { return m_glForward; }
			bool glDebugContext() const { return m_glDebug; }
			bool glStereoBuffer() const { return m_glStereo; }
			bool glSRGB() const { return m_glSRGB; }
			bool glDoubleBuffer() const { return m_glDoubleBuffer; }
			VSync glVSync() const { return m_glVSync; }
			int glMSAA() const { return m_glMSSA; }
	};
}

namespace akd {
	template<> akw::WindowOptions deserialize<akw::WindowOptions>(const akd::PValue& root);
	template<> void serialize<akw::WindowOptions>(akd::PValue& root, const akw::WindowOptions& val);
}

#endif
