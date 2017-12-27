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

#ifndef AK_ANIMATION_SKELETON_HPP_
#define AK_ANIMATION_SKELETON_HPP_

#include <ak/math/Matrix.hpp>
#include <ak/math/Transform.hpp>
#include <stddef.h>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace aka {

	struct Bone {
		std::string parent;
		akm::Transform3D transform;
	};

	class Skeleton {
		private:
			std::vector<Bone> m_bones;
			std::unordered_map<std::string, size_t> m_boneMapping;

		public:
			Skeleton() {}
			Skeleton(const std::vector<Bone>& bones, const std::unordered_map<std::string, size_t>& boneMapping) : m_bones(bones), m_boneMapping(boneMapping) {}
			Skeleton(std::vector<Bone>&& bones, std::unordered_map<std::string, size_t>&& boneMapping) : m_bones(bones), m_boneMapping(boneMapping) {}

			Skeleton(const Skeleton& other) : m_bones(other.m_bones), m_boneMapping(other.m_boneMapping) {}
			Skeleton(Skeleton&& other) : m_bones(std::move(other.m_bones)), m_boneMapping(other.m_boneMapping) {}

			akm::Mat4 resolveTransform(const std::string& boneName) const {
				akm::Mat4 transform(1);

				std::unordered_set<std::string> visted;
				std::string nextBone = boneName;
				while(true) {
					auto iter = m_boneMapping.find(boneName);
					if (iter == m_boneMapping.end()) return transform;

					if (!visted.insert(nextBone).second) throw std::runtime_error("Skeleton::resolveTransform: Transform loop encountered");

					transform *= m_bones[iter->second].transform.composeModelMatrix();
					nextBone = m_bones[iter->second].parent;
				}
			}

			const std::vector<Bone>& bones() const { return m_bones; }
			const std::unordered_map<std::string, size_t>& boneMapping() const { return m_boneMapping; }

			Skeleton& operator=(const Skeleton& other) { m_bones = other.m_bones; m_boneMapping = other.m_boneMapping; return *this; }
			Skeleton& operator=(Skeleton&& other) { m_bones = std::move(other.m_bones); m_boneMapping = std::move(other.m_boneMapping); return *this; }
	};

}

#endif
