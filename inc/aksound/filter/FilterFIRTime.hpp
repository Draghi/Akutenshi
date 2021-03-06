/**
 * Copyright 2018 Michael J. Baker
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

#ifndef AK_SOUND_FILTER_FILTERFIRTIME_HPP_
#define AK_SOUND_FILTER_FILTERFIRTIME_HPP_

#include <akcommon/Memory.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <aksound/backend/Backend.hpp>
#include <aksound/backend/Types.hpp>
#include <aksound/sampler/Sampler.hpp>
#include <vector>

namespace aks {

	class FilterFIRTime : public Sampler {
		private:
			const Sampler* m_sampler;
			std::vector<fpSingle> m_kernal;

		public:
			FilterFIRTime(const Sampler& sampler, const std::vector<fpSingle>& kernal) : m_sampler(&sampler), m_kernal(kernal) {}
			FilterFIRTime(const FilterFIRTime&) = default;
			FilterFIRTime& operator=(const FilterFIRTime&) = default;

			akSize sample(fpSingle* out, akSSize start, akSize count) const override {
				std::vector<fpSingle> buffer;
				buffer.resize(count + m_kernal.size(), 0.f);
				m_sampler->sample(buffer.data(), start - m_kernal.size()/2, buffer.size());
				akc::memset(out, 0.f, count);
				for(akSize i = 0; i < count; i++) {
					for(akSize j = 0; j < m_kernal.size(); j++) {
						out[i] += buffer[i + j] * m_kernal[j];
					}
				}
				return count;
			}

			akSize sampleCount() const override {
				return m_sampler->sampleCount();
			}

			bool loops() const override {
				return m_sampler->loops();
			}

			void setKernal(const std::vector<fpSingle>& kernal) {
				m_kernal = kernal;
			}

			const std::vector<fpSingle>& kernal() const {
				return m_kernal;
			}
	};

	inline std::vector<fpSingle> generateLowPassFilterAvg(fpSingle frequency) {
		akSize kernalSize = static_cast<akSize>(backend::getDeviceInfo()->streamFormat.sampleRate/frequency);
		return std::vector<fpSingle>(kernalSize, 1.f/kernalSize);
	}


}

#endif /* AKSOUND_BACKEND_FILTERFIRTIME_HPP_ */
