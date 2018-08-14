
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
#include <ak/sound/Types.hpp>
#include <ak/sound/Backend.hpp>
#include <ak/sound/internal/MalUtil.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/util/Iterator.hpp>
#include <mini_al.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <string>

using namespace aks;

namespace aks {
	namespace internal {
		struct DeviceIdentifier final { mal_device_id id; };
		void DeviceIdentifierDeletetor::operator()(DeviceIdentifier* p) const { delete p; }
	}
}

static bool initContext(mal_context* context, const std::vector<Backend>& backends);
static bool initDevice(mal_context* context, mal_device* device, const DeviceIdentifier& deviceID, uint32 sampleRate, Format format, const std::vector<Channel>& channels);

static void recvMalMessage(mal_context* /*pContext*/, mal_device* /*pDevice*/, mal_uint32 logLevel, const char* message);
static mal_uint32 malCallback_internal(mal_device* /*device*/, mal_uint32 frameCount, void* dst);

static std::atomic<bool> malIsInit = false;
static mal_context malContext;
static mal_device  malDevice;

static std::function<upload_callback_f> malCallback;
static std::vector<Channel> malChannels;
static Format malFormat;

bool aks::init(const DeviceIdentifier& deviceID, uint32 sampleRate, Format format, const std::vector<Channel>& channels, const std::function<upload_callback_f>& callback) {
	return aks::init(aks::internal::DEFAULT_BACKENDS, deviceID, sampleRate, format, channels, callback);
}

bool aks::init(const std::vector<Backend>& backends, const DeviceIdentifier& deviceID, uint32 sampleRate, Format format, const std::vector<Channel>& channels, const std::function<upload_callback_f>& callback) {
	if (malIsInit.exchange(true)) return false;
	ak::ScopeGuard resetInitFlag([&]{malIsInit = false;});

	if (!initContext(&malContext, backends)) return false;
	if (!initDevice(&malContext, &malDevice, deviceID, sampleRate, format, channels)) { mal_context_uninit(&malContext); return false; }

	malCallback = callback; malChannels = channels; malFormat = format;
	resetInitFlag.clear();
	return true;
}

void aks::startDevice() { if (malIsInit) mal_device_start(&malDevice); }
void  aks::stopDevice() { if (malIsInit) mal_device_stop(&malDevice); }

bool aks::isDeviceStarted() { return (malIsInit) && (mal_device_is_started(&malDevice) == MAL_TRUE); }

static std::vector<DeviceInfo> getAvailableDevicesForContext(mal_context* context) {
	mal_device_info* deviceInfo = nullptr; mal_uint32 deviceCount = 0;
	mal_context_get_devices(context, &deviceInfo, &deviceCount, nullptr, nullptr);

	std::vector<DeviceInfo> result; result.reserve(deviceCount);
	for(mal_uint32 i = 0; i < deviceCount; i++) {
		auto& device = deviceInfo[i];
		result.push_back(DeviceInfo{
			DeviceIdentifier(new internal::DeviceIdentifier{device.id}),
			std::string(device.name),
			aku::convert_to_if<std::vector<aks::Format>>(device.formats, device.formats+device.formatCount, internal::fromMalFormat),
			{device.minChannels, device.maxChannels},
			{device.minSampleRate, device.maxSampleRate}
		});
	}

	return result;
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

std::vector<DeviceInfo> aks::getAvailableDevices() { return malIsInit ? getAvailableDevicesForContext(&malContext) : std::vector<DeviceInfo>(); }
std::vector<DeviceInfo> aks::getAvailableDevices(Backend backend) {
	if ((malIsInit) && (malContext.backend == internal::toMalBackend(backend))) return getAvailableDevicesForContext(&malContext);
	mal_context tmpContext;
	if (!initContext(&tmpContext, std::vector<Backend>({backend}))) return std::vector<DeviceInfo>();
	auto result = getAvailableDevicesForContext(&tmpContext);
	mal_context_uninit(&tmpContext);
	return result;
}

static bool initContext(mal_context* context, const std::vector<Backend>& backends) {
	std::vector<mal_backend> malBackends; malBackends.reserve(backends.size());
	std::for_each(backends.begin(), backends.end(), [&](Backend backend){ malBackends.push_back(internal::toMalBackend(backend)); });

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

static bool initDevice(mal_context* context, mal_device* device, const DeviceIdentifier& deviceID, uint32 sampleRate, Format format, const std::vector<Channel>& channels) {
	if (channels.size() > MAL_MAX_CHANNELS) {
		akl::Logger("MAL").error("Number of channels(", channels.size() ,") greater than max allowed: ", MAL_MAX_CHANNELS);
		return false;
	}

	auto bufferFrames = mal_calculate_default_buffer_size_in_frames(mal_performance_profile::mal_performance_profile_low_latency, sampleRate, 1);
	mal_device_config cfg{
		internal::toMalFormat(format),       // mal_format format;
		channels.size(),                     // mal_uint32 channels;
		sampleRate,                          // mal_uint32 sampleRate;
		{},                                  // mal_channel channelMap[MAL_MAX_CHANNELS];
		bufferFrames,                        // mal_uint32 bufferSizeInFrames;
		2,                                   // mal_uint32 periods;
		mal_share_mode_shared,               // mal_share_mode shareMode;
		mal_performance_profile_low_latency, // mal_performance_profile performanceProfile;
		nullptr,                             // mal_recv_proc onRecvCallback;
		malCallback_internal,                // mal_send_proc onSendCallback;
		nullptr,                             // mal_stop_proc onStopCallback;
		{false},                             // struct {  mal_bool32 noMMap;  // Disables MMap mode. } alsa;
		{nullptr}                            // struct { const char* pStreamName; } pulse;
	};

	for(akSize i = 0; i < channels.size(); i++) cfg.channelMap[i] = static_cast<uint8>(channels[i]);

	if (mal_device_init(context, mal_device_type::mal_device_type_playback, deviceID ? &deviceID->id : nullptr, &cfg, nullptr, device) != MAL_SUCCESS) {
		akl::Logger("MAL").error("Failed to create audio device.");
		return false;
	}

	return true;
}

static void recvMalMessage(mal_context* /*pContext*/, mal_device* /*pDevice*/, mal_uint32 logLevel, const char* message) {
	switch(logLevel) {
		case MAL_LOG_LEVEL_ERROR:   akl::Logger("MAL").error(message); return;
		case MAL_LOG_LEVEL_WARNING: akl::Logger("MAL").warn(message);  return;
		case MAL_LOG_LEVEL_INFO:    akl::Logger("MAL").info(message);  return;
		case MAL_LOG_LEVEL_VERBOSE: akl::Logger("MAL").debug(message); return;

		default: akl::Logger("MAL").info(message); return;
	}

}

static mal_uint32 malCallback_internal(mal_device* /*device*/, mal_uint32 frameCount, void* dst) { return malCallback(dst, frameCount, malFormat, malChannels); }
