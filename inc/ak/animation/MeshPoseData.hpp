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

#ifndef AK_ANIMATION_MESHPOSEDATA_HPP_
#define AK_ANIMATION_MESHPOSEDATA_HPP_

#include <ak/animation/Fwd.hpp>
#include <ak/animation/Skeleton.hpp>
#include <ak/animation/Type.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <string>
#include <utility>
#include <vector>

namespace aka {

	inline std::vector<PoseData> createPoseData(const aka::Skeleton& skeleton, const aka::Mesh& mesh) {
		std::vector<PoseData> result;
		result.resize(mesh.boneWeights().size());
		for(auto i = 0u; i < result.size(); i++) {
			// Restrict to 4 weights per bone
			std::vector<std::pair<std::string, fpSingle>> validWeights;
			for(auto iter = mesh.boneWeights()[i].begin(); iter != mesh.boneWeights()[i].end(); iter++) {
				if (validWeights.size() < 4) { validWeights.push_back(*iter); continue; }
				fpSingle maxDiff = 0;
				akSSize replaceID = -1;
				for(auto j = 0u; j < validWeights.size(); j++) {
					auto diff = iter->second - validWeights[j].second;
					if (maxDiff < diff) { maxDiff = diff; replaceID = static_cast<akSSize>(j); }
				}
				if (replaceID >= 0) validWeights[static_cast<akSize>(replaceID)] = *iter;
			}

			// Normalize Weights
			fpSingle totalWeight = 0;
			for(auto j = 0u; j < validWeights.size(); j++) totalWeight += validWeights[j].second;
			fpSingle multWeight = 1.0f/totalWeight;
			for(auto j = 0u; j < validWeights.size(); j++) validWeights[j].second *= multWeight;

			// Fetch bones
			aka::PoseData poseData = {{0,0,0,0},{0,0,0,0}};
			for(auto j = 0u; j < validWeights.size(); j++) {
				auto boneID = skeleton.findIDByName(validWeights[j].first);
				poseData.boneIndicies[j] = static_cast<uint16>(boneID);
				poseData.boneWeights[static_cast<akSSize>(j)] = validWeights[j].second;
			}
			result[i] = poseData;
		}

		return result;
	}

}

#endif
