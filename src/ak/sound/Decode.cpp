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

#include <ak/sound/backend/Backend.hpp>
#include <ak/sound/backend/internal/MalUtil.hpp>
#include <ak/sound/Decode.hpp>
#include <ak/ScopeGuard.hpp>
#include <mini_al.h>
#include <memory>
#include <utility>

using namespace aks;

static std::unordered_map<Channel, aks::SamplerBuffer> decode(const std::vector<uint8>& data, mal_format format, akSize sampleRate, bool shouldLoop, mal_dither_mode ditherMode) {

	// Configure
	mal_decoder_config cfg = {
		format,                                //mal_format format;      // Set to 0 or mal_format_unknown to use the stream's internal format.
		0,                                     //mal_uint32 channels;    // Set to 0 to use the stream's internal channels.
		sampleRate,                            //mal_uint32 sampleRate;  // Set to 0 to use the stream's internal sample rate.
		{0},                                   //mal_channel channelMap[MAL_MAX_CHANNELS];
		mal_channel_mix_mode_planar_blend,     //mal_channel_mix_mode channelMixMode;
		ditherMode,                            //mal_dither_mode ditherMode;
		mal_src_algorithm_sinc,                //mal_src_algorithm srcAlgorithm;
		{mal_src_sinc_window_function_hann, 0} //union { mal_src_config_sinc sinc; } src;
	};

	// Decode
	mal_uint64 frameCount;
	void* bufferData = nullptr;
	auto bufferDataGuard = ak::ScopeGuard([&]{if (bufferData != nullptr) mal_free(bufferData); });
	if (mal_decode_memory(data.data(), data.size(), &cfg, &frameCount, &bufferData) != MAL_SUCCESS) {
		return std::unordered_map<Channel, aks::SamplerBuffer>();
	}

	// Deinterleave
	std::unordered_map<mal_channel, std::vector<fpSingle>> samples;
	for(akSize j = 0; j < cfg.channels; j++) samples[cfg.channelMap[j]].resize(frameCount);
	for(akSize i = 0; i < frameCount; i++) {
		for(akSize j = 0; j < cfg.channels; j++) {
			samples[cfg.channelMap[j]][i] = static_cast<fpSingle*>(bufferData)[i*cfg.channels + j];
		}
	}

	// Convert
	std::unordered_map<Channel, aks::SamplerBuffer> result;
	for(auto channel : samples) result[static_cast<Channel>(channel.first)] = aks::SamplerBuffer(channel.second.data(), channel.second.size(), shouldLoop);
	return result;
}

std::unordered_map<Channel, aks::SamplerBuffer> aks::decode(const std::vector<uint8>& data, bool shouldLoop, backend::DitherMode ditherMode) {
	return decode(data, mal_format_f32, backend::getDeviceInfo()->streamFormat.sampleRate, shouldLoop, aks::backend::internal::toMalDitherMode(ditherMode));
}



