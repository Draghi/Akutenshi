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

#include <ak/sound/Buffer.hpp>
#include <ak/sound/Decode.hpp>
#include <ak/sound/Enums.hpp>
#include <ak/sound/internal/MalUtil.hpp>
#include <ak/sound/Util.hpp>
#include <ak/util/Iterator.hpp>
#include <ak/ScopeGuard.hpp>
#include <mini_al.h>
#include <stdexcept>
#include <vector>

using namespace aks;

std::optional<aks::Buffer> aks::decode(const std::vector<uint8>& data, Format format, ChannelMap channelMap, akSize sampleRate, DitherMode ditherMode) {
	mal_uint64 frameCount;
	void* bufferData = nullptr;
	auto bufferDataGuard = ak::ScopeGuard([&]{if (bufferData != nullptr) mal_free(bufferData); });

	mal_decoder_config cfg = {
		aks::internal::toMalFormat(format),          //mal_format format;      // Set to 0 or mal_format_unknown to use the stream's internal format.
		aks::getChannelLayoutFor(channelMap).size(), //mal_uint32 channels;    // Set to 0 to use the stream's internal channels.
		sampleRate,                                  //mal_uint32 sampleRate;  // Set to 0 to use the stream's internal sample rate.
		{0},                                         //mal_channel channelMap[MAL_MAX_CHANNELS];
		mal_channel_mix_mode_planar_blend,           //mal_channel_mix_mode channelMixMode;
		aks::internal::toMalDitherMode(ditherMode),  //mal_dither_mode ditherMode;
		mal_src_algorithm_default,                   //mal_src_algorithm srcAlgorithm;
		{mal_src_sinc_window_function_hann, 0}       //union { mal_src_config_sinc sinc; } src;
	};
	for(akSize i = 0; i < aks::getChannelLayoutFor(channelMap).size(); i++) cfg.channelMap[i] = static_cast<uint8>(aks::getChannelLayoutFor(channelMap)[i]);

	auto result = mal_decode_memory(data.data(), data.size(), &cfg, &frameCount, &bufferData);
	if (result != MAL_SUCCESS) return {};

	if (internal::toMalFormat(format) != cfg.format) throw std::logic_error("Decode failure. Format.");
	if (aks::getChannelLayoutFor(channelMap).size() != cfg.channels) throw std::logic_error("Decode failure. Channels.");
	if (sampleRate != cfg.sampleRate) throw std::logic_error("Decode failure. SampleRate.");

	return {Buffer(bufferData, frameCount, sampleRate, format, channelMap)};
}


