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

#include <AkCommon/PrimitiveTypes.hpp>
#include <AkSound/backend/Types.hpp>
#include <AkSound/util/Types.hpp>
#include <vector>

namespace aks {
	namespace backend {
		akSize resampleStream(void* samplesOut, akSize frameCountOut, StreamFormat streamFormatOut, const void* samplesIn, akSize frameCountIn, StreamFormat streamFormatIn, DitherMode ditherMode = DitherMode::Trianglar);
		const std::vector<Channel>& channelLayoutOf(ChannelMap channelMap);
		akSize frameSizeOf(StreamFormat streamFormat);
		akSize elementSizeOf(Format format);
		akSize calcResampledFrameCount(akSize sampleRateOld, akSize sampleRateNew, akSize frameCount);
	}
}

#endif /* AK_SOUND_UTIL_HPP_ */
