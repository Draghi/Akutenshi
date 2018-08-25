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
#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/Sampler.hpp>
#include <ak/util/Bits.hpp>
#include <ak/util/Memory.hpp>
#include <algorithm>
#include <complex>
#include <vector>

namespace aks {
	class FreqDomainFilter final : public Sampler {
		private:
			const Sampler* m_src;

			std::vector<fpSingle> m_filterR;
			std::vector<fpSingle> m_filterI;

			//akSize m_lastStart, m_lastCount;
			mutable std::vector<fpSingle> m_bufferR;
			mutable std::vector<fpSingle> m_bufferI;

			mutable akm::FTTBuffer m_fftBuffer;

			void freqDomainInplace(fpSingle* inputR, fpSingle* inputI, const fpSingle* filterR, const fpSingle* filterI, akSize count) const {
				for(akSize i = 0; i < count; i++) {
					auto result = std::complex<fpSingle>(inputR[i], inputI[i]) * std::complex<fpSingle>(filterR[i], filterI[i]);
					inputR[i] = result.real();
					inputI[i] = result.imag();
				}
			}

		public:
			FreqDomainFilter() : m_src(nullptr), m_fftBuffer(1024) {}

			FreqDomainFilter(const Sampler& src, const Sampler& filter) : m_src(&src), m_filterR(), m_filterI(), m_bufferR(), m_bufferI(), m_fftBuffer(aku::nearestPowerOfTwo(filter.sampleCount())) {
				m_bufferR.resize(akm::calcFFTOutputSize(m_fftBuffer.bufSize()), 0);
				m_bufferI.resize(akm::calcFFTOutputSize(m_fftBuffer.bufSize()), 0);

				std::vector<fpSingle> signalBuffer;
				signalBuffer.resize(m_fftBuffer.bufSize(), 0);
				filter.sample(signalBuffer.data(), 0, m_fftBuffer.bufSize());

				m_filterR.resize(akm::calcFFTOutputSize(m_fftBuffer.bufSize()), 0);
				m_filterI.resize(akm::calcFFTOutputSize(m_fftBuffer.bufSize()), 0);
				akm::fft(signalBuffer.data(), m_filterR.data(), m_filterI.data(), m_fftBuffer.bufSize(), m_fftBuffer);
			}

			FreqDomainFilter(const Sampler& src, const std::vector<fpSingle>& filterR, const std::vector<fpSingle>& filterI) : m_src(&src), m_filterR(filterR), m_filterI(filterI), m_bufferR(), m_bufferI(), m_fftBuffer(filterR.size()) {
				m_bufferR.resize(akm::calcFFTOutputSize(m_fftBuffer.bufSize()), 0);
				m_bufferI.resize(akm::calcFFTOutputSize(m_fftBuffer.bufSize()), 0);
				if (filterR.size() != filterI.size()) throw std::logic_error("Filter size mismatch.");
			}

			FreqDomainFilter(const FreqDomainFilter&) = default;
			FreqDomainFilter& operator=(const FreqDomainFilter&) = default;

			akSize sample(fpSingle* out, akSSize start, akSize count) const override {
				if (!m_src) return 0;

				akSize writtenSamples = 0;
				while(writtenSamples < count) {

					std::vector<fpSingle> signalBuffer;
					signalBuffer.resize(m_fftBuffer.bufSize(), 0);
					if (m_src->sample(signalBuffer.data(), start + writtenSamples, m_fftBuffer.bufSize()) == 0) return writtenSamples;

					fpSingle maxMag = 0.f;
					for(akSize i = 0; i < m_fftBuffer.bufSize(); i++) maxMag = akm::max(maxMag, akm::abs(signalBuffer[i]));

					akm::fft(signalBuffer.data(), m_bufferR.data(), m_bufferI.data(), m_fftBuffer.bufSize(), m_fftBuffer);
					freqDomainInplace(m_bufferR.data(), m_bufferI.data(), m_filterR.data(), m_filterI.data(), m_bufferR.size());
					akm::ifft(signalBuffer.data(), m_bufferR.data(), m_bufferI.data(), m_fftBuffer.bufSize(), m_fftBuffer);

					fpSingle maxMagNew = 0.f;
					for(akSize i = 0; i < m_fftBuffer.bufSize(); i++) maxMagNew = akm::max(maxMagNew, akm::abs(signalBuffer[i]));

					fpSingle scaleFactor = maxMag/maxMagNew;
					for(akSize i = 0; i < m_fftBuffer.bufSize(); i++) signalBuffer[i] *= scaleFactor;

					writtenSamples += aku::memcpy(out + writtenSamples, signalBuffer.data(), std::min<akSize>(count, signalBuffer.size()));
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

#endif /* AK_UTIL_FREQDOMAINFILTER_HPP_ */
