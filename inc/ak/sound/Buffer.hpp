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

#ifndef AK_SOUND_BUFFER_HPP_
#define AK_SOUND_BUFFER_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/Sampler.hpp>
#include <ak/util/Memory.hpp>
#include <cstdlib>
#include <vector>

namespace aks {
	class Buffer final : public Sampler {
		private:
			std::vector<fpSingle> m_buffer;
			akSize m_sampleCount;
			bool m_loops;

		public:
			Buffer() : m_buffer(), m_sampleCount(0), m_loops(false) {}
			Buffer(fpSingle* src, akSize sampleCount, bool loops) : m_buffer(), m_sampleCount(sampleCount), m_loops(loops) {
				m_buffer.resize(sampleCount);
				aku::memcpy(m_buffer.data(), src, sampleCount);
			}

			Buffer(const Buffer&) = default;
			Buffer& operator=(const Buffer&) = default;

			akSize sample(fpSingle* out, akSSize start, akSize count) const override {
				if (count == 0) return 0;

				akSize copiedSamples = 0;

				if (!m_loops && (start < 0)) {
					aku::memset(out, 0.f, std::abs(start));
					copiedSamples += std::abs(start);
				} else while(start < 0) start += m_sampleCount;

				do {
					akSize curFrameStart = (start + copiedSamples) % m_sampleCount;
					akSize framesToCopy = (curFrameStart + (count - copiedSamples)) >= m_sampleCount ? (m_sampleCount - curFrameStart) : count - copiedSamples;
					aku::memcpy(out + copiedSamples, m_buffer.data() + curFrameStart, framesToCopy);
					copiedSamples += framesToCopy;
				} while(m_loops && (copiedSamples < count));
				return copiedSamples;
			}

			akSize sampleCount() const override { return m_sampleCount; }

			void setLoops(bool v) { m_loops = v; }
			bool loops() const override { return m_loops; }
	};
}

#endif /* AK_SOUND_BUFFER_HPP_ */
