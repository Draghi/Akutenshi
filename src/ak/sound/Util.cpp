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

#include <ak/math/Scalar.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/Buffer.hpp>
#include <ak/sound/Enums.hpp>
#include <ak/sound/internal/MalUtil.hpp>
#include <ak/sound/Util.hpp>
#include <ak/util/Iterator.hpp>
#include <mini_al.h>
#include <cstring>
#include <array>
#include <vector>

using namespace aks;

bool aks::convertSamples(
	      void* samplesOut, Format formatOut, akSize sampleRateOut, ChannelMap channelMapOut,
	const void* samplesIn,  Format formatIn,  akSize sampleRateIn,  ChannelMap channelMapIn,
	akSize frameCount, DitherMode dither) {

	auto malChannelOut = aku::convert_to<std::vector<mal_channel>>(aks::getChannelLayoutFor(channelMapOut), [](const auto& v) { return static_cast<mal_channel>(v); });
	auto malChannelIn  = aku::convert_to<std::vector<mal_channel>>(aks::getChannelLayoutFor(channelMapIn),  [](const auto& v) { return static_cast<mal_channel>(v); });

	mal_convert_frames_ex(
		samplesOut, aks::internal::toMalFormat(formatOut), malChannelOut.size(), sampleRateOut, malChannelOut.data(),
		samplesIn,  aks::internal::toMalFormat(formatIn),  malChannelIn.size(),  sampleRateIn,  malChannelIn.data(),
		frameCount, aks::internal::toMalDitherMode(dither)
	);

	return true;
}

aks::Buffer aks::generateSineWave(akSize sampleRate, fpSingle frequency, Format format, DitherMode dither) {
	static constexpr float SINE_PERIOD = 2.f*akm::PI; // The base period of a sine wave
	const fpDouble sineWaveStep = (SINE_PERIOD*frequency)/sampleRate;

	std::vector<fpSingle> pcmData;
	pcmData.resize(static_cast<akSize>(sampleRate/frequency));
	for(akSize i = 0; i < pcmData.size(); i++) pcmData[i] = akm::sin(sineWaveStep * i);
	if (format == Format::FPSingle) return aks::Buffer(pcmData.data(), pcmData.size(), sampleRate, format, aks::ChannelMap::Mono);

	std::vector<uint8> bufferData;
	bufferData.resize(pcmData.size() * aks::getFormatElementSize(format));
	aks::convertSamples(bufferData.data(), format, sampleRate, ChannelMap::Mono, pcmData.data(), Format::FPSingle, sampleRate, ChannelMap::Mono, pcmData.size(), dither);
	return aks::Buffer(bufferData.data(), bufferData.size()/aks::getFormatElementSize(format), sampleRate, format, aks::ChannelMap::Mono);
}

akSize aks::getFormatElementSize(Format format) {
	switch(format) {
		case Format::UInt8:    return 1;
		case Format::SInt16:   return 2;
		case Format::SInt24:   return 3;
		case Format::SInt32:   return 4;
		case Format::FPSingle: return 4;
	}
}

const std::vector<Channel>& aks::getChannelLayoutFor(ChannelMap channelMap) {
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
