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

#ifndef AK_ANIMATION_FWD_HPP_
#define AK_ANIMATION_FWD_HPP_

#include <ak/animation/Type.hpp>
#include <ak/math/Matrix.hpp>
#include <ak/math/Quaternion.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace aka {

	class Mesh;
	class Skeleton;
	class Animation;
	class AnimPoseMap;

	class Mesh final {
		private:
			std::vector<VertexData> m_vertexData;
			std::vector<BoneWeights> m_boneWeights;
			std::vector<IndexData> m_indexData;

		public:
			Mesh();
			Mesh(std::vector<VertexData>&& vertexData, std::vector<BoneWeights>&& boneWeights, std::vector<IndexData>&& indexData);
			Mesh(const std::vector<VertexData>& vertexData, const std::vector<BoneWeights>& boneWeights, const std::vector<IndexData>& indexData);

			std::vector<VertexData>&  vertexData();
			std::vector<BoneWeights>& boneWeights();
			std::vector<IndexData>&   indexData();

			const std::vector<VertexData>&  vertexData()  const;
			const std::vector<BoneWeights>& boneWeights() const;
			const std::vector<IndexData>&   indexData()   const;
	};


	inline std::vector<PoseData> createPoseData(const aka::Skeleton& skeleton, const aka::Mesh& mesh);

	class Skeleton final {
		private:
			akSSize m_rootID;
			std::vector<IndexedBone> m_bones;
			std::unordered_map<std::string, akSize> m_indexLookup;

			mutable bool m_finalTranformDirty;
			mutable std::vector<akm::Mat4> m_finalTransform;

			void setRootID(akSize newRootID);
			void recalculateBindPose() const;

		public:
			Skeleton();

			Skeleton(const std::vector<NamedBone>& bones);

			const std::vector<akm::Mat4> finalTransform() const;

			akSize applyPose(const std::vector<PoseNode>& pose, bool lazyRecalc = true);
			akSize applyPose(fpSingle time, const Animation& animation, bool lazyRecalc = true);

			akSize applyPose(const std::vector<akm::Mat4>& pose, const std::vector<akSSize>& animMap, bool lazyRecalc = true);
			akSize applyPose(fpSingle time, const Animation& animation, const AnimPoseMap& poseMap, bool lazyRecalc = true);

			bool setBoneTransform(akSize id, const akm::Mat4& mat, bool lazyRecalc = true);
			bool setNodeTransform(akSize id, const akm::Mat4& mat, bool lazyRecalc = true);

			akSSize tryFindIDByName(const std::string& name) const;
			akSize findIDByName(const std::string& name) const;
			const std::string& findNameByID(akSize id) const;

			const std::vector<IndexedBone>& bones() const;
			const std::unordered_map<std::string, akSize>& lookup() const;
			bool isDirty() const;

			NamedBone getNamedBone(akSize id) const;
			const IndexedBone& getIndexedBone(akSize id) const;

			akSize rootID() const;
			akSize boneCount() const;

			std::vector<akSize> getRootPath(akSize id) const;
	};

	inline std::vector<akm::Mat4> calculateFinalTransform(akSize rootID, const std::vector<IndexedBone>& bones);

	inline akSize applyPose(std::vector<IndexedBone>& bones, const std::vector<PoseNode>& pose, const std::unordered_map<std::string, akSize>& indexLookup);
	inline akSize applyPose(fpSingle time, std::vector<IndexedBone>& bones, const Animation& animation, const std::unordered_map<std::string, akSize>& lookup);

	inline akSize applyPose(std::vector<IndexedBone>& bones, const std::vector<akm::Mat4>& pose, const std::vector<akSSize>& animMap);
	inline akSize applyPose(fpSingle time, std::vector<IndexedBone>& bones, const Animation& animation, const AnimPoseMap& animMap);

	class Animation final {
		private:
			fpSingle m_ticksPerSecond;
			fpSingle m_duration;
			std::vector<AnimNode> m_animNodes;

			akm::Vec3 calculateFramePos(fpSingle animTime, const AnimNode& node) const;
			akm::Vec3 calculateFrameScl(fpSingle animTime, const AnimNode& node) const;
			akm::Quat calculateFrameRot(fpSingle animTime, const AnimNode& node) const;
			akm::Mat4 calculateFrame(const AnimNode& node, fpSingle time) const;

		public:
			Animation();
			Animation(fpSingle ticksPerSecond, fpSingle duration, std::vector<AnimNode>& animNodes);

			std::vector<PoseNode> calculatePose(fpSingle time) const;
			std::vector<akm::Mat4> calculateIndexedPose(fpSingle time) const;
			const std::vector<AnimNode>& nodes() const;

			fpSingle duration() const;
			fpSingle ticksPerSecond() const;
	};

	class AnimPoseMap final {
		private:
			std::vector<akSSize> m_animMap;

		public:
			AnimPoseMap(const Skeleton& skeleton, const Animation& animation);
			const std::vector<akSSize>& mapping() const;
	};
}

#endif
