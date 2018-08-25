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

#ifndef AK_SOUND_BACKEND_TYPES_HPP_
#define AK_SOUND_BACKEND_TYPES_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <memory>
#include <string>
#include <vector>

namespace aks {
	namespace backend {
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

		namespace internal {
			struct DeviceIdentifier;
			struct DeviceIdentifierDeletetor final {
				DeviceIdentifierDeletetor() = default;
				void operator()(DeviceIdentifier* p) const;
			};
		}

		using DeviceIdentifier = std::unique_ptr<aks::backend::internal::DeviceIdentifier, internal::DeviceIdentifierDeletetor>;

		struct DeviceCapabilities {
			DeviceIdentifier identifier;
			std::string name;
			std::vector<Format> nativeFormats;
			struct { uint32 min, max; } channelLimits;
			struct { uint32 min, max; } sampleRange;
		};


		struct ContextInfo {
			std::string name;
		};

		struct StreamFormat final {
			Format format;
			ChannelMap channelMap;
			akSize sampleRate;

			bool operator==(const StreamFormat& o) const { return (format == o.format) && (channelMap == o.channelMap) && (sampleRate == o.sampleRate); }
			bool operator!=(const StreamFormat& o) const { return !(*this == o); }
		};

		struct DeviceInfo {
			std::string name;
			StreamFormat streamFormat;
		};

		using upload_callback_f = akSize(void* audioFrames, akSize frameCount, StreamFormat streamFormat);
	}
}



#endif /* AK_SOUND_TYPES_HPP_ */
