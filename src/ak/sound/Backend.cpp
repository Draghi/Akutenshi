
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

#include <ak/Log.hpp>
#include <ak/sound/Backend.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/util/Iterator.hpp>
#include <mini_al.h>
#include <algorithm>
#include <atomic>
#include <stdexcept>

using namespace aks;

static const std::vector<Backend> DEFAULT_BACKENDS = {
	Backend::WASAPI, Backend::DSound,    Backend::WinMM,      // Windows
	Backend::JACK,   Backend::ALSA,      Backend::PulseAudio, // Linux
	Backend::SndIO,  Backend::AudioIO,   Backend::OSS,        // BSD
	Backend::OpenAL, Backend::SDL,                            // Fallbacks
	Backend::OpenSL, Backend::CoreAudio, Backend::Null        // Other OSes
};

static void recvMalMessage(mal_context* /*pContext*/, mal_device* /*pDevice*/, const char* message);
static mal_backend toMalBackend(Backend backend);
static std::optional<aks::DeviceFormat> malToFormat(mal_format v);
static mal_format formatToMal(aks::DeviceFormat v);
static mal_uint32 malCallback_internal(mal_device* /*device*/, mal_uint32 frameCount, void* dst);

static bool initContext(mal_context* context, const std::vector<Backend>& backends);
static bool initDevice(mal_context* context, mal_device* device, const DeviceIdentifier& deviceID, uint32 sampleRate, DeviceFormat format, const std::vector<Channel>& channels);

namespace aks {
	namespace internal {
		struct DeviceIdentifier final { mal_device_id id; };
		void DeviceIdentifierDeletetor::operator()(DeviceIdentifier* p) const { delete p; }
	}
}

static std::atomic<bool> malIsInit = false;
static mal_context malContext;
static mal_device  malDevice;

static std::function<upload_callback_f> malCallback;
static std::vector<Channel> malChannels;
static DeviceFormat malFormat;

bool aks::init(const DeviceIdentifier& deviceID, uint32 sampleRate, DeviceFormat format, const std::vector<Channel>& channels, const std::function<upload_callback_f>& callback) {
	return aks::init(DEFAULT_BACKENDS, deviceID, sampleRate, format, channels, callback);
}

bool aks::init(const std::vector<Backend>& backends, const DeviceIdentifier& deviceID, uint32 sampleRate, DeviceFormat format, const std::vector<Channel>& channels, const std::function<upload_callback_f>& callback) {
	if (malIsInit.exchange(true)) return false;
	ak::ScopeGuard resetInitFlag([&]{malIsInit = false;});

	if (!initContext(&malContext, backends)) return false;
	if (!initDevice(&malContext, &malDevice, deviceID, sampleRate, format, channels)) { mal_context_uninit(&malContext); return false; }

	resetInitFlag.clear();

	malCallback = callback;
	malChannels = channels;
	malFormat   = format;

	return true;
}

void aks::startDevice() {
	if (!malIsInit) return;
	mal_device_start(&malDevice);
}

void aks::stopDevice() {
	if (!malIsInit) return;
	mal_device_stop(&malDevice);
}

bool aks::isDeviceStarted() {
	if (!malIsInit) return false;
	return mal_device_is_started(&malDevice) == MAL_TRUE;
}

static std::vector<DeviceInfo> getAvailableDevicesForContext(mal_context* context) {
	mal_device_info* deviceInfo = nullptr; mal_uint32 deviceCount = 0;
	mal_context_get_devices(context, &deviceInfo, &deviceCount, nullptr, nullptr);

	std::vector<DeviceInfo> result; result.reserve(deviceCount);
	for(mal_uint32 i = 0; i < deviceCount; i++) {
		auto& device = deviceInfo[i];
		result.push_back(DeviceInfo{
			DeviceIdentifier(new internal::DeviceIdentifier{device.id}),
			std::string(device.name),
			aku::convert_to_if<std::vector<aks::DeviceFormat>>(device.formats, device.formats+device.formatCount, malToFormat),
			{device.minChannels, device.maxChannels},
			{device.minSampleRate, device.maxSampleRate}
		});
	}

	return result;
}

std::vector<DeviceInfo> aks::getAvailableDevices(Backend backend) {
	if ((malIsInit) && (malContext.backend == toMalBackend(backend))) return getAvailableDevicesForContext(&malContext);
	mal_context tmpContext;
	if (!initContext(&tmpContext, std::vector<Backend>({backend}))) return std::vector<DeviceInfo>();
	auto result = getAvailableDevicesForContext(&tmpContext);
	mal_context_uninit(&tmpContext);
	return result;
}

std::vector<DeviceInfo> aks::getAvailableDevices() {
	if (!malIsInit) return std::vector<DeviceInfo>();
	return getAvailableDevicesForContext(&malContext);
}

ContextInfo aks::getContextInfo() {
	switch(malContext.backend) {
		case mal_backend::mal_backend_null:       return {"Null"};
		case mal_backend::mal_backend_wasapi:     return {"WASAPI"};
		case mal_backend::mal_backend_dsound:     return {"DirectSound"};
		case mal_backend::mal_backend_winmm:      return {"WinMM"};
		case mal_backend::mal_backend_alsa:       return {"ALSA"};
		case mal_backend::mal_backend_pulseaudio: return {"PulseAudio"};
		case mal_backend::mal_backend_jack:       return {"JACK"};
		case mal_backend::mal_backend_coreaudio:  return {"CoreAudio"};
		case mal_backend::mal_backend_sndio:      return {"SndIO"};
		case mal_backend::mal_backend_audioio:    return {"AudioIO"};
		case mal_backend::mal_backend_oss:        return {"OSS"};
		case mal_backend::mal_backend_opensl:     return {"OpenSL"};
		case mal_backend::mal_backend_openal:     return {"OpenAL"};
		case mal_backend::mal_backend_sdl:        return {"SDL"};
	}
}



static bool initContext(mal_context* context, const std::vector<Backend>& backends) {
	std::vector<mal_backend> malBackends; malBackends.reserve(backends.size());
	std::for_each(backends.begin(), backends.end(), [&](Backend backend){ malBackends.push_back(toMalBackend(backend)); });

	mal_context_config malConfig = {
		&recvMalMessage,
		mal_thread_priority::mal_thread_priority_default,
		{ MAL_TRUE },
		{ nullptr, nullptr, MAL_TRUE },
		{ nullptr, MAL_TRUE }
	};

	if (mal_context_init(malBackends.data(), malBackends.size(), &malConfig, context) != MAL_SUCCESS) {
		akl::Logger("MAL").error("Failed to create audio context.");
		return false;
	}

	return true;
}

static bool initDevice(mal_context* context, mal_device* device, const DeviceIdentifier& deviceID, uint32 sampleRate, DeviceFormat format, const std::vector<Channel>& channels) {
	if (channels.size() > MAL_MAX_CHANNELS) {
		akl::Logger("MAL").error("Number of channels(", channels.size() ,") greater than max allowed: ", MAL_MAX_CHANNELS);
		return false;
	}

	auto bufferFrames = mal_calculate_default_buffer_size_in_frames(mal_performance_profile::mal_performance_profile_low_latency, sampleRate, 1);
	mal_device_config cfg{
		formatToMal(format),   // mal_format format;
		channels.size(),       // mal_uint32 channels;
		sampleRate,            // mal_uint32 sampleRate;
		{},                    // mal_channel channelMap[MAL_MAX_CHANNELS];
		bufferFrames,          // mal_uint32 bufferSizeInFrames;
		2,                     // mal_uint32 periods;
		mal_share_mode_shared, // mal_share_mode shareMode;
		mal_performance_profile_low_latency, // mal_performance_profile performanceProfile;
		nullptr, // mal_recv_proc onRecvCallback;
		malCallback_internal, // mal_send_proc onSendCallback;
		nullptr, // mal_stop_proc onStopCallback;
		{false}, // struct {  mal_bool32 noMMap;  // Disables MMap mode. } alsa;
		{nullptr}// struct { const char* pStreamName; } pulse;
	};
	for(akSize i = 0; i < channels.size(); i++) cfg.channelMap[i] = static_cast<uint8>(channels[i]);

	if (mal_device_init(context, mal_device_type::mal_device_type_playback, deviceID ? &deviceID->id : nullptr, &cfg, nullptr, device) != MAL_SUCCESS) {
		akl::Logger("MAL").error("Failed to create audio device.");
		return false;
	}

	return true;
}


const std::vector<Channel> ChannelMap::Mono1      ({Channel::Mono});
const std::vector<Channel> ChannelMap::Stereo2    ({Channel::FrontLeft, Channel::FrontRight});
const std::vector<Channel> ChannelMap::Stereo3    ({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter});
const std::vector<Channel> ChannelMap::Surround3  ({Channel::FrontLeft, Channel::FrontRight, Channel::BackCenter});
const std::vector<Channel> ChannelMap::Surround4  ({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackCenter});
const std::vector<Channel> ChannelMap::Surround5  ({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackLeft, Channel::BackRight});
const std::vector<Channel> ChannelMap::Surround6  ({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackLeft, Channel::BackRight, Channel::BackCenter});
const std::vector<Channel> ChannelMap::Surround7  ({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackLeft, Channel::BackRight, Channel::SideLeft, Channel::SideRight});

const std::vector<Channel> ChannelMap::Mono1LF    ({Channel::Mono,      Channel::LFE});
const std::vector<Channel> ChannelMap::Stereo2LF  ({Channel::FrontLeft, Channel::FrontRight, Channel::LFE});
const std::vector<Channel> ChannelMap::Stereo3LF  ({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::LFE});
const std::vector<Channel> ChannelMap::Surround3LF({Channel::FrontLeft, Channel::FrontRight, Channel::BackCenter,  Channel::LFE});
const std::vector<Channel> ChannelMap::Surround4LF({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackCenter, Channel::LFE});
const std::vector<Channel> ChannelMap::Surround5LF({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackLeft,   Channel::BackRight, Channel::LFE});
const std::vector<Channel> ChannelMap::Surround6LF({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackLeft,   Channel::BackRight, Channel::BackCenter, Channel::LFE});
const std::vector<Channel> ChannelMap::Surround7LF({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackLeft,   Channel::BackRight, Channel::SideLeft,   Channel::SideRight, Channel::LFE});

static void recvMalMessage(mal_context* /*pContext*/, mal_device* /*pDevice*/, const char* message) {
	akl::Logger("MAL").info(message);
}

static mal_backend toMalBackend(Backend backend){
	switch(backend) {
		case Backend::Null:       return mal_backend::mal_backend_null;
		case Backend::WASAPI:     return mal_backend::mal_backend_wasapi;
		case Backend::DSound:     return mal_backend::mal_backend_dsound;
		case Backend::WinMM:      return mal_backend::mal_backend_winmm;
		case Backend::ALSA:       return mal_backend::mal_backend_alsa;
		case Backend::PulseAudio: return mal_backend::mal_backend_pulseaudio;
		case Backend::JACK:       return mal_backend::mal_backend_jack;
		case Backend::CoreAudio:  return mal_backend::mal_backend_coreaudio;
		case Backend::SndIO:      return mal_backend::mal_backend_sndio;
		case Backend::AudioIO:    return mal_backend::mal_backend_audioio;
		case Backend::OSS:        return mal_backend::mal_backend_oss;
		case Backend::OpenSL:     return mal_backend::mal_backend_opensl;
		case Backend::OpenAL:     return mal_backend::mal_backend_openal;
		case Backend::SDL:        return mal_backend::mal_backend_sdl;
		default: throw std::invalid_argument("Unhandled backend type.");
	}
}

static std::optional<aks::DeviceFormat> malToFormat(mal_format v) {
	switch(v) {
		case mal_format::mal_format_u8:  return {aks::DeviceFormat::UInt8};
		case mal_format::mal_format_s16: return {aks::DeviceFormat::SInt16};
		case mal_format::mal_format_s24: return {aks::DeviceFormat::SInt24};
		case mal_format::mal_format_s32: return {aks::DeviceFormat::SInt32};
		case mal_format::mal_format_f32: return {aks::DeviceFormat::FPSingle};
		case mal_format::mal_format_count:   [[fallthrough]];
		case mal_format::mal_format_unknown: [[fallthrough]];
		default: return {};
	}
}

static mal_format formatToMal(aks::DeviceFormat v) {
	switch(v) {
		case aks::DeviceFormat::UInt8:    return mal_format::mal_format_u8;
		case aks::DeviceFormat::SInt16:   return mal_format::mal_format_s16;
		case aks::DeviceFormat::SInt24:   return mal_format::mal_format_s24;
		case aks::DeviceFormat::SInt32:   return mal_format::mal_format_s32;
		case aks::DeviceFormat::FPSingle: return mal_format::mal_format_f32;
		default: throw std::invalid_argument("Unhandled audio format.");
	}
}

static mal_uint32 malCallback_internal(mal_device* /*device*/, mal_uint32 frameCount, void* dst) {
	return malCallback(dst, frameCount, malFormat, malChannels);
}
