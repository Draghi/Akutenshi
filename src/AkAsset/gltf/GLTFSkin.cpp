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

#include <AkAsset/gltf/GLTFSkin.hpp>
#include <AkAsset/gltf/Asset.hpp>
#include <AkAsset/gltf/Skin.hpp>
#include <AkAsset/gltf/Types.hpp>
#include <AkAsset/gltf/Util.hpp>
#include <AkAsset/Skin.hpp>
#include <AkCommon/PrimitiveTypes.hpp>
#include <AkCommon/Iterator.hpp>
#include <AkMath/Types.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <algorithm>
#include <iterator>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace aka::gltf;

aka::Skin aka::gltf::processGLTFSkin(const Asset& asset, const Skin& skin, gltfID nodeID, const std::unordered_map<gltfID, gltfID>& parentMap) {
	auto inverseBindMatricies = extractAccessorData<akm::Mat4>(asset, skin.inverseBindMatricies);

	// Find path from joint to a root node
	std::vector<std::vector<gltfID>> parentTraces;
	for(auto jointID : skin.jointNodeIDs) {
		std::vector<gltfID> parentTrace({jointID}); // Start with joint node ID
		decltype(std::begin(parentMap)) iter;
		while((iter = parentMap.find(parentTrace.back())) != parentMap.end()) parentTrace.push_back(iter->second); // Push back last node's parent node, until not found (root)
		parentTraces.push_back(std::move(parentTrace));
	}

	gltfID rootID = -1;
	akSSize traceDepth = -1;
	for(akSize i = 1;; i++) {
		gltfID cID = -1;
		for(const auto& parentTrace : parentTraces) {
			if (i >= parentTrace.size()) { cID = -1; break; } // Ran out of path, root node is at the pervious i-index
			if (cID < 0) cID = parentTrace[parentTrace.size() - i];
			if (cID != parentTrace[parentTrace.size() - i]) { cID = -1; break; } // Mismatch, root node is at the previous i-index
		}
		if (cID < 0) {
			traceDepth = i - 1;
			rootID = parentTraces.front()[parentTraces.front().size() - traceDepth];
			if (!std::all_of(parentTraces.begin(), parentTraces.end(), [&](const auto& parentTrace) {return parentTrace[parentTrace.size() - traceDepth] == rootID;})) {
				rootID = -1;
			}
			break;
		}
	}

	// Trace common node to child of skin node. We don't support out-of-place armatures.
	decltype(std::begin(parentMap)) iter;
	while((iter = parentMap.find(rootID)) != parentMap.end()) if (iter->second == nodeID) break;
	if (iter == parentMap.end()) throw std::runtime_error("Skin nodes must be children of the node it's attached to.");
	else rootID = iter->first;

	// Collect list of nodes involved in skeleton, to ensure we're not missing out on some important structural nodes
	std::set<gltfID> jointSet({rootID});
	for(const auto& parentTrace : parentTraces) {
		for(akSize i = 0; i < parentTrace.size() - traceDepth; i++) {
			jointSet.insert(parentTrace[i]);
		}
	}

	// Our nodes are sorted and are reindexed from 0, however, we re-index them so meshes know which bone is what.
	std::map<gltfID, uint32> jointIndexMap;
	for(const auto& jointID : jointSet) jointIndexMap.emplace(jointID, jointIndexMap.size());

	// Extracting the marked nodes to form the skin
	std::vector<aka::Joint> jointHierarchy; jointHierarchy.reserve(jointSet.size());
	for(const auto& jointID : jointSet) {
		jointHierarchy.push_back({
			asset.nodes[jointID].name,
			aku::convert_to_if<std::vector<uint32>>(asset.nodes[jointID].childrenIDs, [&](const auto& childID){
				auto jointIter = jointIndexMap.find(childID);
				return jointIter == jointIndexMap.end() ? std::optional<uint32>() : std::optional<uint32>(jointIter->second);
			}),
			akm::Mat4(1),
			asset.nodes[jointID].position,
			asset.nodes[jointID].rotation,
			asset.nodes[jointID].scale
		});
	}

	std::vector<uint32> jointMapping; jointMapping.reserve(skin.jointNodeIDs.size());
	for(uint32 i = 0; i < skin.jointNodeIDs.size(); i++) {
		jointHierarchy[jointIndexMap.at(skin.jointNodeIDs[i])].inverseBindMatrix = inverseBindMatricies[i];
		jointMapping.push_back(skin.jointNodeIDs[i]);
	}

	return aka::Skin{
		jointIndexMap.at(rootID),
		jointHierarchy,
		jointMapping
	};
}


