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

#ifndef AK_SOUND_SINE_HPP_
#define AK_SOUND_SINE_HPP_

#include <ak/math/Scalar.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/Backend.hpp>
#include <ak/sound/Buffer.hpp>
#include <vector>

namespace aks {
	inline aks::Buffer generateSineWave(akSize sampleRate, fpSingle frequency, Format format) {
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

}



#endif /* AK_SOUND_SINE_HPP_ */
