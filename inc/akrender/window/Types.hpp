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

#ifndef AK_RENDER_WINDOW_TYPES_HPP_
#define AK_RENDER_WINDOW_TYPES_HPP_

#include <akmath/Vector.hpp>

namespace akr {
	namespace win {

		using RealCoord  = akm::Vec2;
		using WindowCoord = akm::Vec2;
		using FrameCoord  = akm::Vec2;

	/*	struct RealCoord {
			int x;
			int y;

			bool operator==(const RealCoord& other) const { return x == other.x && y == other.y; }
		};

		struct WindowCoord {
			int x;
			int y;

			bool operator==(const WindowCoord& other) const { return x == other.x && y == other.y; }
		};

		struct FrameCoord {
			int x;
			int y;

			bool operator==(const FrameCoord& other) const { return x == other.x && y == other.y; }
		};*/

		enum class CursorMode {
			Captured,
			Hidden,
			Normal
		};
	}
}

#endif
