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
#include <ak/sound/Buffer.hpp>
#include <ak/sound/internal/MalUtil.hpp>
#include <ak/sound/Util.hpp>
#include <mini_al.h>

using namespace aks;

void aks::convertPCMSamples(void* sampleOut, Format formatOut, const void* sampleIn, Format formatIn, akSize sampleCount, DitherMode dither) {
	mal_pcm_convert(sampleOut, internal::toMalFormat(formatOut), sampleIn, internal::toMalFormat(formatIn), sampleCount, internal::toMalDitherMode(dither));
}

aks::Buffer aks::generateSineWave(akSize sampleRate, fpSingle frequency, Format format) {
	static constexpr float SINE_PERIOD = 2.f*akm::PI; // The base period of a sine wave
	const fpDouble sineWaveStep = (SINE_PERIOD*frequency)/sampleRate;

	std::vector<fpSingle> pcmData;
	pcmData.resize(static_cast<akSize>(sampleRate/frequency));
	for(akSize i = 0; i < pcmData.size(); i++) pcmData[i] = akm::sin(sineWaveStep * i);
	if (format == Format::FPSingle) return aks::Buffer(pcmData.data(), pcmData.size(), sampleRate, format, aks::ChannelMap::Mono1);

	std::vector<uint8> bufferData;
	bufferData.resize(pcmData.size() * aks::getFormatElementSize(format));
	aks::convertPCMSamples(bufferData.data(), format, pcmData.data(), Format::FPSingle, pcmData.size(), DitherMode::Trianglar);
	return aks::Buffer(bufferData.data(), bufferData.size()/aks::getFormatElementSize(format), sampleRate, format, aks::ChannelMap::Mono1);
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
