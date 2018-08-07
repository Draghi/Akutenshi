/**
 * Copyright 2018 Michael J. Baker
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

#include <ak/sound/Context.hpp>

#include <algorithm>
#include <atomic>
#include <stdexcept>
#include <mini_al.h>

#include <ak/Log.hpp>
#include <ak/util/String.hpp>
#include <ak/ScopeGuard.hpp>

using namespace aks;

static const std::vector<Backend> DEFAULT_BACKENDS = {
	Backend::WASAPI, Backend::DSound,  Backend::WinMM,  Backend::CoreAudio,
	Backend::SndIO,  Backend::AudioIO, Backend::OSS,    Backend::PulseAudio,
	Backend::ALSA,   Backend::JACK,    Backend::OpenSL, Backend::OpenAL,
	Backend::SDL,    Backend::Null
};

static std::atomic<bool> malIsInit = false;
static mal_context malContext;

static void recvMalMessage(mal_context* /*pContext*/, mal_device* /*pDevice*/, const char* message) {
	akl::Logger("MAL").info(AK_STRING_VIEW(message));
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

bool aks::init() {
	return aks::init(DEFAULT_BACKENDS);
}

bool aks::init(const std::vector<Backend>& backends) {
	if(malIsInit.exchange(true)) return false;

	std::vector<mal_backend> malBackends; malBackends.reserve(backends.size());
	std::for_each(backends.begin(), backends.end(), [&](Backend backend){ malBackends.push_back(toMalBackend(backend)); });

	mal_context_config malConfig = {
		&recvMalMessage,
		mal_thread_priority::mal_thread_priority_default,
		{ MAL_TRUE },
		{ nullptr, nullptr, MAL_TRUE },
		{ nullptr, MAL_TRUE }
	};

	if (mal_context_init(malBackends.data(), malBackends.size(), &malConfig, &malContext) != MAL_SUCCESS) { malIsInit = false; return false; }
	ak::ScopeGuard releaseContext([]{mal_context_uninit(&malContext);});

	return true;
}

void* aks::internal::getContext() {
	return &malContext;
}

std::vector<std::pair<std::string>> aks::getDeviceNames() {
	static auto processDevices = [](mal_context* pContext, mal_device_type type, const mal_device_info* pInfo, void* pUserData) -> mal_uint32 {
		if (type != mal_device_type::mal_device_type_playback) return MAL_TRUE;
		auto& result = *static_cast<std::vector<std::string>*>(pUserData);
		result.push_back(pInfo->formats);
		return MAL_TRUE;
	};
	mal_context_get_devices(mal_context* pContext, mal_device_info** ppPlaybackDeviceInfos, mal_uint32* pPlaybackDeviceCount, mal_device_info** ppCaptureDeviceInfos, mal_uint32* pCaptureDeviceCount);
	std::vector<std::string> result;
	mal_context_enumerate_devices(&malContext, +processDevices, &result);
	return result;
}
