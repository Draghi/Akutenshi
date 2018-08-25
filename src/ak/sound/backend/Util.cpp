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

#include <ak/sound/backend/internal/MalUtil.hpp>
#include <ak/sound/backend/Util.hpp>
#include <ak/sound/Types.hpp>
#include <ak/util/Iterator.hpp>
#include <mini_al.h>
#include <cstring>
#include <algorithm>
#include <array>

using namespace aks;

akSize aks::backend::resampleStream(void* samplesOut, akSize frameCountOut, StreamFormat streamFormatOut, const void* samplesIn, akSize frameCountIn, StreamFormat streamFormatIn, DitherMode ditherMode) {
	if (streamFormatOut.channelMap == streamFormatIn.channelMap) {
		if ((streamFormatOut.format == streamFormatIn.format) && (streamFormatOut.sampleRate == streamFormatIn.sampleRate)) {
			std::memcpy(samplesOut, samplesIn, aks::backend::frameSizeOf(streamFormatOut)*std::min(frameCountOut, frameCountIn));
			return std::min(frameCountOut, frameCountIn);
		} else if (streamFormatOut.channelMap == ChannelMap::Mono) {
			mal_pcm_convert(samplesOut, aks::backend::internal::toMalFormat(streamFormatOut.format), samplesIn, aks::backend::internal::toMalFormat(streamFormatIn.format), std::min(frameCountOut, frameCountIn), aks::backend::internal::toMalDitherMode(ditherMode));
			return std::min(frameCountOut, frameCountIn);
		}
	}

	auto malChannelOut = aku::convert_to<std::vector<mal_channel>>(aks::backend::channelLayoutOf(streamFormatOut.channelMap), [](const auto& v) { return static_cast<mal_channel>(v); });
	auto malChannelIn  = aku::convert_to<std::vector<mal_channel>>(aks::backend::channelLayoutOf( streamFormatIn.channelMap), [](const auto& v) { return static_cast<mal_channel>(v); });

	return mal_convert_frames_ex_lim(
		samplesOut, aks::backend::internal::toMalFormat(streamFormatOut.format), malChannelOut.size(), streamFormatOut.sampleRate, malChannelOut.data(), frameCountOut,
		samplesIn,  aks::backend::internal::toMalFormat(streamFormatIn.format),  malChannelIn.size(),  streamFormatIn.sampleRate,  malChannelIn.data(),  frameCountIn,
		aks::backend::internal::toMalDitherMode(ditherMode), mal_src_algorithm_sinc
	);
}

akSize aks::backend::frameSizeOf(StreamFormat streamFormat) {
	return elementSizeOf(streamFormat.format)*channelLayoutOf(streamFormat.channelMap).size();
}

akSize aks::backend::elementSizeOf(Format format) {
	switch(format) {
		case Format::UInt8:    return 1;
		case Format::SInt16:   return 2;
		case Format::SInt24:   return 3;
		case Format::SInt32:   return 4;
		case Format::FPSingle: return 4;
	}
}

akSize aks::backend::calcResampledFrameCount(akSize sampleRateOld, akSize sampleRateNew, akSize frameCount) {
	return mal_calculate_frame_count_after_src(sampleRateNew, sampleRateOld, frameCount);
}


const std::vector<Channel>& aks::backend::channelLayoutOf(ChannelMap channelMap) {
	static const std::array<std::vector<Channel>, 16> ChannelLayouts {
		std::vector<Channel>({Channel::Mono}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::BackCenter}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackCenter}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackLeft, Channel::BackRight}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackLeft, Channel::BackRight, Channel::BackCenter}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackLeft, Channel::BackRight, Channel::SideLeft, Channel::SideRight}),

		std::vector<Channel>({Channel::Mono,      Channel::LFE}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::LFE}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::LFE}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::BackCenter,  Channel::LFE}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackCenter, Channel::LFE}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackLeft, Channel::BackRight, Channel::LFE}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackLeft, Channel::BackRight, Channel::BackCenter, Channel::LFE}),
		std::vector<Channel>({Channel::FrontLeft, Channel::FrontRight, Channel::FrontCenter, Channel::BackLeft, Channel::BackRight, Channel::SideLeft, Channel::SideRight, Channel::LFE}),
	};
	return ChannelLayouts[static_cast<uint8>(channelMap)];
}
