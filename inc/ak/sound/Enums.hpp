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

#ifndef AK_SOUND_ENUMS_HPP_
#define AK_SOUND_ENUMS_HPP_

#include <ak/PrimitiveTypes.hpp>

namespace aks {
	enum class Backend {
		Null,
		WASAPI, DSound, WinMM,
		ALSA, PulseAudio, JACK,
		CoreAudio,
		SndIO, AudioIO, OSS,
		OpenSL, OpenAL, SDL
	};

	enum class Format {
		UInt8,
		SInt16,
		SInt24,
		SInt32,
		FPSingle,
	};

	enum class Channel : uint8 {
		None = 0,

		Mono = 1,

		FrontLeft = 2,  FrontCenter = 4, FrontRight = 3,
		SideLeft  = 11,         LFE = 5,  SideRight = 12,
		BackLeft  = 6,  BackCenter = 10,  BackRight = 7,

		Left  = FrontLeft,
		Right = FrontRight

		 // FrontLeftCenter  = 8, FrontRightCenter = 9, TopCenter = 13, TopFrontLeft = 14, TopFrontCenter = 15, TopFrontRight = 16, TopBackLeft = 17, TopBackCenter = 18, TopBackRight = 19,
	};

	enum class ChannelMap : uint8 {
		Mono,
		Stereo,
		Stereo3,
		Surround3,
		Surround4,
		Surround5,
		Surround6,
		Surround7,

		Mono_LF,
		Stereo_LF,
		Stereo3_LF,
		Surround3_LF,
		Surround4_LF,
		Surround5_LF,
		Surround6_LF,
		Surround7_LF,
	};

	enum class DitherMode {
		None,
		Rectangular,
		Trianglar
	};
}

#endif /* AK_SOUND_ENUMS_HPP_ */
