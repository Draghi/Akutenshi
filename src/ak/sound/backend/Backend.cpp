
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
#include <ak/sound/backend/Backend.hpp>
#include <ak/sound/backend/internal/MalUtil.hpp>
#include <ak/sound/backend/Util.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/util/Iterator.hpp>
#include <mini_al.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <optional>
#include <string>

using namespace aks::backend;

namespace aks {
	namespace backend {
		namespace internal {
			struct DeviceIdentifier final { mal_device_id id; };
			void DeviceIdentifierDeletetor::operator()(DeviceIdentifier* p) const { delete p; }
		}
	}
}

static bool initContext(mal_context* context, const std::vector<Backend>& backends);
static bool initDevice(mal_context* context, mal_device* device, const DeviceIdentifier& deviceID, StreamFormat streamFormat, const std::function<upload_callback_f>& callback);

static void recvMalMessage(mal_context* /*pContext*/, mal_device* /*pDevice*/, mal_uint32 logLevel, const char* message);
static mal_uint32 malCallback_internal(mal_device* /*device*/, mal_uint32 frameCount, void* dst);

static std::atomic<bool> malIsInit = false;
static mal_context malContext;
static mal_device  malDevice;

bool aks::backend::init(const DeviceIdentifier& deviceID, StreamFormat streamFormat, const std::function<upload_callback_f>& callback) {
	return init(internal::DEFAULT_BACKENDS, deviceID, streamFormat, callback);
}

bool aks::backend::init(const std::vector<Backend>& backends, const DeviceIdentifier& deviceID, StreamFormat streamFormat, const std::function<upload_callback_f>& callback) {
	if (malIsInit.exchange(true)) return false;
	ak::ScopeGuard resetInitFlag([&]{malIsInit = false;});

	if (!initContext(&malContext, backends)) return false;
	if (!initDevice(&malContext, &malDevice, deviceID, streamFormat, callback)) { mal_context_uninit(&malContext); return false; }

	resetInitFlag.clear();

	return true;
}

void aks::backend::startDevice() { if (malIsInit) mal_device_start(&malDevice); }
void  aks::backend::stopDevice() { if (malIsInit) mal_device_stop(&malDevice); }

bool aks::backend::isDeviceStarted() { return (malIsInit) && (mal_device_is_started(&malDevice) == MAL_TRUE); }

std::optional<ContextInfo> aks::backend::getContextInfo() {
	if (!malIsInit) return {};
	switch(malContext.backend) {
		case mal_backend::mal_backend_null:       return ContextInfo{"Null"};
		case mal_backend::mal_backend_wasapi:     return ContextInfo{"WASAPI"};
		case mal_backend::mal_backend_dsound:     return ContextInfo{"DirectSound"};
		case mal_backend::mal_backend_winmm:      return ContextInfo{"WinMM"};
		case mal_backend::mal_backend_alsa:       return ContextInfo{"ALSA"};
		case mal_backend::mal_backend_pulseaudio: return ContextInfo{"PulseAudio"};
		case mal_backend::mal_backend_jack:       return ContextInfo{"JACK"};
		case mal_backend::mal_backend_coreaudio:  return ContextInfo{"CoreAudio"};
		case mal_backend::mal_backend_sndio:      return ContextInfo{"SndIO"};
		case mal_backend::mal_backend_audioio:    return ContextInfo{"AudioIO"};
		case mal_backend::mal_backend_oss:        return ContextInfo{"OSS"};
		case mal_backend::mal_backend_opensl:     return ContextInfo{"OpenSL"};
		case mal_backend::mal_backend_openal:     return ContextInfo{"OpenAL"};
		case mal_backend::mal_backend_sdl:        return ContextInfo{"SDL"};
	}
}

std::optional<DeviceInfo> aks::backend::getDeviceInfo() {
	if (!malIsInit) return {};
	return DeviceInfo{
		std::string(malDevice.name),
		{
			*internal::fromMalFormat(malDevice.format),
			*internal::fromMalChannels(malDevice.channelMap, malDevice.channels),
			malDevice.sampleRate
		}
	};
}


static std::vector<DeviceCapabilities> getAvailableDevicesForContext(mal_context* context) {
	mal_device_info* deviceInfo = nullptr; mal_uint32 deviceCount = 0;
	mal_context_get_devices(context, &deviceInfo, &deviceCount, nullptr, nullptr);

	std::vector<DeviceCapabilities> result; result.reserve(deviceCount);
	for(mal_uint32 i = 0; i < deviceCount; i++) {
		auto& device = deviceInfo[i];
		result.push_back(DeviceCapabilities{
			DeviceIdentifier(new internal::DeviceIdentifier{device.id}),
			std::string(device.name),
			aku::convert_to_if<std::vector<Format>>(device.formats, device.formats+device.formatCount, internal::fromMalFormat),
			{device.minChannels, device.maxChannels},
			{device.minSampleRate, device.maxSampleRate}
		});
	}

	return result;
}

std::vector<DeviceCapabilities> aks::backend::getAvailableDevices() {
	return malIsInit ? getAvailableDevicesForContext(&malContext) : std::vector<DeviceCapabilities>();
}

std::vector<DeviceCapabilities> aks::backend::getAvailableDevices(Backend backend) {
	if ((malIsInit) && (malContext.backend == internal::toMalBackend(backend))) return getAvailableDevicesForContext(&malContext);
	mal_context tmpContext;
	if (!initContext(&tmpContext, std::vector<Backend>({backend}))) return std::vector<DeviceCapabilities>();
	auto result = getAvailableDevicesForContext(&tmpContext);
	mal_context_uninit(&tmpContext);
	return result;
}

akSize aks::backend::audioBufferSize() {
	if (!malIsInit) return 0;
	return malDevice.bufferSizeInFrames;
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

struct MalUserData {
	std::function<upload_callback_f> callback;
	StreamFormat streamFormat;
};

static bool initDevice(mal_context* context, mal_device* device, const DeviceIdentifier& deviceID, StreamFormat streamFormat, const std::function<upload_callback_f>& callback) {
	auto bufferFrames = mal_calculate_default_buffer_size_in_frames(mal_performance_profile::mal_performance_profile_low_latency, streamFormat.sampleRate, 1);
	auto& channels = channelLayoutOf(streamFormat.channelMap);

	mal_device_config cfg{
		internal::toMalFormat(streamFormat.format), // mal_format format;
	    channels.size(),                            // mal_uint32 channels;
		streamFormat.sampleRate,                    // mal_uint32 sampleRate;
		{},                                         // mal_channel channelMap[MAL_MAX_CHANNELS];
		bufferFrames,                               // mal_uint32 bufferSizeInFrames;
		4,                                          // mal_uint32 periods;
		mal_share_mode_shared,                      // mal_share_mode shareMode;
		mal_performance_profile_low_latency,        // mal_performance_profile performanceProfile;
		nullptr,                                    // mal_recv_proc onRecvCallback;
		malCallback_internal,                       // mal_send_proc onSendCallback;
		nullptr,                                    // mal_stop_proc onStopCallback;
		{false},                                    // struct {  mal_bool32 noMMap;  // Disables MMap mode. } alsa;
		{nullptr}                                   // struct { const char* pStreamName; } pulse;
	};
	for(akSize i = 0; i < cfg.channels; i++) cfg.channelMap[i] = static_cast<uint8>(channels[i]);

	if (mal_device_init(context, mal_device_type::mal_device_type_playback, deviceID ? &deviceID->id : nullptr, &cfg, new MalUserData{callback, streamFormat}, device) != MAL_SUCCESS) {
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

static mal_uint32 malCallback_internal(mal_device* device, mal_uint32 frameCount, void* dst) {
	return static_cast<MalUserData*>(device->pUserData)->callback(dst, frameCount, static_cast<MalUserData*>(device->pUserData)->streamFormat);
}
