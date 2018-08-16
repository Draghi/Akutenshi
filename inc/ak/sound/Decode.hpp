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

#ifndef AK_SOUND_DECODE_HPP_
#define AK_SOUND_DECODE_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <vector>
#include <optional>

namespace aks {
	class Buffer;
} /* namespace aks */

namespace aks {

	std::optional<aks::Buffer> decode(const std::vector<uint8>& data, Format format, ChannelMap channelMap, akSize sampleRate, DitherMode ditherMode = DitherMode::Trianglar);

}

#endif /* AK_SOUND_DECODE_HPP_ */
