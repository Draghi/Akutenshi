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

#ifndef AK_ANIMATION_TYPE_HPP_
#define AK_ANIMATION_TYPE_HPP_

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>

namespace aka {

	// /////////////// //
	// // Anim Data // //
	// /////////////// //

	enum class TweenMode {
		None,
		Nearest,
		Linear,
		Repeat
	};

	struct AnimNode {
		std::string nodeName;
		std::map<fpSingle, akm::Quat> rotFrames;
		std::map<fpSingle, akm::Vec3> posFrames;
		std::map<fpSingle, akm::Vec3> sclFrames;
		TweenMode preAnim;
		TweenMode postAnim;
	};

	struct PoseNode {
		std::string nodeName;
		akm::Mat4 transform;
	};

	// //////////////// //
	// // Skele Data // //
	// //////////////// //

	struct BoneData {
		akm::Mat4 nodeMatrix;
		akm::Mat4 boneMatrix;
	};

	struct NamedBone {
		std::string parentName;
		std::string name;
		std::vector<std::string> childrenNames;

		BoneData data;
	};

	struct IndexedBone {
		std::string name;

		akSSize parentID;
		akSize id;
		std::vector<akSize> childrenIDs;

		BoneData data;
	};

	// //////////////////// //
	// // Mesh Pose Data // //
	// //////////////////// //

	struct PoseData {
		uint16 boneIndicies[4];
		akm::Vec4 boneWeights;
	};
	static_assert(sizeof(PoseData) == (4*sizeof(uint16) + 4*sizeof(fpSingle)), "PoseData size mismatch");

	// /////////////// //
	// // Mesh Data // //
	// /////////////// //

	struct IndexData {
		uint16 indicies[3];
	};

	struct VertexData {
		akm::Vec3 position;
		akm::Vec3 tangent;
		akm::Vec3 bitangent;
		akm::Vec3 normal;
		akm::Vec2 texCoord;
	};
	static_assert(sizeof(VertexData) == 14*sizeof(fpSingle), "VertexData size mismatch");

	using BoneWeights = std::unordered_map<std::string, fpSingle>;
}

#endif
