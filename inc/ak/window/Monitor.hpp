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

#ifndef AK_WINDOW_MONITOR_HPP_
#define AK_WINDOW_MONITOR_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/window/Types.hpp>
#include <string>
#include <vector>

namespace akw {

	struct VideoMode {
		WindowCoord resolution;
		int refreshRate;

		static VideoMode DefaultVideoMode() { return {{800, 600}, 0}; }

		bool operator==(const VideoMode& other) const { return resolution == other.resolution; }
	};

	struct Monitor {
		void* handle;

		std::string name;
		WindowCoord position;
		RealCoord realSizeMM;

		VideoMode prefVideoMode;
		std::vector<VideoMode> videoModes;

		static Monitor NullMonitor() { return {nullptr, "NullMonitor", {0, 0}, {0, 0}, VideoMode::DefaultVideoMode(), {VideoMode::DefaultVideoMode()}}; }

		bool operator==(const Monitor& other) const { return handle == other.handle; }
		bool operator!=(const Monitor& other) const { return handle != other.handle; }
	};

	std::vector<Monitor> monitors();

	std::vector<Monitor> getMonitorsAt(WindowCoord pos, WindowCoord size = {0,0});
	std::vector<Monitor> getAllWindowMonitors();

	Monitor primaryMonitor();
	Monitor currentMonitor();

	void setGamma(const Monitor* monitor, fpDouble gamma);

}

#endif
