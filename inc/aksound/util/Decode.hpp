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

#ifndef AK_SOUND_UTIL_DECODE_HPP_
#define AK_SOUND_UTIL_DECODE_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <aksound/backend/Types.hpp>
#include <aksound/Channels.hpp>
#include <aksound/sampler/SamplerBuffer.hpp>
#include <unordered_map>
#include <vector>

namespace aks {
	class Sound;

	std::unordered_map<Channel, aks::SamplerBuffer> decode(const std::vector<uint8>& data, bool shouldLoop = false, aks::backend::DitherMode ditherMode = aks::backend::DitherMode::Trianglar);
}

#endif /* AKSOUND_BACKEND_DECODE_HPP_ */
