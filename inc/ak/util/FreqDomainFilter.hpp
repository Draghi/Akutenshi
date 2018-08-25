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

#ifndef AK_UTIL_FREQDOMAINFILTER_HPP_
#define AK_UTIL_FREQDOMAINFILTER_HPP_

#include <ak/math/FourierTransform.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/Sampler.hpp>
#include <ak/util/Bits.hpp>
#include <ak/util/Memory.hpp>
#include <algorithm>
#include <complex>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace aks {
	class FreqDomainFilter final : public Sampler {
		private:
			const Sampler* m_src;

			std::vector<fpSingle> m_filterR;
			std::vector<fpSingle> m_filterI;

			//akSize m_lastStart, m_lastCount;
			mutable akSSize m_lastStart;
			mutable std::vector<fpSingle> m_outputBuffer,  m_bufferR, m_bufferI;
			mutable bool m_isOutputReady;

			mutable akm::FTTBuffer m_fftBuffer;

		public:
			FreqDomainFilter() : m_src(nullptr), m_fftBuffer(1024) {}

			FreqDomainFilter(const Sampler& src, const Sampler& filter) : m_src(&src), m_filterR(), m_filterI(), m_lastStart(-1), m_outputBuffer(), m_isOutputReady(false), m_fftBuffer(aku::nearestPowerOfTwo(filter.sampleCount())) {
				std::vector<fpSingle> signalBuffer(m_fftBuffer.signalSize(), 0);
				filter.sample(signalBuffer.data(), 0, m_fftBuffer.signalSize());

				m_filterR.resize(m_fftBuffer.filterSize(), 0);
				m_filterI.resize(m_fftBuffer.filterSize(), 0);
				akm::fft(signalBuffer.data(), m_filterR.data(), m_filterI.data(), m_fftBuffer.signalSize(), m_fftBuffer);

				m_bufferR.resize(m_fftBuffer.filterSize(), 0);
				m_bufferI.resize(m_fftBuffer.filterSize(), 0);
				m_outputBuffer.resize(m_fftBuffer.signalSize(), 0);
			}

			FreqDomainFilter(const FreqDomainFilter&) = default;
			FreqDomainFilter& operator=(const FreqDomainFilter&) = default;

			akSize sample(fpSingle* out, akSSize start, akSize count) const override {
				if (!m_src) return 0;

				start -= (m_fftBuffer.signalSize() - 1)/2;
				akSize writtenSamples = 0;
				while(writtenSamples < count) {
					if (m_src->sample(m_outputBuffer.data(), start + writtenSamples, m_fftBuffer.signalSize()) == 0) return writtenSamples;

					akm::fft(m_outputBuffer.data(), m_bufferR.data(), m_bufferI.data(), m_fftBuffer.signalSize(), m_fftBuffer);

					for(akSize i = 0; i < m_fftBuffer.filterSize(); i++) {
						auto result = std::complex<fpSingle>(m_bufferR[i], m_bufferI[i]) * std::complex<fpSingle>(m_filterR[i], m_filterI[i]);
						m_bufferR[i] = result.real(); m_bufferI[i] = result.imag();
					}

					akm::ifft(m_outputBuffer.data(), m_bufferR.data(), m_bufferI.data(), m_fftBuffer.signalSize(), m_fftBuffer);

					m_lastStart = start + writtenSamples;
					writtenSamples += aku::memcpy(out + writtenSamples, m_outputBuffer.data(), std::min<akSize>(count - writtenSamples, m_outputBuffer.size()));
				}

				return writtenSamples;
			}

			void setSrc(const aks::Sampler& src) {
				m_src = &src;
				m_lastStart = -1;
			}

			akSize sampleCount() const override {
				return m_src->sampleCount();
			}

			bool loops() const override {
				return m_src->loops();
			}

			akSize signalSize() const {
				return m_fftBuffer.signalSize();
			}
	};
}

#endif /* AK_UTIL_FREQDOMAINFILTER_HPP_ */
