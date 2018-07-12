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

#include <akres/Models.hpp>

bool akres::doPackModel(const akfs::Path& srcPath, const akfs::Path& outPath, const std::string& modelName) {
	return false;
}

/*#include <ak/animation/Animation.hpp>
#include <ak/animation/Fwd.hpp>
#include <ak/animation/Mesh.hpp>
#include <ak/animation/Serialize.hpp>
#include <ak/animation/Skeleton.hpp>
#include <ak/animation/Type.hpp>
#include <ak/data/Brotli.hpp>
#include <ak/data/MsgPack.hpp>
#include <ak/data/PValue.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/util/Timer.hpp>
#include <akres/Models.hpp>
#include <assimp/anim.h>
#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/types.h>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>
#include <deque>
#include <iostream>
#include <map>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace akres;

static aka::Mesh extractMesh(const aiMesh& mesh);
static aka::Skeleton extractSkeleton(const aiScene& scene, const aiMesh& skeleton);
static std::vector<std::pair<std::string, aka::Animation>> extractAnimations(const aiScene& scene, const aka::Skeleton& skeleton);

template<typename type_t> static akm::Mat4 aiMatToAk(const aiMatrix4x4t<type_t>& mat) {
	return akm::Mat4{
		mat.a1, mat.b1, mat.c1, mat.d1,
		mat.a2, mat.b2, mat.c2, mat.d2,
		mat.a3, mat.b3, mat.c3, mat.d3,
		mat.a4, mat.b4, mat.c4, mat.d4
	};
}

bool akres::doPackModel(const akfs::Path& srcPath, const akfs::Path& outPath, const std::string& modelName) {
	aku::Timer modelFileTimer;
	std::cout << "[Models] Start processing: " << (srcPath/modelName).str() << std::endl;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile((srcPath/modelName).str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_MakeLeftHanded);

	if (scene) std::cout << "[Models] Successfully loaded model file." << std::endl;
	else {     std::cout << "[Models] Failed to load model file."      << std::endl; return false; }

	if (scene->mNumMeshes <= 0) { std::cout << "[Models] Model file contains no sub-models." << std::endl; return false; }

	akd::PValue akModelsData;

	for(uint32 i = 0; i < scene->mNumMeshes; i++) {
		aku::Timer modelTimer;

		const aiMesh& mesh = *scene->mMeshes[i];
		auto meshName = std::string(mesh.mName.data, mesh.mName.length);
		if (akModelsData.exists(meshName)) {
			std::cout << "[Model] Model name conflict: " << meshName << std::endl;
			return false;
		}

		std::cout << std::endl << "[Model] Started processing model: " << meshName << std::endl;

		auto& akModelData = akModelsData[meshName];
		{
			aku::Timer meshTimer;
			auto meshData = extractMesh(mesh);
			if (meshData.vertexData().empty()) continue;
			akd::serialize(akModelData["mesh"], meshData);
			std::cout << "[Mesh] Processed mesh for in " << meshTimer.mark().msecs() << "ms." << std::endl;
		}

		if (mesh.mNumBones > 0) {
			aku::Timer skeleTimer;
			auto skeletonData = extractSkeleton(*scene, mesh);
			akd::serialize(akModelData["skeleton"], skeletonData);
			std::cout << "[Skel] Processed skeleton in " << skeleTimer.mark().msecs() << "ms." << std::endl;

			if (scene->mNumAnimations > 0) {
				aku::Timer animTimer;
				auto animations = extractAnimations(*scene, skeletonData);
				for(auto& anim : animations) {
					std::cout << "[Anim] Converting to generic data representation..." << std::endl;
					akd::PValue animCfg;
					if (akModelData["animations"].exists(anim.first)) { std::cout << "[Anim] Animation name conflict: " << anim.first << std::endl; return false; }
					akd::serialize(akModelData["animations"][anim.first], anim.second);
				}
				std::cout << "[Anim] Processed animations in " << animTimer.mark().msecs() << "ms." << std::endl;
			} else {
				std::cout << "[Anim] No animations found. Skipping." << std::endl;
			}
		} else {
			std::cout << "[Skel] No skeleton found. Skipping." << std::endl;
		}

		std::cout << "[Model] Processed model in " << modelTimer.mark().msecs() << "ms." << std::endl;
	}

	std::cout << std::endl  << "[Models] Finished processing model file in " << modelFileTimer.mark().msecs() << "ms." << std::endl << std::endl;

	aku::Timer writeTimer;

	auto tmpPath = outPath/modelName;
	auto outFilename = akfs::Path(tmpPath.str().substr(0, tmpPath.str().size() - tmpPath.extension().size()) + ".akmesh");
	auto oFile = akfs::CFile(outFilename, akfs::OpenFlags::Out | akfs::OpenFlags::Truncate);
	if (oFile) std::cout << "[Write] Opened '"         << oFile.path().str() << "' for output." << std::endl;
	else {     std::cout << "[Write] Failed to open '" << oFile.path().str() << "' for output." << std::endl; return false; }

	std::cout << "[Write] Exporting data..." << std::endl;
	auto dstData = akd::toMsgPack(akModelsData);

	std::cout << "[Write] Compressing data..." << std::endl;
	dstData = akd::compressBrotli(dstData, 10);

	std::cout << "[Write] Writing..." << std::endl;
	if (!oFile.write(dstData.data(), dstData.size())) {
		std::cout << "[Write] Failed to write mesh to file." << std::endl;
		return false;
	}

	std::cout << "[Write] Finished writing model to disk in " << writeTimer.mark().msecs() << "ms." << std::endl;

	return true;
}

static aka::Mesh extractMesh(const aiMesh& mesh) {
	auto meshName = std::string(mesh.mName.data, mesh.mName.length);

	std::cout << "[Mesh] Extracting vertex data..." << std::endl;
		std::vector<aka::VertexData> vertexData;
		if ((!mesh.mTangents) || (!mesh.mBitangents) || (!mesh.mNormals)) {
			std::cout << "[Mesh] Missing normal data. Skipping." << std::endl;
			return aka::Mesh();
		}
		vertexData.reserve(mesh.mNumVertices);

		for(auto vertIndex = 0u; vertIndex < mesh.mNumVertices; vertIndex++) {
			vertexData.push_back(aka::VertexData{
				akm::Vec3{mesh.mVertices[vertIndex].x, mesh.mVertices[vertIndex].y, mesh.mVertices[vertIndex].z},
				akm::normalize(akm::Vec3{mesh.mTangents[vertIndex].x, mesh.mTangents[vertIndex].y, mesh.mTangents[vertIndex].z}),
				akm::normalize(akm::Vec3{mesh.mBitangents[vertIndex].x, mesh.mBitangents[vertIndex].y, mesh.mBitangents[vertIndex].z}),
				akm::normalize(akm::Vec3{mesh.mNormals[vertIndex].x, mesh.mNormals[vertIndex].y, mesh.mNormals[vertIndex].z}),
				mesh.mTextureCoords ? akm::Vec2{mesh.mTextureCoords[0][vertIndex].x, mesh.mTextureCoords[0][vertIndex].y} : akm::Vec2{0, 0}
			});
		}

	std::cout << "[Mesh] Extracting index data..." << std::endl;
		std::vector<aka::IndexData> indexData;
		indexData.reserve(mesh.mNumFaces);
		for(auto faceIndex = 0u; faceIndex < mesh.mNumFaces; faceIndex++) {
			const aiFace& inFace = mesh.mFaces[faceIndex];
			if (inFace.mNumIndices != 3) throw std::runtime_error("Invalid index count. Only triangles supported.");
			indexData.push_back(aka::IndexData{{
				inFace.mIndices[0],
				inFace.mIndices[1],
				inFace.mIndices[2]
			}});
		}

	std::cout << "[Mesh] Extracting bone weights..." << std::endl;
		std::vector<aka::BoneWeights> boneWeights;
		boneWeights.resize(vertexData.size());
		for(auto boneIndex = 0u; boneIndex < mesh.mNumBones; boneIndex++) {
			auto& bone = mesh.mBones[boneIndex];
			std::string boneName = std::string(bone->mName.data, bone->mName.length);
			for(auto weightIndex = 0u; weightIndex < bone->mNumWeights; weightIndex++) {
				auto vIndex = bone->mWeights[weightIndex].mVertexId;
				boneWeights[vIndex][boneName] = bone->mWeights[weightIndex].mWeight;
			}
		}

	return aka::Mesh(std::move(vertexData), std::move(boneWeights), std::move(indexData));
}

static aka::Skeleton extractSkeleton(const aiScene& scene, const aiMesh& mesh) {

	std::cout << "[Skel] Building scenegraph lookup..." << std::endl;
		const aiNode* meshNode = nullptr;
		std::unordered_map<std::string, const aiNode*> nodeMap;
		std::vector<const aiNode*> nodes{scene.mRootNode};
		while(!nodes.empty()) {
			auto node = nodes.back(); nodes.pop_back();
			nodeMap.insert(std::make_pair(std::string(node->mName.data, node->mName.length), node));
			for(auto i = 0u; i < node->mNumMeshes; i++) if (node->mMeshes[i] == 0) meshNode = node; //@todo Mesh node, we're loading all of them...
			for(auto i = 0u; i < node->mNumChildren; i++) nodes.push_back(node->mChildren[i]);
		}

	std::cout << "[Skel] Identifying mesh bones..." << std::endl;
		std::unordered_set<const aiNode*> necessaryNodes;
		std::unordered_map<std::string, const aiBone*> boneLookup;
		for(auto boneIndex = 0u; boneIndex < mesh.mNumBones; boneIndex++) {
			auto& boneInfo = mesh.mBones[boneIndex];
			std::string boneName = std::string(boneInfo->mName.data, boneInfo->mName.length);
			boneLookup.insert(std::make_pair(boneName, boneInfo));

			auto& boneNode = nodeMap[boneName];
			std::deque<const aiNode*> nodeStack{boneNode};
			while((boneNode) && (necessaryNodes.find(boneNode) == necessaryNodes.end())) {
				necessaryNodes.insert(boneNode);
				boneNode = boneNode->mParent;
			}
		}

	std::cout << "[Skel] Extracting skeleton..." << std::endl;
		std::vector<aka::NamedBone> skeletonBones;
		skeletonBones.reserve(necessaryNodes.size());
		for(auto iter = necessaryNodes.begin(); iter != necessaryNodes.end(); iter++) {
			auto& cNode = (*iter);

			auto parentName = cNode->mParent ? std::string(cNode->mParent->mName.data, cNode->mParent->mName.length) : "";
			auto name = std::string(cNode->mName.data, cNode->mName.length);

			std::vector<std::string> children;
			children.reserve(cNode->mNumChildren);
			for(auto i = 0u; i < cNode->mNumChildren; i++) {
				if (necessaryNodes.find(cNode->mChildren[i]) == necessaryNodes.end()) continue;
				children.push_back(std::string(cNode->mChildren[i]->mName.data, cNode->mChildren[i]->mName.length));
			}

			auto nodeMatrix = aiMatToAk(cNode->mTransformation);

			auto boneIter = boneLookup.find(name);
			auto boneMatrix = (boneIter == boneLookup.end()) ? akm::Mat4(1) : aiMatToAk(boneIter->second->mOffsetMatrix);

			skeletonBones.push_back(aka::NamedBone{
				parentName,
				name,
				children,
				{nodeMatrix, boneMatrix}
			});
		}

	return aka::Skeleton(skeletonBones);
}

static std::vector<std::pair<std::string, aka::Animation>> extractAnimations(const aiScene& scene, const aka::Skeleton& skeleton) {

	std::vector<std::pair<std::string, aka::Animation>> result;

	for(auto i = 0u; i < scene.mNumAnimations; i++) {
		auto& anim = *scene.mAnimations[i];
		auto animName = std::string(anim.mName.data, anim.mName.length);

		std::vector<aka::AnimNode> animNodes;
		animNodes.reserve(anim.mNumChannels);

		for(auto j = 0u; j < anim.mNumChannels; j++) {
			auto& node = *anim.mChannels[j];

			std::string name = std::string(node.mNodeName.data, node.mNodeName.length);
			if (skeleton.tryFindIDByName(name) < 0) continue;

			std::map<fpSingle, akm::Quat> rotFrames;
			for(auto k = 0u; k < node.mNumRotationKeys; k++) {
				auto& frame = node.mRotationKeys[k];
				rotFrames.insert(std::pair<fpSingle, akm::Quat>(static_cast<fpSingle>(frame.mTime), akm::Quat(frame.mValue.z, frame.mValue.w, frame.mValue.x, frame.mValue.y)));
			}

			std::map<fpSingle, akm::Vec3> posFrames;
			for(auto k = 0u; k < node.mNumPositionKeys; k++) {
				auto& frame = node.mPositionKeys[k];
				posFrames.insert(std::make_pair<fpSingle, akm::Vec3>(static_cast<fpSingle>(frame.mTime), akm::Vec3{frame.mValue.x, frame.mValue.y, frame.mValue.z}));
			}

			std::map<fpSingle, akm::Vec3> sclFrames;
			for(auto k = 0u; k < node.mNumScalingKeys; k++) {
				auto& frame = node.mScalingKeys[k];
				sclFrames.insert(std::make_pair<fpSingle, akm::Vec3>(static_cast<fpSingle>(frame.mTime), akm::Vec3{frame.mValue.x, frame.mValue.y, frame.mValue.z}));
			}

			auto aiTweenToAk = [](auto val){
				switch(val) {
					case _aiAnimBehaviour_Force32Bit: [[fallthrough]];
					case aiAnimBehaviour_DEFAULT:  return aka::TweenMode::None;
					case aiAnimBehaviour_CONSTANT: return aka::TweenMode::Nearest;
					case aiAnimBehaviour_LINEAR:   return aka::TweenMode::Linear;
					case aiAnimBehaviour_REPEAT:   return aka::TweenMode::Repeat;
				}
			};

			animNodes.push_back(aka::AnimNode{name, rotFrames, posFrames, sclFrames, aiTweenToAk(node.mPreState), aiTweenToAk(node.mPostState)});
		}

		if (animNodes.empty()) continue;

		result.push_back(std::make_pair(
			animName,
			aka::Animation(static_cast<fpSingle>(anim.mTicksPerSecond), static_cast<fpSingle>(anim.mDuration), animNodes)
		));
	}
	return result;
}*/
