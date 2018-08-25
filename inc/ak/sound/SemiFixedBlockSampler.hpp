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

#ifndef AK_SOUND_SEMIFIXEDBLOCKSAMPLER_HPP_
#define AK_SOUND_SEMIFIXEDBLOCKSAMPLER_HPP_

#include <ak/math/Scalar.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/Sampler.hpp>
#include <ak/util/Memory.hpp>
#include <vector>

namespace aks {

	class SemiFixedBlockSampler final : public aks::Sampler {
		private:
			const Sampler* m_src;

			mutable akSSize m_lastStart, m_bufferSize;
			mutable std::vector<fpSingle> m_buffer;

			akSize resampleBuffer(akSize start) const {
				m_bufferSize = m_src->sample(m_buffer.data(), start, m_buffer.size());
				m_lastStart = start;
				return m_bufferSize;
			}

		public:
			SemiFixedBlockSampler(const Sampler& src, akSize size) : m_src(&src), m_lastStart(0), m_bufferSize(0), m_buffer(size, 0) {}

			SemiFixedBlockSampler(const SemiFixedBlockSampler&) = default;
			SemiFixedBlockSampler& operator=(const SemiFixedBlockSampler&) = default;

			akSize sample(fpSingle* out, akSSize start, akSize count) const override {
				if (!m_src) return 0;

				akSize writtenSamples = 0;

				/* Sample from internal buffer */ {
					akSSize offset = (start + writtenSamples) - m_lastStart;
					if ((offset >= 0) && (offset < m_bufferSize)) {
						writtenSamples += aku::memcpy(out + writtenSamples, m_buffer.data() + offset, akm::min(count - writtenSamples, m_bufferSize - offset));
						if (writtenSamples >= count) return writtenSamples;
					}
				}

				/* Sample in multiples of blocks */ {
					akSize quickReadSize = akm::floor((count - writtenSamples)/m_buffer.size()) * m_buffer.size();
					writtenSamples += m_src->sample(out + writtenSamples, start + writtenSamples, quickReadSize);
					if (writtenSamples >= count) return writtenSamples;
				}

				/* Sample into internal buffer */ {
					if (resampleBuffer(start + writtenSamples) == 0) return writtenSamples;
					akSSize offset = (start + writtenSamples) - m_lastStart;
					writtenSamples += aku::memcpy(out + writtenSamples, m_buffer.data() + offset, akm::min(count - writtenSamples, m_bufferSize - offset));
				}

				return writtenSamples;
			}

			void setSrc(const aks::Sampler& src) {
				m_src = &src;
			}

			akSize sampleCount() const override {
				return m_src->sampleCount();
			}

			bool loops() const override {
				return m_src->loops();
			}
	};

}

#endif /* AK_SOUND_SEMIFIXEDBLOCKSAMPLER_HPP_ */
