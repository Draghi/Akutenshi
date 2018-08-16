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
	bool convertSamples(void* samplesOut, Format formatOut, akSize sampleRateOut, ChannelMap channelMapOut,
		          const void* samplesIn,  Format formatIn,  akSize sampleRateIn,  ChannelMap channelMapIn,
		          akSize frameCount, DitherMode dither = DitherMode::Trianglar);

	aks::Buffer generateSineWave(akSize sampleRate, fpSingle frequency, Format format, DitherMode dither = DitherMode::Trianglar);

	akSize getFormatElementSize(Format format);
	const std::vector<Channel>& getChannelLayoutFor(ChannelMap channelMap);
}

#endif /* AK_SOUND_UTIL_HPP_ */
