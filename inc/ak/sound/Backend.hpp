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

#ifndef AK_SOUND_BACKEND_HPP_
#define AK_SOUND_BACKEND_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

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

	inline akSize getFormatElementSize(Format format) {
		switch(format) {
			case Format::UInt8:    return 1;
			case Format::SInt16:   return 2;
			case Format::SInt24:   return 3;
			case Format::SInt32:   return 4;
			case Format::FPSingle: return 4;
		}
	}

	enum class Channel : uint8 {
		None = 0,

		Mono = 1,

		FrontLeft = 2,  FrontCenter = 4, FrontRight = 3,
		SideLeft  = 11,         LFE = 5,  SideRight = 12,
		BackLeft  = 6,  BackCenter = 10,  BackRight = 7,

		Left  = FrontLeft,
		Right = FrontRight

		/* FrontLeftCenter  = 8, FrontRightCenter = 9, TopCenter = 13, TopFrontLeft = 14, TopFrontCenter = 15, TopFrontRight = 16, TopBackLeft = 17, TopBackCenter = 18, TopBackRight = 19,*/
	};

	struct ChannelMap final {
		ChannelMap() = delete;
		static const std::vector<Channel> Mono1;
		static const std::vector<Channel> Stereo2;
		static const std::vector<Channel> Stereo3;
		static const std::vector<Channel> Surround3;
		static const std::vector<Channel> Surround4;
		static const std::vector<Channel> Surround5;
		static const std::vector<Channel> Surround6;
		static const std::vector<Channel> Surround7;

		static const std::vector<Channel> Mono1LF;
		static const std::vector<Channel> Stereo2LF;
		static const std::vector<Channel> Stereo3LF;
		static const std::vector<Channel> Surround3LF;
		static const std::vector<Channel> Surround4LF;
		static const std::vector<Channel> Surround5LF;
		static const std::vector<Channel> Surround6LF;
		static const std::vector<Channel> Surround7LF;
	};

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

	using upload_callback_f = akSize(void* audioFrames, akSize frameCount, aks::Format format, const std::vector<aks::Channel>& channels);
	bool init(const DeviceIdentifier& deviceID, uint32 sampleRate, Format format, const std::vector<Channel>& channels, const std::function<upload_callback_f>& callback);
	bool init(const std::vector<Backend>& backends, const DeviceIdentifier& deviceID, uint32 sampleRate, Format format, const std::vector<Channel>& channels, const std::function<upload_callback_f>& callback);

	void startDevice();
	void stopDevice();
	bool isDeviceStarted();

	ContextInfo getContextInfo();

	enum class DitherMode {
		None,
		Rectangular,
		Trianglar
	};

	void convertPCMSamples(void* sampleOut, Format formatOut, const void* sampleIn, Format formatIn, akSize sampleCount, DitherMode dither = DitherMode::Trianglar);

	std::vector<DeviceInfo> getAvailableDevices(Backend backends);
	std::vector<DeviceInfo> getAvailableDevices();
}

#endif /* AK_SOUND_BACKEND_HPP_ */
