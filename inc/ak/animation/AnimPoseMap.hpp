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

#ifndef AK_ANIMATION_ANIMPOSEMAP_HPP_
#define AK_ANIMATION_ANIMPOSEMAP_HPP_

#include <vector>

#include <ak/animation/Animation.hpp>
#include <ak/animation/Fwd.hpp>
#include <ak/animation/Skeleton.hpp>
#include <ak/PrimitiveTypes.hpp>

namespace aka {
		inline AnimPoseMap::AnimPoseMap(const Skeleton& skeleton, const Animation& animation) {
			m_animMap.resize(animation.nodes().size());
			for(auto i = 0u; i < animation.nodes().size(); i++) {
				m_animMap[i] = skeleton.tryFindIDByName(animation.nodes()[i].nodeName);
			}
		}

		inline const std::vector<akSSize>& AnimPoseMap::mapping() const {
			return m_animMap;
		}
}

#endif



