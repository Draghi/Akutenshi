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

#ifndef AK_SOUND_HRTF_HRTFFILTERLOOKUP_HPP_
#define AK_SOUND_HRTF_HRTFFILTERLOOKUP_HPP_

#include <ak/math/Scalar.hpp>
#include <ak/math/Types.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/Sampler.hpp>
#include <algorithm>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace aks {
	class Sampler;
}

namespace aks {
	namespace hrtf {

		class HRTFFilterLookup final {
			private:
				std::vector<std::pair<akm::Vec3, std::unique_ptr<aks::Sampler>>> m_hrtfFilters;

			public:

				std::vector<std::pair<fpSingle, const Sampler*>> findClosestFiltersByDir(const akm::Vec3& dir, akSize count = 4) const {
					// Calculate shortest angle to all points (effectively distance)
					std::multimap<fpSingle, const Sampler*> sortedFilters;
					for(const auto& entry : m_hrtfFilters) {
						sortedFilters.emplace(akm::acos(akm::dot(dir, entry.first)), entry.second.get());
					}

					// Select nearest points
					fpSingle sum = 0.f;
					std::vector<std::pair<fpSingle, const Sampler*>> result; result.reserve(count);
					for(const auto& entry : sortedFilters) {
						sum += entry.first;
						result.push_back(entry);
						if (result.size() == count) break;
					}

					// Normalize
					for(auto& entry : result)  entry.first /= sum;

					return result;
				}

				std::vector<std::pair<fpSingle, const Sampler*>> findClosestFiltersByPos(const akm::Vec3& pos, akSize count = 4) const {
					return findClosestFiltersByDir(akm::normalize(pos), count);
				}

				std::vector<std::pair<fpSingle, const Sampler*>> findClosestFiltersByAngle(fpSingle azimuth, fpSingle inclination, akSize count = 4) const {
					return findClosestFiltersByPos(
						{
							akm::sin(inclination) * akm::cos(azimuth),
							akm::sin(inclination) * akm::sin(azimuth),
							akm::cos(inclination),
						},
						count
					);
				}

				bool addEntryByDir(std::unique_ptr<aks::Sampler> sampler, const akm::Vec3& dir) {
					m_hrtfFilters.push_back(std::pair<akm::Vec3, std::unique_ptr<aks::Sampler>>{dir, std::move(sampler)});
					return true;
				}

				bool addEntryByPos(std::unique_ptr<aks::Sampler> sampler, const akm::Vec3& pos) {
					return addEntryByDir(std::move(sampler), akm::normalize(pos));
				}

				bool addEntryByAngle(std::unique_ptr<aks::Sampler> sampler, fpSingle azimuth, fpSingle inclination) {
					return addEntryByPos(std::move(sampler), {
						akm::sin(inclination) * akm::cos(azimuth),
						akm::sin(inclination) * akm::sin(azimuth),
						akm::cos(inclination),
					});
				}

		};

	}

}


#endif /* AK_SOUND_HRTF_HRTFFILTERLOOKUP_HPP_ */
