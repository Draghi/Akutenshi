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

#ifndef AK_SOUND_UTIL_HPP_
#define AK_SOUND_UTIL_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/SamplerBuffer.hpp>

namespace aks {
	aks::SamplerBuffer generateSineWave(fpSingle frequency, fpSingle ampitude);

	inline aks::SamplerBuffer generateWindowedSinc(fpSingle frequency, akSize sampleRate) {
		const fpSingle freqStep = (2*akm::PI*frequency)/sampleRate;

		akSize count = aku::nearestPowerOfTwo(static_cast<akSize>(akm::ceil(4/(frequency/sampleRate))*400));

		std::vector<fpSingle> result;
		result.resize(count, 0);

		fpSingle maxVal = 0;

		fpSingle sum = 0;

		fpSingle countDiv = count - 1;
		for(akSize i = 0; i < count; i++) {
			result[i] = akm::sinc(freqStep*(i-(countDiv/2.f))) * (0.42 - 0.5  * akm::cos((2*akm::PI*i)/countDiv) + 0.08 * akm::cos((4*akm::PI*i)/countDiv));
			maxVal = akm::max(result[i], maxVal);
			sum += result[i];
		}

		for(akSize i = 0; i < count; i++) result[i] /= sum;

		return aks::SamplerBuffer(result.data(), result.size(), true);
	}
}

#endif /* AK_SOUND_UTIL_HPP_ */
