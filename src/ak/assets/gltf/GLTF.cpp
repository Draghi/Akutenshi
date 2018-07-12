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

#include <ak/animation/Serialize.hpp>
#include <ak/assets/Asset.hpp>
#include <ak/assets/gltf/Asset.hpp>
#include <ak/assets/gltf/Buffer.hpp>
#include <ak/assets/gltf/GLTF.hpp>
#include <ak/assets/gltf/internal/GLTFMaterial.hpp>
#include <ak/assets/gltf/internal/GLTFMesh.hpp>
#include <ak/assets/gltf/internal/GLTFSkin.hpp>
#include <ak/assets/gltf/Types.hpp>
#include <ak/assets/Skin.hpp>
#include <ak/data/Image.hpp>
#include <ak/data/Json.hpp>
#include <ak/data/PValue.hpp>
#include <ak/data/Rand.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/math/Serialize.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/util/Timer.hpp>
#include <ak/window/WindowOptions.hpp>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <string>
#include <optional>
#include <unordered_map>

using namespace akas::gltf;

static Asset loadAsset(const akfs::Path& filename);
static void resolveBuffers(const akfs::Path& root, Asset& asset);

static akas::ConversionInfo getAssetInfo(akd::PValue& cfg, const akas::ConversionHelper& convertHelper, const std::string& categoryName, const std::string& entryName, const std::string& defaultPath, const std::optional<akfs::Path>& source = {}) {
	auto& entry = cfg[categoryName].atOrSet(entryName);
	auto& defEntry = cfg[categoryName].atOrSet("");

	auto defPathStr = (akfs::Path(defEntry.asStrOrDef("data/"))/defaultPath).str();
	auto info = convertHelper.findConversionInfo(
		entryName,
		akd::tryDeserialize<akd::SUID>(entry.atOrDef("identifier")),
		source,
		entry.exists("destination") ? std::optional<akfs::Path>{akfs::Path(entry.at("destination").asStr())} : std::optional<akfs::Path>{},
		akfs::Path(defPathStr)
	);

	entry["identifier"].setPValue(akd::serialize(info.identifier));
	entry["destination"].setStr(info.destination.str());

	return info;
}

bool akas::gltf::convertGLTFFile(akas::ConversionHelper& convertHelper, const akfs::Path& root, akd::PValue& cfg) {
	akd::CMW4096Engine32d randomGenerator(akd::CMW4096Engine32d::default_seed ^ std::rand() ^ std::time(nullptr));
	aku::Timer proccessTimer, stepTimer;

	bool addedConversion = false;

	auto assetFilename = root/cfg["source"].asStr();
	bool skipExisting = false;
	{
		auto lastModifiedTime = cfg.atOrDef("modifiedTime").asOrDef<int64>(std::numeric_limits<int64>::min());
		auto curModifiedTime = akfs::modifiedTime(assetFilename);
		skipExisting = curModifiedTime >= lastModifiedTime;
		if (skipExisting) cfg["modifiedTime"].setInt(curModifiedTime);
	}

	// //////////////////// //
	// // Loading Assets // //
	// //////////////////// //
	gltf::Asset asset = loadAsset(assetFilename);

	// ///////////////////// //
	// // Resolve Buffers // //
	// ///////////////////// //
	resolveBuffers(root, asset);

	// ///////////////////////// //
	// // Processing Textures // //
	// ///////////////////////// //
	std::vector<akas::ConversionInfo> images;
	for(akSize i = 0; i < asset.images.size(); i++) {
		auto& image = asset.images[i];
		auto srcPath = root/image.uri;

		auto info = getAssetInfo(cfg, convertHelper, "images", image.name, akfs::Path(image.uri).filename() + ".akres", srcPath);
		images.push_back(info);

		if (!akfs::exists(srcPath)) {
			akl::Logger("GLTF").warn("Missing image asset: ", srcPath.str());
			continue;
		}

		if (akfs::exists(info.destination) && akfs::exists(akfs::Path(info.destination).clearExtension())) {
			auto srcModifiedTime = akfs::modifiedTime(srcPath);
			auto dstModifiedTime = akfs::modifiedTime(info.destination);
			if (dstModifiedTime >= srcModifiedTime) continue;
		}

		convertHelper.registerAsset(info, srcPath, srcPath);
		addedConversion = true;
	}

	// ////////////////////////// //
	// // Processing Materials // //
	// ////////////////////////// //
	std::vector<akas::ConversionInfo> materials;
	for(akSize i = 0; i < asset.materials.size(); i++) {

		auto& material = asset.materials[i];
		auto info = getAssetInfo(cfg, convertHelper, "materials", material.name, material.name + ".akmat.akres");
		materials.push_back(info);

		if ((skipExisting) && (akfs::exists(info.destination) && akfs::exists(akfs::Path(info.destination).clearExtension()))) continue;

		convertHelper.registerAsset(
			info,
			proccessGLTFMaterial(asset, material, images),
			{}
		);
		addedConversion = true;
	}

	// /////////////////////// //
	// // Build Node Lookup // //
	// /////////////////////// //
		std::unordered_map<gltf::gltfID, gltf::gltfID> parentLookup;
		for(akSize i = 0; i < asset.nodes.size(); i++) for(auto childID : asset.nodes[i].childrenIDs) parentLookup[childID] = i;

	// ///////////////////// //
	// // Build Mesh List // //
	// ///////////////////// //
		std::vector<gltf::gltfID> meshNodes; meshNodes.reserve(asset.meshes.size());
		for(akSize nodeID = 0; nodeID < asset.nodes.size(); nodeID++) if (asset.nodes[nodeID].meshID >= 0) meshNodes.push_back(nodeID);

	// /////////////////////// //
	// // Processing Meshes // //
	// /////////////////////// //
		std::vector<akas::ConversionInfo> meshes;
		for(auto& nodeID : meshNodes) {
			auto& node = asset.nodes[nodeID];
			auto& mesh = asset.meshes[node.meshID];

			auto info = getAssetInfo(cfg, convertHelper, "meshes", mesh.name, mesh.name + ".akmesh.akres");
			meshes.push_back(info);

			if ((skipExisting) && (akfs::exists(info.destination)) && akfs::exists(akfs::Path(info.destination).clearExtension())) continue;

			convertHelper.registerAsset(
				info,
				processGLTFMesh(asset, mesh, materials),
				{}
			);

			addedConversion = true;

			if (node.skinID >= 0) {
				auto& skin = asset.skins[node.skinID];
				convertHelper.getMesh(convertHelper.meshCount() - 1).skin = processGLTFSkin(asset, skin, nodeID, parentLookup);
			}
		}

	return addedConversion;
}

static Asset loadAsset(const akfs::Path& filename) {
	akfs::CFile modelFile(filename, akfs::OpenFlags::In);
	std::string contents; modelFile.readAllLines(contents);
	akd::PValue assetData; akd::fromJson(assetData, contents);
	return extractAsset(assetData);
}

static void resolveBuffers(const akfs::Path& root, Asset& asset) {
	for(auto& buffer : asset.buffers) {
		if (!buffer.dataURI.empty()) continue;
		auto bufferFilename = root/akfs::Path(buffer.uri);
		akfs::CFile bufferFile(bufferFilename, akfs::OpenFlags::In);
		if (!bufferFile.readAll(buffer.dataURI)) std::runtime_error("Failed to read buffer file: " + bufferFilename.str());
		buffer.uri.clear();
	}
}


