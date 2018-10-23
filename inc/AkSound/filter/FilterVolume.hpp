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

#ifndef AKSOUND_BACKEND_FILTERVOLUME_HPP_
#define AKSOUND_BACKEND_FILTERVOLUME_HPP_

#include <AkCommon/PrimitiveTypes.hpp>
#include <AkSound/sampler/Sampler.hpp>

namespace aks {

	class FilterVolume final : public aks::Sampler {
		private:
			const aks::Sampler* m_sampler;
			fpSingle m_volume;
		public:
			FilterVolume() : m_sampler(nullptr), m_volume(1) {}
			FilterVolume(const aks::Sampler& sampler, fpSingle volume) : m_sampler(&sampler), m_volume(volume) {}
			FilterVolume(const FilterVolume&) = default;
			FilterVolume& operator=(const FilterVolume&) = default;

			akSize sample(fpSingle* out, akSSize start, akSize count) const override {
				auto sampleCount = m_sampler->sample(out, start, count);
				for(akSize i = 0; i < sampleCount; i++) out[i] *= m_volume;
				return sampleCount;
			}

			akSize sampleCount() const override { return m_sampler->sampleCount(); }
			bool loops() const override { return m_sampler->loops(); }
	};

}

#endif /* AKSOUND_BACKEND_FILTERVOLUME_HPP_ */
