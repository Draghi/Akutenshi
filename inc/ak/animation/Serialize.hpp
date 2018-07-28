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

#ifndef AK_ANIMATION_SERIALIZE_HPP_
#define AK_ANIMATION_SERIALIZE_HPP_

#include <ak/math/Serialize.hpp>

#include <cstring>
#include <deque>
#include <initializer_list>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ak/animation/Animation.hpp>
#include <ak/animation/Fwd.hpp>
#include <ak/animation/Mesh.hpp>
#include <ak/animation/Skeleton.hpp>
#include <ak/animation/Type.hpp>
#include <ak/PrimitiveTypes.hpp>

// Mesh
namespace akd {
	inline bool deserialize(aka::Mesh& dst, const akd::PValue& src) {
		std::vector<aka::VertexData> vertexData;
		{
			auto binData = src["vertexData"].getBin();
			if ((binData.size() % sizeof(aka::VertexData)) != 0) return false;
			vertexData.resize(binData.size()/sizeof(aka::VertexData));
			std::memcpy(vertexData.data(), binData.data(), binData.size());
		}

		std::vector<aka::BoneWeights> boneWeights;
		{
			auto& boneCfg = src["boneWeights"].getArr();
			boneWeights.resize(boneCfg.size());
			for(auto i = 0u; i < boneCfg.size(); i++) {
				auto& vWeightCfg = boneCfg[i].getArr();
				for(auto j = 0u; j < vWeightCfg.size(); j++) {
					boneWeights[i].emplace(
						vWeightCfg[j]["name"].getStr(),
						vWeightCfg[j]["weight"].getDec()
					);
				}
			}
		}

		std::vector<aka::IndexData> indexData;
		{
			auto binData = src["indexData"].getBin();
			if ((indexData.size() % sizeof(aka::IndexData)) != 0) return false;
			indexData.resize(binData.size()/sizeof(aka::IndexData));
			std::memcpy(indexData.data(), binData.data(), binData.size());
		}

		dst = aka::Mesh(vertexData, boneWeights, indexData);
		return true;
	}

	inline void serialize(akd::PValue& dst, const aka::Mesh& src) {
		// @todo Not portable across endianess... But it's way too slow otherwise...
		{
			std::vector<uint8> binData;
			binData.resize(src.vertexData().size()*sizeof(aka::VertexData));
			std::memcpy(binData.data(), src.vertexData().data(), binData.size());
			dst["vertexData"].setBin(binData);
		}

		{
			auto& boneCfg = dst["boneWeights"];
			for(auto i = 0u; i < src.boneWeights().size(); i++) {
				auto& vWeightCfg = boneCfg[i];
				auto j = 0u;
				for(auto iter = src.boneWeights()[i].begin(); iter != src.boneWeights()[i].end(); iter++, j++) {
					vWeightCfg[j]["name"].setStr(iter->first);
					vWeightCfg[j]["weight"].setDec(iter->second);
				}
			}
		}

		// @todo Not portable across endianess... But it's way too slow otherwise...
		{
			std::vector<uint8> binData;
			binData.resize(src.indexData().size()*sizeof(uint16[3]));
			std::memcpy(binData.data(), src.indexData().data(), binData.size());
			dst["indexData"].setBin(binData);
		}
	}
}

// Animation
namespace akd {
	inline void serialize(akd::PValue& dst, aka::TweenMode src) {
		switch(src) {
			case aka::TweenMode::None:    dst.setStr("None");    return;
			case aka::TweenMode::Nearest: dst.setStr("Nearest"); return;
			case aka::TweenMode::Linear:  dst.setStr("Linear");  return;
			case aka::TweenMode::Repeat:  dst.setStr("Repeat");  return;
		}
	}

	inline void serialize(akd::PValue& dst, const aka::AnimNode& src) {
		dst["nodeName"].setStr(src.nodeName);

		akSize i = 0;
		for(auto& iter : src.rotFrames) {
			dst["rotFrames"][i][0].setDec(iter.first);
			akd::serialize(dst["rotFrames"][i][1], iter.second);
			i++;
		}

		i = 0;
		for(auto& iter : src.posFrames) {
			dst["posFrames"][i][0].setDec(iter.first);
			akd::serialize(dst["posFrames"][i][1], iter.second);
			i++;
		}

		i = 0;
		for(auto& iter : src.sclFrames) {
			dst["sclFrames"][i][0].setDec(iter.first);
			akd::serialize(dst["sclFrames"][i][1], iter.second);
			i++;
		}

		serialize(dst["preAnim"], src.preAnim);
		serialize(dst["postAnim"], src.postAnim);
	}

	inline void serialize(akd::PValue& dst, const aka::Animation& src) {
		dst["ticksPerSecond"].set(src.ticksPerSecond());
		dst["duration"].set(src.duration());
		for(auto i = 0u; i < src.nodes().size(); i++) serialize(dst["nodes"][i], src.nodes()[i]);
	}

	inline bool deserialize(aka::TweenMode& dst, const akd::PValue& src) {
		auto enumStr = src.getStr();
		if      (enumStr == "None"   ) dst = aka::TweenMode::None;
		else if (enumStr == "Nearest") dst = aka::TweenMode::Nearest;
		else if (enumStr == "Linear" ) dst = aka::TweenMode::Linear;
		else if (enumStr == "Repeat" ) dst = aka::TweenMode::Repeat;
		else return false;

		return true;
	}

	inline bool deserialize(aka::AnimNode& dst, const akd::PValue& src) {
		std::string nodeName = src["nodeName"].getStr();

		auto& rotFrameArr = src["rotFrames"].getArr();
		std::map<fpSingle, akm::Quat> rotFrames;
		for(auto i = 0u; i < rotFrameArr.size(); i++) {
			akm::Quat quat;
			if (!deserialize(quat, rotFrameArr[i][1])) throw std::runtime_error("Quat failed");
			rotFrames.emplace(rotFrameArr[i][0].as<fpSingle>(), quat);
		}

		auto& posFrameArr = src["posFrames"].getArr();
		std::map<fpSingle, akm::Vec3> posFrames;
		for(auto i = 0u; i < posFrameArr.size(); i++) {
			akm::Vec3 vec;
			if (!deserialize(vec, posFrameArr[i][1])) throw std::runtime_error("Pos failed");
			posFrames.emplace(posFrameArr[i][0].as<fpSingle>(), vec);
		}

		auto& sclFrameArr = src["sclFrames"].getArr();
		std::map<fpSingle, akm::Vec3> sclFrames;
		for(auto i = 0u; i < sclFrameArr.size(); i++) {
			akm::Vec3 vec;
			if (!deserialize(vec, sclFrameArr[i][1])) throw std::runtime_error("Scl failed");
			sclFrames.emplace(sclFrameArr[i][0].as<fpSingle>(), vec);
		}

		aka::TweenMode preTween, postTween;
		if (!deserialize(preTween,  src["preAnim"]))  return false;
		if (!deserialize(postTween, src["postAnim"])) return false;

		dst = aka::AnimNode{
			nodeName,
			rotFrames, posFrames, sclFrames,
			preTween, postTween
		};
		return true;
	}

	inline bool deserialize(aka::Animation& dst, const akd::PValue& src) {
		auto& nodeArr = src["nodes"].getArr();
		std::vector<aka::AnimNode> animNodes;
		animNodes.resize(nodeArr.size());
		for(auto i = 0u; i < nodeArr.size(); i++) deserialize(animNodes[i], nodeArr[i]);

		dst = aka::Animation(src["ticksPerSecond"].as<fpSingle>(), src["duration"].as<fpSingle>(), animNodes);

		return true;
	}

}

// Skeleton
namespace akd {
	inline void serialize(akd::PValue& dst, const aka::NamedBone& src) {
		dst["parentName"].setStr(src.parentName);
		dst["name"].setStr(src.name);

		auto& childrenNames = dst["childrenNames"].setArr().getArr();
		childrenNames.resize(src.childrenNames.size());
		for(auto i = 0u; i < src.childrenNames.size(); i++) childrenNames[i] = akd::PValue::from(src.childrenNames[i]);

		auto& data = dst["data"];
		akd::serialize(data["nodeMatrix"], src.data.nodeMatrix);
		akd::serialize(data["boneMatrix"], src.data.boneMatrix);
	}

	inline bool deserialize(aka::NamedBone& dst, const akd::PValue& src) {
		try {
			aka::NamedBone result;

			result.parentName = src["parentName"].getStr();
			result.name = src["name"].getStr();

			auto& childrenNames = src["childrenNames"].getArr();
			result.childrenNames.clear(); dst.childrenNames.reserve(childrenNames.size());
			for(auto i = 0u; i < childrenNames.size(); i++) result.childrenNames.push_back(childrenNames[i].getStr());

			auto& data = src["data"];
			if (!akd::deserialize(result.data.nodeMatrix, data["nodeMatrix"])) return false;
			if (!akd::deserialize(result.data.boneMatrix, data["boneMatrix"])) return false;

			dst = result;
		} catch(const std::logic_error&) {
			return false;
		}
		return true;
	}

	inline void serialize(akd::PValue& dst, const aka::Skeleton& src) {
		for(auto i = 0u; i < src.boneCount(); i++) serialize(dst[i], src.getNamedBone(i));
	}

	inline bool deserialize(aka::Skeleton& dst, const akd::PValue& src) {
		if (!src.isArr()) return false;
		auto boneArr = src.getArr();

		std::vector<aka::NamedBone> bones;
		bones.resize(boneArr.size());
		for(auto i = 0u; i < boneArr.size(); i++) {
			if (!akd::deserialize(bones[i], boneArr[i])) return false;
		}

		dst = aka::Skeleton(bones);

		return true;
	}
}

#endif
