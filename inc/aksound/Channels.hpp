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

#ifndef AK_SOUND_CHANNELS_HPP_
#define AK_SOUND_CHANNELS_HPP_

namespace aks {
	enum class Channel : uint8 {
		None = 0,

		Mono = 1,

		FrontLeft = 2,  FrontCenter = 4, FrontRight = 3,
		SideLeft  = 11,         LFE = 5,  SideRight = 12,
		BackLeft  = 6,  BackCenter = 10,  BackRight = 7,

		Left  = FrontLeft,
		Right = FrontRight,

		// Not officially supported.
		FrontLeftCenter  = 8,
		FrontRightCenter = 9,
		TopCenter = 13,
		TopFrontLeft = 14,
		TopFrontCenter = 15,
		TopFrontRight = 16,
		TopBackLeft = 17,
		TopBackCenter = 18,
		TopBackRight = 19,
	};
}



#endif /* AKSOUND_BACKEND_TYPES_HPP_ */
