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
#include <ak/sound/backend/Backend.hpp>
#include <ak/sound/backend/Types.hpp>
#include <ak/sound/Util.hpp>
#include <ak/util/Memory.hpp>
#include <memory>
#include <vector>

using namespace aks;

aks::SamplerBuffer aks::generateSineWave(fpSingle frequency, fpSingle ampitude) {
	akSize sampleRate = backend::getDeviceInfo()->streamFormat.sampleRate;
	static constexpr float SINE_PERIOD = 2.f*akm::PI; // The base period of a sine wave
	const fpDouble sineWaveStep = (SINE_PERIOD*frequency)/sampleRate;

	std::vector<fpSingle> pcmData; pcmData.resize(static_cast<akSize>(sampleRate/frequency));
	for(akSize i = 0; i < pcmData.size(); i++) pcmData[i] = ampitude * akm::sin(sineWaveStep * i);

	return aks::SamplerBuffer(pcmData.data(), pcmData.size(), true);
}
