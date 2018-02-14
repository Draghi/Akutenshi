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

#include <ak/animation/Animation.hpp>
#include <ak/animation/AnimPoseMap.hpp>
#include <ak/animation/Fwd.hpp>
#include <ak/animation/Type.hpp>
#include <ak/math/Matrix.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/matrix.hpp>
#include <algorithm>
#include <stdexcept>
#include <deque>
#include <functional>
#include <iterator>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace aka {


	inline Skeleton::Skeleton() : m_rootID(-1), m_bones(), m_indexLookup(), m_finalTranformDirty(true), m_finalTransform() {}

	inline Skeleton::Skeleton(const std::vector<NamedBone>& bones) : m_rootID(-1), m_bones(), m_indexLookup(), m_finalTranformDirty(true), m_finalTransform() {
		// Create lookups
		std::vector<std::string> nameLookup;
		nameLookup.resize(bones.size());
		for(auto i = 0u; i < bones.size(); i++) {
			const std::string& name = bones[i].name;
			if (!m_indexLookup.insert(std::make_pair(name, i)).second) throw std::logic_error("aka::Skeleton: Found conflicting bone.");
			nameLookup[i] = name;
		}

		// Insert bone data
		m_bones.reserve(bones.size());
		for(auto i = 0u; i < bones.size(); i++) {
			auto& bone = bones[i];

			akSSize parentId = -1;
			if (bone.parentName.empty()) setRootID(i);
			else parentId = static_cast<akSSize>(findIDByName(bone.parentName));

			std::vector<akSize> childrenIDs;
			childrenIDs.reserve(bone.childrenNames.size());
			for(auto iter = bone.childrenNames.begin(); iter != bone.childrenNames.end(); iter++) childrenIDs.push_back(findIDByName(*iter)); //@todo Error handling for missing bones

			m_bones.push_back({bone.name, parentId, i, std::move(childrenIDs), bone.data});
		}

		// Validate
		if (m_rootID < 0) throw std::logic_error("aka::Skeleton: No root node provided for skeleton.");

		// @todo Add validation
		// - Ensure parent <-> child is 1-to-many
		// - Ensure all nodes are reachable from the root
		// - Ensure that there are no circular references
	}

	// /////////////// //
	// // Calculate // //
	// /////////////// //
	inline const std::vector<akm::Mat4> Skeleton::finalTransform() const {
		if (m_finalTranformDirty) recalculateBindPose();
		return m_finalTransform;
	}

	// ////////// //
	// // Pose // //
	// ////////// //
	inline akSize Skeleton::applyPose(const std::vector<PoseNode>& pose, bool lazyRecalc) {
		auto opCount = aka::applyPose(m_bones, pose, m_indexLookup);
		if (opCount) { m_finalTranformDirty = true; if (!lazyRecalc) recalculateBindPose(); }
		return opCount;
	}

	inline akSize Skeleton::applyPose(fpSingle time, const Animation& animation, bool lazyRecalc) {
		auto opCount = aka::applyPose(time, m_bones, animation, m_indexLookup);
		if (opCount) { m_finalTranformDirty = true; if (!lazyRecalc) recalculateBindPose(); }
		return opCount;
	}

	inline akSize Skeleton::applyPose(const std::vector<akm::Mat4>& pose, const std::vector<akSSize>& animMap, bool lazyRecalc) {
		auto opCount = aka::applyPose(m_bones, pose, animMap);
		if (opCount) { m_finalTranformDirty = true; if (!lazyRecalc) recalculateBindPose(); }
		return opCount;
	}

	inline akSize Skeleton::applyPose(fpSingle time, const Animation& animation, const AnimPoseMap& poseMap, bool lazyRecalc) {
		auto opCount = aka::applyPose(time, m_bones, animation, poseMap);
		if (opCount) { m_finalTranformDirty = true; if (!lazyRecalc) recalculateBindPose(); }
		return opCount;
	}

	// /////////// //
	// // Bones // //
	// /////////// //

	inline bool Skeleton::setBoneTransform(akSize id, const akm::Mat4& mat, bool lazyRecalc) {
		if (id >= m_bones.size()) return false;
		m_bones[id].data.boneMatrix = mat;
		m_finalTranformDirty = true;
		if (!lazyRecalc) recalculateBindPose();
		return true;
	}

	inline bool Skeleton::setNodeTransform(akSize id, const akm::Mat4& mat, bool lazyRecalc) {
		if (id >= m_bones.size()) return false;
		m_bones[id].data.nodeMatrix = mat;
		m_finalTranformDirty = true;
		if (lazyRecalc) recalculateBindPose();
		return true;
	}

	// //////////// //
	// // Lookup // //
	// //////////// //
	inline akSSize Skeleton::tryFindIDByName(const std::string& name) const {
		auto iter = m_indexLookup.find(name);
		if (iter == m_indexLookup.end()) return -1;
		return static_cast<akSSize>(iter->second);
	}

	inline akSize Skeleton::findIDByName(const std::string& name) const {
		auto iter = m_indexLookup.find(name);
		if (iter == m_indexLookup.end()) throw std::out_of_range("aka::Skeleton: Attempt to find name out of range");
		return iter->second;
	}

	inline const std::string& Skeleton::findNameByID(akSize id) const {
		if (id >= m_bones.size()) throw std::out_of_range("aka::Skeleton: Attempt to index out of range");
		return m_bones[id].name;
	}

	// //////////// //
	// // Access // //
	// //////////// //
	inline const std::vector<IndexedBone>& Skeleton::bones() const {
		return m_bones;
	}

	inline const std::unordered_map<std::string, akSize>& Skeleton::lookup() const {
		return m_indexLookup;
	}

	inline bool Skeleton::isDirty() const {
		return m_finalTranformDirty;
	}

	inline NamedBone Skeleton::getNamedBone(akSize id) const {
		if (id >= m_bones.size()) throw std::out_of_range("aka::Skeleton: Attempt to index out of range");

		std::vector<std::string> children;
		children.resize(m_bones[id].childrenIDs.size());
		for(auto i = 0u; i < m_bones[id].childrenIDs.size(); i++) children[i] = findNameByID(m_bones[id].childrenIDs[i]);

		return NamedBone{
			m_bones[id].parentID < 0 ? "" : findNameByID(static_cast<akSize>(m_bones[id].parentID)),
			findNameByID(id),
			std::move(children),
			m_bones[id].data
		};
	}

	inline const IndexedBone& Skeleton::getIndexedBone(akSize id) const {
		if (id >= m_bones.size()) throw std::out_of_range("aka::Skeleton: Attempt to index out of range");
		return m_bones[id];
	}

	inline akSize Skeleton::rootID() const {
		return static_cast<akSize>(m_rootID);
	}

	inline akSize Skeleton::boneCount() const {
		return static_cast<akSize>(m_bones.size());
	}

	// ///////////// //
	// // Utility // //
	// ///////////// //
	inline std::vector<akSize> Skeleton::getRootPath(akSize id) const {
		if (id >= m_bones.size()) throw std::out_of_range("aka::Skeleton: Attempt to index out of range");
		std::vector<akSize> result;

		auto* cBone = &m_bones[id];
		while(true) {
			result.push_back(cBone->id);
			if (cBone->parentID < 0) break;
			cBone = &m_bones[static_cast<akSize>(cBone->parentID)];
		}

		std::reverse(result.begin(), result.end());
		return result;
	}

	// ///////////// //
	// // Private // //
	// ///////////// //

	inline void Skeleton::setRootID(akSize newRootID) {
		if (m_rootID >= 0) throw std::logic_error("aka::Skeleton: Root bone already exists");
		m_rootID = static_cast<akSSize>(newRootID);
	}

	inline void Skeleton::recalculateBindPose() const {
		m_finalTransform = calculateFinalTransform(m_rootID, m_bones);
		m_finalTranformDirty = false;
	}

	// ///////////// //
	// // Methods // //
	// ///////////// //

	inline std::vector<akm::Mat4> calculateFinalTransform(akSize rootID, const std::vector<IndexedBone>& bones) {
		std::vector<akm::Mat4> result;
		result.resize(bones.size(), akm::Mat4(0));
		result[rootID] = akm::Mat4(1); // Parent of root is an identity matrix

		auto invRootTransform = akm::inverse(bones[rootID].data.nodeMatrix);

		std::deque<akSize> processStack;
		processStack.push_back(rootID);
		while(!processStack.empty()) {
			auto cID = processStack.back(); processStack.pop_back();
			auto& bone = bones[cID];

			auto globalTransform = result[cID] * bone.data.nodeMatrix;
			result[cID] = invRootTransform * globalTransform * bone.data.boneMatrix;
			// @todo Does the root matrix need to be transformed by the inverse of the root?
			// Since we use it in the shader, I think so. But we might be able to premultiply by not transforming the root node to an identity matrix.

			for(auto i = 0u; i < bone.childrenIDs.size(); i++) {
				result[bone.childrenIDs[i]] = globalTransform;
				processStack.push_back(bone.childrenIDs[i]);
			}
		}

		return result;
	}

	inline akSize applyPose(std::vector<IndexedBone>& bones, const std::vector<PoseNode>& pose, const std::unordered_map<std::string, akSize>& indexLookup) {
		akSize affectedBones = 0;
		for(auto& nodePose : pose) {
			auto iter = indexLookup.find(nodePose.nodeName);
			if (iter == indexLookup.end()) continue;
			bones[iter->second].data.nodeMatrix = nodePose.transform;
			affectedBones++;
		}
		return affectedBones;
	}

	inline akSize applyPose(fpSingle time, std::vector<IndexedBone>& bones, const Animation& animation, const std::unordered_map<std::string, akSize>& lookup) {
		return applyPose(bones, animation.calculatePose(time), lookup);
	}

	inline akSize applyPose(std::vector<IndexedBone>& bones, const std::vector<akm::Mat4>& pose, const std::vector<akSSize>& animMap) {
		akSize affectedBones = 0;
		for(auto i = 0u; i < animMap.size(); i++) {
			auto id = animMap[i];
			if (id < 0) continue;
			bones[static_cast<size_t>(id)].data.nodeMatrix = pose[i];
			affectedBones++;
		}
		return affectedBones;
	}

	inline akSize applyPose(fpSingle time, std::vector<IndexedBone>& bones, const Animation& animation, const AnimPoseMap& animMap) {
		return applyPose(bones, animation.calculateIndexedPose(time), animMap.mapping());
	}
}

#endif
