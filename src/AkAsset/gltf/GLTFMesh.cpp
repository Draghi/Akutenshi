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

#include <AkAsset/gltf/GLTFMesh.hpp>
#include <AkAsset/Convert.hpp>
#include <AkAsset/gltf/Accessor.hpp>
#include <AkAsset/gltf/Asset.hpp>
#include <AkAsset/gltf/Mesh.hpp>
#include <AkAsset/gltf/Util.hpp>
#include <AkAsset/Mesh.hpp>
#include <AkCommon/PrimitiveTypes.hpp>
#include <AkCommon/Iterator.hpp>
#include <AkMath/Types.hpp>
#include <AkMath/Vector.hpp>
#include <AkRender/gl/Types.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>
#include <array>
#include <deque>
#include <map>
#include <utility>
#include <vector>

using namespace aka::gltf;

static aka::Primitive processPrimitive(const Asset& asset, const Primitive& primitive, const std::vector<aka::ConversionInfo>& materialUIDs);
static std::vector<aka::VertexSurfaceData> extractSurfaceData(const Asset& asset, const Primitive& primitive);
static std::array<std::vector<akm::Vec2>, 4> extractTextureData(const Asset& asset, const Primitive& primitive);
static std::vector<akm::Vec4> extractColourData(const Asset& asset, const Primitive& primitive);
static std::vector<aka::VertexWeightData> extractSkinningData(const Asset& asset, const Primitive& primitive);
static void extractTangentSpaceVectors(const std::vector<akm::Vec3>& normals, const std::vector<akm::Vec4>& signedTangents, std::vector<akm::Vec3>& tangents, std::vector<akm::Vec3>& bitangents);
static std::vector<aka::VertexSurfaceData> processSurfaceData(const std::vector<akm::Vec3>& positions, const std::vector<akm::Vec3>& tangents, const std::vector<akm::Vec3>& bitangents, std::vector<akm::Vec3> normals);
static std::vector<std::map<uint32, fpSingle>> collectBoneWeights(const Asset& asset, const Primitive& primitive);
static aka::VertexWeightData selectHighestWeightedBones(const std::map<uint32, fpSingle>& boneWeights);

aka::Mesh aka::gltf::processGLTFMesh(const Asset& asset, const Mesh& mesh, const std::vector<aka::ConversionInfo>& materialUIDs) {
	std::vector<aka::Primitive> primitives;
	for(auto& primitive : mesh.primitives) primitives.push_back(processPrimitive(asset, primitive, materialUIDs));
	return {{primitives}, {}};
}

static aka::Primitive processPrimitive(const Asset& asset, const Primitive& primitive, const std::vector<aka::ConversionInfo>& materialUIDs) {
	akr::gl::DrawType drawType;
	switch(primitive.mode) {
		case PrimitiveMode::Points:    drawType = akr::gl::DrawType::Points; break;

		case PrimitiveMode::Lines:     drawType = akr::gl::DrawType::Lines;     break;
		case PrimitiveMode::LineStrip: drawType = akr::gl::DrawType::LineStrip; break;
		case PrimitiveMode::LineLoop:  drawType = akr::gl::DrawType::LineLoop;  break;

		case PrimitiveMode::Triangles:     drawType = akr::gl::DrawType::Triangles;     break;
		case PrimitiveMode::TriangleStrip: drawType = akr::gl::DrawType::TriangleStrip; break;
		case PrimitiveMode::TriangleFan:   drawType = akr::gl::DrawType::TriangleFan;   break;
	}

	auto indexData    = extractAccessorData<uint16>(asset, primitive.indiciesID);
	auto surfaceData  = extractSurfaceData(asset, primitive);
	auto texCoords    = extractTextureData(asset, primitive);
	auto colourData   = extractColourData(asset, primitive);
	auto skinningData = extractSkinningData(asset, primitive);

	return aka::Primitive{
		drawType,
		indexData,
		surfaceData,
		texCoords,
		colourData,
		skinningData,
		materialUIDs[primitive.materialID].identifier
	};
}

// ////////////////////////// //
// // Extract surface data // //
// ////////////////////////// //

static std::vector<aka::VertexSurfaceData> extractSurfaceData(const Asset& asset, const Primitive& primitive) {
	std::vector<akm::Vec3> positions = extractAccessorData<akm::Vec3>(asset, primitive.positionID);
	std::vector<akm::Vec3> normals = extractAccessorData<akm::Vec3>(asset, primitive.normalID);
	std::vector<akm::Vec3> tangents, bitangents; extractTangentSpaceVectors(normals, extractAccessorData<akm::Vec4>(asset, primitive.tangentID), tangents, bitangents);
	return processSurfaceData(positions, tangents, bitangents, normals);
}

static std::vector<aka::VertexSurfaceData> processSurfaceData(const std::vector<akm::Vec3>& positions, const std::vector<akm::Vec3>& tangents, const std::vector<akm::Vec3>& bitangents, std::vector<akm::Vec3> normals) {
	std::vector<aka::VertexSurfaceData> result; result.reserve(positions.size());
	for(akSize i = 0; i < positions.size(); i++) result.push_back({positions[i], tangents[i], bitangents[i], normals[i]});
	return result;
}

static void extractTangentSpaceVectors(const std::vector<akm::Vec3>& normals, const std::vector<akm::Vec4>& signedTangents, std::vector<akm::Vec3>& tangents, std::vector<akm::Vec3>& bitangents) {
	tangents.reserve(signedTangents.size()); bitangents.reserve(signedTangents.size());
	for(akSize i = 0; i < normals.size(); i++) {
		tangents.push_back(akm::Vec3(signedTangents[i]));
		bitangents.push_back(signedTangents[i].w * akm::cross(normals[i], tangents[i]));
	}
}

// ///////////////////////////// //
// // Extract appearance data // //
// ///////////////////////////// //

static std::array<std::vector<akm::Vec2>, 4> extractTextureData(const Asset& asset, const Primitive& primitive) {
	std::array<std::vector<akm::Vec2>, 4> result;
	for(akSize i = 0; i < result.size() && i < primitive.texCoordIDs.size(); i++) result[i] = extractAccessorData<akm::Vec2>(asset, primitive.texCoordIDs[i]);
	if (result.size() < primitive.texCoordIDs.size()) akl::Logger("GLTF").warn("Only up to ", result.size(), " UV sets are supported.");
	return result;
}

static std::vector<akm::Vec4> extractColourData(const Asset& asset, const Primitive& primitive) {
	if (primitive.colourIDs.size() > 0) {
		if (primitive.colourIDs.size() > 1) akl::Logger("GLTF").warn("Multiple colour sets not supported.");
		if (asset.accessors[primitive.colourIDs[0]].type == AccessorType::Vec3) return aku::convert_to<std::vector<akm::Vec4>>(extractAccessorData<akm::Vec3>(asset, primitive.colourIDs[0]), [](const auto& colourRGB){ return akm::Vec4{colourRGB.r, colourRGB.g, colourRGB.b, 1.0f};});
		else return extractAccessorData<akm::Vec4>(asset, primitive.colourIDs[0]);
	}
	return std::vector<akm::Vec4>();
}

// /////////////////////////// //
// // Extract skinning data // //
// /////////////////////////// //

static std::vector<aka::VertexWeightData> extractSkinningData(const Asset& asset, const Primitive& primitive) {
	if (primitive.jointIDs.size() > 0) {
		auto vertexWeights = collectBoneWeights(asset, primitive);
		std::vector<aka::VertexWeightData> result; result.reserve(vertexWeights.size());
		for(auto& boneWeights : vertexWeights) result.push_back(selectHighestWeightedBones(boneWeights));
		return result;
	}
	return std::vector<aka::VertexWeightData>();
}

static std::vector<std::map<uint32, fpSingle>> collectBoneWeights(const Asset& asset, const Primitive& primitive) {
	if (primitive.jointIDs.size() > 1) akl::Logger("GLTF").warn("More than 4 bone weights per vertex provided, will select 4 highest weighted bones and normalize their associated weights.");
	std::vector<std::map<uint32, fpSingle>> vertexWeights;
	for(uint32 i = 0; i < primitive.jointIDs.size(); i++) {
		std::vector<akm::Vec4> tmpBones = extractAccessorData<akm::Vec4>(asset, primitive.jointIDs[0]);
		std::vector<akm::Vec4> tmpWeights = extractAccessorData<akm::Vec4>(asset, primitive.weightIDs[0]);

		if (vertexWeights.size() < tmpBones.size()) vertexWeights.resize(tmpBones.size());

		for(uint32 j = 0; j < tmpBones.size(); j++) {
			auto& boneWeights = vertexWeights[j];
			for(uint32 k = 0; k < 4; k++) {
				auto boneID = static_cast<uint32>(tmpBones[j][k]);
				auto weightID = tmpWeights[j][k];
				if (weightID == 0) continue;

				auto inResult = boneWeights.emplace(boneID, weightID);
				if (!inResult.second) inResult.first->second += weightID;
			}
		}
	}
	return vertexWeights;
}

static aka::VertexWeightData selectHighestWeightedBones(const std::map<uint32, fpSingle>& boneWeights) {
	std::deque<uint32> selectedBones({0,0,0,0});
	std::deque<fpSingle> selectedWeights({0,0,0,0});
	for(auto boneWeight : boneWeights) {
		bool didInsert = false;
		for(akSize k = 0; k < selectedBones.size(); k++) {
			if (boneWeight.second < selectedWeights[k]) continue;
			didInsert = true;

			selectedBones.insert(selectedBones.begin() + k, boneWeight.first);
			selectedWeights.insert(selectedWeights.begin() + k, boneWeight.second);

			if (selectedWeights.size() > 4) {
				selectedBones.pop_back();
				selectedWeights.pop_back();
			}

			break;
		}

		if ((!didInsert) && (selectedBones.size() < 4)) {
			selectedBones.push_back(boneWeight.first);
			selectedWeights.push_back(boneWeight.second);
		}
	}

	aka::VertexWeightData result{
		{{selectedBones[0], selectedBones[1], selectedBones[2], selectedBones[3]}},
		{selectedWeights[0], selectedWeights[1], selectedWeights[2], selectedWeights[3]}
	};

	result.weights = result.weights/akm::sum(result.weights);
	return result;
}


