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

namespace ak {
	namespace window {

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

				static Monitor findMonitorByName(const std::vector<Monitor>& allMonitors, const std::string& name) {
					auto iter = std::find_if(allMonitors.begin(), allMonitors.end(), [&](const Monitor& monitor){ return monitor.name == name; });
					if (iter == allMonitors.end()) return Monitor::NullMonitor();
					if (std::find_if(iter + 1, allMonitors.end(), [&](const Monitor& monitor){ return monitor.name == name; }) != allMonitors.end()) return Monitor::NullMonitor();
					return *iter;
				}

				static Monitor findTargetMonitor(const std::string& name, WindowCoord pos) {
					auto monitorByName = findMonitorByName(monitors(), name);
					if (monitorByName != Monitor::NullMonitor()) return monitorByName;

					auto monitorsAt = ak::window::getMonitorsAt(pos, {0,0});
					if (monitorsAt.size() == 1) {
						if (monitorsAt.front() != Monitor::NullMonitor()) return monitorsAt.front();
						return primaryMonitor();
					}

					monitorByName = findMonitorByName(monitorsAt, name);
					if (monitorByName != Monitor::NullMonitor()) return monitorByName;
					return primaryMonitor();
				}

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
				WindowOptions& glStereo(bool val) { m_glStereo = val; return *this; }
				WindowOptions& glsRGB(bool val) { m_glSRGB = val; return *this; }
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
				bool glStereo() const { return m_glStereo; }
				bool glsRGB() const { return m_glSRGB; }
				bool glDoubleBuffer() const { return m_glDoubleBuffer; }
				VSync glVSync() const { return m_glVSync; }
				int glMSAA() const { return m_glMSSA; }

				static WindowOptions deserialize(const ak::data::PValue& root) {
					WindowCoord pos = {root["pos"][0].as<int>(), root["pos"][1].as<int>()};
					WindowCoord winSize = {root["videoMode"]["size"][0].as<int>(), root["videoMode"]["size"][1].as<int>()};
					VideoMode videoMode = {winSize, root["videoMode"]["refreshRate"].as<int>()};

					auto targetMonitor = findTargetMonitor(root["monitor"]["name"].asStr(), {root["monitor"]["pos"][0].as<int>(), root["monitor"]["pos"][1].as<int>()});

					VSync vsyncType = vsyncType = VSync::NONE;
					auto vsyncStr = root["glVSync"].asStr();
					if (vsyncStr == "None") vsyncType = VSync::NONE;
					else if (vsyncStr == "Full") vsyncType = VSync::FULL;
					else if (vsyncStr == "Half") vsyncType = VSync::HALF;
					else if (vsyncStr == "Adaptive") vsyncType = VSync::ADAPTIVE;

					return WindowOptions()
						.position(pos)
						.title(root["title"].asStr())

						.fullscreen(root["isFullscreen"].as<bool>())
						.centerOnMonitor(root["centerOnMonitor"].as<bool>())

						.targetMonitor(targetMonitor)
						.videoMode(videoMode)

						.resizable(root["isResizable"].as<bool>())
						.visible(root["isVisible"].as<bool>())
						.decorated(root["isDecorated"].as<bool>())
						.alwaysOnTop(root["isAlwaysOnTop"].as<bool>())
						.maximised(root["isMinimised"].as<bool>())

						.glVSync(vsyncType)

						.glVersion(root["glVersion"][0].as<int>(), root["glVersion"][1].as<int>())
						.glCore(root["glCore"].as<bool>())
						.glForwardCompat(root["glForward"].as<bool>())
						.glDebugContext(root["glDebug"].as<bool>())
						.glStereo(root["glStereo"].as<bool>())
						.glsRGB(root["glSRGB"].as<bool>())
						.glDoubleBuffer(root["glDoubleBuffer"].as<bool>())
						.glMSAA(root["glMSAA"].as<uint8>());
				}

				void serialize(ak::data::PValue& root) {
					root["pos"][0].setInt(m_offsetPosition.x);
					root["pos"][1].setInt(m_offsetPosition.y);

					root["title"].setStr(m_title);

					root["isFullscreen"].setBool(m_fullscreen);
					root["centerOnMonitor"].setBool(m_centerOnMonitor);

					root["monitor"]["name"].setStr(m_targetMonitor.name);
					root["monitor"]["pos"][0].setInt(m_targetMonitor.position.x);
					root["monitor"]["pos"][1].setInt(m_targetMonitor.position.y);

					root["videoMode"]["refreshRate"].setInt(m_videoMode.refreshRate);
					root["videoMode"]["size"][0].setInt(m_videoMode.resolution.x);
					root["videoMode"]["size"][1].setInt(m_videoMode.resolution.y);

					root["isResizable"].setBool(m_isResizable);
					root["isVisible"].setBool(m_isVisible);
					root["isDecorated"].setBool(m_isDecorated);
					root["isAlwaysOnTop"].setBool(m_alwaysOnTop);
					root["isMinimised"].setBool(m_minimised);

					switch(m_glVSync) {
						case VSync::NONE:      root["glVSync"].setStr("None"); break;
						case VSync::FULL:      root["glVSync"].setStr("Full"); break;
						case VSync::HALF:      root["glVSync"].setStr("Half"); break;
						case VSync::ADAPTIVE:  root["glVSync"].setStr("Adaptive"); break;
					}

					root["glVersion"][0].setInt(m_glMajor);
					root["glVersion"][1].setInt(m_glMinor);
					root["glCore"].setBool(m_glCore);
					root["glForward"].setBool(m_glForward);
					root["glDebug"].setBool(m_glDebug);
					root["glStereo"].setBool(m_glStereo);
					root["glSRGB"].setBool(m_glSRGB);
					root["glDoubleBuffer"].setBool(m_glDoubleBuffer);
					root["glMSAA"].setInt(m_glMSSA);
				}
		};
	}
}

#endif
