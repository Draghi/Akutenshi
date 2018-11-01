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

#ifndef AK_SOUND_SAMPLER_SAMPLERBUFFER_HPP_
#define AK_SOUND_SAMPLER_SAMPLERBUFFER_HPP_

#include <akcommon/Memory.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <aksound/sampler/Sampler.hpp>
#include <cmath>
#include <vector>

namespace aks {
	class SamplerBuffer final : public Sampler {
		private:
			std::vector<fpSingle> m_buffer;
			akSize m_sampleCount;
			bool m_loops;

		public:
			SamplerBuffer() : m_buffer(), m_sampleCount(0), m_loops(false) {}
			SamplerBuffer(fpSingle* src, akSize sampleCount, bool loops) : m_buffer(), m_sampleCount(sampleCount), m_loops(loops) {
				m_buffer.resize(sampleCount);
				akc::memcpy(m_buffer.data(), src, sampleCount);
			}

			SamplerBuffer(const SamplerBuffer&) = default;
			SamplerBuffer& operator=(const SamplerBuffer&) = default;

			akSize sample(fpSingle* out, akSSize start, akSize count) const override {
				if (count == 0) return 0;

				akSize copiedSamples = 0;

				if (!m_loops && (start < 0)) {
					akc::memset(out, 0.f, std::abs(start));
					copiedSamples += std::abs(start);
				} else while(start < 0) start += m_sampleCount;

				do {
					akSize curFrameStart = (start + copiedSamples) % m_sampleCount;
					akSize framesToCopy = (curFrameStart + (count - copiedSamples)) >= m_sampleCount ? (m_sampleCount - curFrameStart) : count - copiedSamples;
					akc::memcpy(out + copiedSamples, m_buffer.data() + curFrameStart, framesToCopy);
					copiedSamples += framesToCopy;
				} while(m_loops && (copiedSamples < count));
				return copiedSamples;
			}

			akSize sampleCount() const override { return m_sampleCount; }

			void setLoops(bool v) { m_loops = v; }
			bool loops() const override { return m_loops; }
	};
}

#endif /* AKSOUND_BACKEND_SAMPLERBUFFER_HPP_ */
