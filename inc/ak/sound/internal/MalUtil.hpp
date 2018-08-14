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

#ifndef AK_SOUND_INTERNAL_MALUTIL_HPP_
#define AK_SOUND_INTERNAL_MALUTIL_HPP_

#include <ak/sound/Enums.hpp>
#include <mini_al.h>
#include <stdexcept>
#include <vector>
#include <optional>

namespace aks {
	namespace internal {
		inline const std::vector<Backend> DEFAULT_BACKENDS = {
			Backend::WASAPI, Backend::DSound,    Backend::WinMM,      // Windows
			Backend::JACK,   Backend::ALSA,      Backend::PulseAudio, // Linux
			Backend::SndIO,  Backend::AudioIO,   Backend::OSS,        // BSD
			Backend::OpenAL, Backend::SDL,                            // Fallbacks
			Backend::OpenSL, Backend::CoreAudio, Backend::Null        // Other OSes
		};


		inline mal_backend toMalBackend(Backend backend){
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

		inline mal_dither_mode toMalDitherMode(DitherMode ditherMode) {
			switch(ditherMode) {
				case DitherMode::None:        return mal_dither_mode_none;
				case DitherMode::Rectangular: return mal_dither_mode_rectangle;
				case DitherMode::Trianglar:   return mal_dither_mode_triangle;
				default: throw std::invalid_argument("Unhandled dither mode type.");
			}
		}

		inline std::optional<aks::Format> fromMalFormat(mal_format v) {
			switch(v) {
				case mal_format::mal_format_u8:  return {aks::Format::UInt8};
				case mal_format::mal_format_s16: return {aks::Format::SInt16};
				case mal_format::mal_format_s24: return {aks::Format::SInt24};
				case mal_format::mal_format_s32: return {aks::Format::SInt32};
				case mal_format::mal_format_f32: return {aks::Format::FPSingle};
				case mal_format::mal_format_count:   [[fallthrough]];
				case mal_format::mal_format_unknown: [[fallthrough]];
				default: return {};
			}
		}

		inline mal_format toMalFormat(aks::Format v) {
			switch(v) {
				case aks::Format::UInt8:    return mal_format::mal_format_u8;
				case aks::Format::SInt16:   return mal_format::mal_format_s16;
				case aks::Format::SInt24:   return mal_format::mal_format_s24;
				case aks::Format::SInt32:   return mal_format::mal_format_s32;
				case aks::Format::FPSingle: return mal_format::mal_format_f32;
				default: throw std::invalid_argument("Unhandled audio format.");
			}
		}
	}
}

#endif /* AK_SOUND_INTERNAL_MALUTIL_HPP_ */
