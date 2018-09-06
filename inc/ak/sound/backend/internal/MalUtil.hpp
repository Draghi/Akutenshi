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

#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/backend/Types.hpp>
#include <ak/sound/backend/Util.hpp>
#include <mini_al.h>
#include <optional>
#include <stdexcept>
#include <vector>

namespace aks {
	namespace backend {
		namespace internal {
			inline const std::vector<Backend> DEFAULT_BACKENDS = {
				Backend::WASAPI, Backend::DSound,     Backend::WinMM,   // Windows
				Backend::ALSA,   Backend::PulseAudio, Backend::JACK,    // Linux
				Backend::SndIO,  Backend::AudioIO,    Backend::OSS,     // BSD
				Backend::OpenAL, Backend::SDL,                          // Fallbacks
				Backend::OpenSL, Backend::CoreAudio,  Backend::Null     // Other OSes
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

			inline std::optional<Format> fromMalFormat(mal_format v) {
				switch(v) {
					case mal_format::mal_format_u8:  return {Format::UInt8};
					case mal_format::mal_format_s16: return {Format::SInt16};
					case mal_format::mal_format_s24: return {Format::SInt24};
					case mal_format::mal_format_s32: return {Format::SInt32};
					case mal_format::mal_format_f32: return {Format::FPSingle};
					case mal_format::mal_format_count:   [[fallthrough]];
					case mal_format::mal_format_unknown: [[fallthrough]];
					default: return {};
				}
			}

			inline mal_format toMalFormat(Format v) {
				switch(v) {
					case Format::UInt8:    return mal_format::mal_format_u8;
					case Format::SInt16:   return mal_format::mal_format_s16;
					case Format::SInt24:   return mal_format::mal_format_s24;
					case Format::SInt32:   return mal_format::mal_format_s32;
					case Format::FPSingle: return mal_format::mal_format_f32;
					default: throw std::invalid_argument("Unhandled audio format.");
				}
			}

			inline std::optional<ChannelMap> fromMalChannels(const mal_channel* channels, akSize channelCount) {
				static auto compareChannels = [&](ChannelMap channelMap){
					const auto& map = channelLayoutOf(channelMap);
					if (map.size() != channelCount) return false;
					for(akSize i = 0; i < map.size(); i++) if (static_cast<uint8>(map[i]) != channels[i]) return false;
					return true;
				};
				if (compareChannels(ChannelMap::Mono     )) return ChannelMap::Mono;
				if (compareChannels(ChannelMap::Stereo   )) return ChannelMap::Stereo;
				if (compareChannels(ChannelMap::Stereo3  )) return ChannelMap::Stereo3;
				if (compareChannels(ChannelMap::Surround3)) return ChannelMap::Surround3;
				if (compareChannels(ChannelMap::Surround4)) return ChannelMap::Surround4;
				if (compareChannels(ChannelMap::Surround5)) return ChannelMap::Surround5;
				if (compareChannels(ChannelMap::Surround6)) return ChannelMap::Surround6;
				if (compareChannels(ChannelMap::Surround7)) return ChannelMap::Surround7;

				if (compareChannels(ChannelMap::Mono_LF     )) return ChannelMap::Mono_LF;
				if (compareChannels(ChannelMap::Stereo_LF   )) return ChannelMap::Stereo_LF;
				if (compareChannels(ChannelMap::Stereo3_LF  )) return ChannelMap::Stereo3_LF;
				if (compareChannels(ChannelMap::Surround3_LF)) return ChannelMap::Surround3_LF;
				if (compareChannels(ChannelMap::Surround4_LF)) return ChannelMap::Surround4_LF;
				if (compareChannels(ChannelMap::Surround5_LF)) return ChannelMap::Surround5_LF;
				if (compareChannels(ChannelMap::Surround6_LF)) return ChannelMap::Surround6_LF;
				if (compareChannels(ChannelMap::Surround7_LF)) return ChannelMap::Surround7_LF;

				return {};
			}
		}
	}
}

#endif /* AK_SOUND_INTERNAL_MALUTIL_HPP_ */
