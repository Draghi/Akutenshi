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

#ifndef AK_SOUND_MIXER_MIXERBASIC_HPP_
#define AK_SOUND_MIXER_MIXERBASIC_HPP_

#include <akcommon/Memory.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <akmath/Scalar.hpp>
#include <aksound/sampler/Sampler.hpp>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace aks {

	class MixerBasic final : public aks::Sampler {
		private:
			std::unordered_set<const Sampler*> m_sources;

			akSize m_maxSampleCount;
			akSize m_maxLoopSampleCount;
			bool m_loops;

			void updateSamplingInfo(const aks::Sampler& source) {
				m_maxSampleCount = std::max(m_maxSampleCount, source.sampleCount());
				if (source.loops()) {
					m_loops = true;
					m_maxLoopSampleCount = std::max(m_maxLoopSampleCount, source.sampleCount());
				}
				if (m_loops) m_maxSampleCount = static_cast<akSize>(akm::ceil(m_maxSampleCount/static_cast<fpSingle>(m_maxLoopSampleCount))*m_maxLoopSampleCount);
			}

		public:
			MixerBasic() : m_maxSampleCount(0) {}
			MixerBasic(const MixerBasic&) = default;
			MixerBasic& operator=(const MixerBasic&) = default;

			bool addSource(const Sampler& source) {
				if (!m_sources.emplace(&source).second) return false;
				updateSamplingInfo(source);
				return true;
			}

			bool removeSource(const Sampler& source) {
				if (!m_sources.erase(&source)) return false;
				m_maxSampleCount = 0; m_maxLoopSampleCount = 0; m_loops = false;
				for(const auto& entry : m_sources) updateSamplingInfo(*entry);
				return true;
			}

			akSize sample(fpSingle* out, akSSize start, akSize count) const override {
				aku::memset(out, 0.f, count);
				akSize maxSampleCount = 0;
				std::vector<fpSingle> buffer(count, 0);
				for(const auto& entry : m_sources) {
					akSize sampleCount = entry->sample(buffer.data(), start, count);
					maxSampleCount = std::max(maxSampleCount, sampleCount);
					for(akSize i = 0; i < sampleCount; i++) out[i] += buffer[i]; //out[i] = akm::clamp(out[i] + buffer[i], -1, 1);
				}
				return maxSampleCount;
			}

			akSize sampleCount() const override { return m_maxSampleCount; }
			bool loops() const override { return m_loops; }
	};

}

#endif /* AKSOUND_BACKEND_MIXERBASIC_HPP_ */
