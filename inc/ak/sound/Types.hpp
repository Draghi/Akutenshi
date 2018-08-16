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

#ifndef AK_SOUND_TYPES_HPP_
#define AK_SOUND_TYPES_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/Enums.hpp>
#include <memory>
#include <string>
#include <vector>

namespace aks {
	namespace internal {
		struct DeviceIdentifier;
		struct DeviceIdentifierDeletetor final {
			DeviceIdentifierDeletetor() = default;
			void operator()(DeviceIdentifier* p) const;
		};
	}

	using DeviceIdentifier = std::unique_ptr<aks::internal::DeviceIdentifier, internal::DeviceIdentifierDeletetor>;

	struct DeviceInfo {
		DeviceIdentifier identifier;
		std::string name;
		std::vector<Format> nativeFormats;
		struct { uint32 min, max; } channelLimits;
		struct { uint32 min, max; } sampleRange;
	};

	struct ContextInfo {
		std::string name;
	};

	using upload_callback_f = akSize(void* audioFrames, akSize frameCount, aks::Format format, aks::ChannelMap channelMap);
}



#endif /* AK_SOUND_TYPES_HPP_ */
