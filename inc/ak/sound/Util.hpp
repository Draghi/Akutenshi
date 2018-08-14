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

#ifndef AK_SOUND_UTIL_HPP_
#define AK_SOUND_UTIL_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/Enums.hpp>
#include <vector>

namespace aks {
	class Buffer;
}

namespace aks {
	struct ChannelMap final {
		ChannelMap() = delete;
		static const std::vector<Channel> Mono1,   Stereo2,   Stereo3,   Surround3,   Surround4,   Surround5,   Surround6,   Surround7;
		static const std::vector<Channel> Mono1LF, Stereo2LF, Stereo3LF, Surround3LF, Surround4LF, Surround5LF, Surround6LF, Surround7LF;
	};

	void convertPCMSamples(void* sampleOut, Format formatOut, const void* sampleIn, Format formatIn, akSize sampleCount, DitherMode dither = DitherMode::Trianglar);

	aks::Buffer generateSineWave(akSize sampleRate, fpSingle frequency, Format format);

	akSize getFormatElementSize(Format format);
}

#endif /* AK_SOUND_UTIL_HPP_ */
