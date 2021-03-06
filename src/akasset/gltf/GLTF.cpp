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

#include <akasset/Animation.hpp>
#include <akasset/Convert.hpp>
#include <akasset/gltf/Animation.hpp>
#include <akasset/gltf/Asset.hpp>
#include <akasset/gltf/Buffer.hpp>
#include <akasset/gltf/GLTF.hpp>
#include <akasset/gltf/GLTFAnimation.hpp>
#include <akasset/gltf/GLTFMaterial.hpp>
#include <akasset/gltf/GLTFMesh.hpp>
#include <akasset/gltf/GLTFSkin.hpp>
#include <akasset/gltf/Types.hpp>
#include <akasset/Material.hpp>
#include <akasset/Mesh.hpp>
#include <akasset/Skin.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/Timer.hpp>
#include <akengine/data/Json.hpp>
#include <akengine/data/PValue.hpp>
#include <akengine/data/Random.hpp>
#include <akengine/data/Serialize.hpp>
#include <akengine/data/SUID.hpp>
#include <akengine/filesystem/CFile.hpp>
#include <akengine/filesystem/Filesystem.hpp>
#include <akengine/filesystem/Path.hpp>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using namespace aka::gltf;

static Asset loadAsset(const akfs::Path& filename);
static void resolveBuffers(const akfs::Path& root, Asset& asset);

static aka::ConversionInfo getAssetInfo(akd::PValue& cfg, const aka::ConversionHelper& convertHelper, const std::string& categoryName, const std::string& entryName, const std::string& defaultPath, const std::optional<akfs::Path>& source = {}) {
	auto& entry = cfg[categoryName].atOrSet(entryName);
	auto& defEntry = cfg[categoryName].atOrSet("");

	auto defPathStr = (akfs::Path(defEntry.getStrOrDef("data/"))/defaultPath).str(); // This isn't right
	auto info = convertHelper.findConversionInfo(
		entryName,
		akd::tryDeserialize<akd::SUID>(entry.atOrDef("identifier")),
		source,
		entry.exists("destination") ? std::optional<akfs::Path>{akfs::Path(entry.at("destination").getStr())} : std::optional<akfs::Path>{},
		akfs::Path(defPathStr)
	);

	akd::serialize(entry["identifier"], info.identifier);
	entry["destination"].setStr(info.destination.str());

	return info;
}

bool aka::gltf::convertGLTFFile(aka::ConversionHelper& convertHelper, const akfs::Path& cfgPath, akd::PValue& cfg) {
	akd::CMW4096Engine32d randomGenerator(akd::CMW4096Engine32d::default_seed ^ std::rand() ^ std::time(nullptr));
	akc::Timer proccessTimer, stepTimer;

	bool addedConversion = false;

	auto root = akfs::Path(cfgPath).pop_back();
	auto assetFilename = root/cfg["source"].getStr();
	bool skipExisting = false;
	{
		auto lastModifiedTime = cfg.atOrDef("modifiedTime").asOrDef<int64>(std::numeric_limits<int64>::min());
		auto curModifiedTime = akfs::modifiedTime(assetFilename);
		skipExisting = curModifiedTime >= lastModifiedTime;
		if (!skipExisting) cfg["modifiedTime"].setSInt(curModifiedTime);
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
	std::vector<aka::ConversionInfo> images;
	for(akSize i = 0; i < asset.images.size(); i++) {
		auto& image = asset.images[i];
		auto srcPath = root/image.uri;
		auto info = convertHelper.tryFindAssetInfoBySource(srcPath);
		if (!info) info = convertHelper.tryFindAssetInfoByDestination(convertHelper.getDestinationSuggestionFor(root, image.uri));

		if (!info) {
			akl::Logger("GLTF").warn("Could not find asset info for '", srcPath.str(), "' for '", image.name);
			return false;
		}

		images.push_back(*info);
	}

	// ////////////////////////// //
	// // Processing Materials // //
	// ////////////////////////// //
	std::vector<aka::ConversionInfo> materials;
	for(akSize i = 0; i < asset.materials.size(); i++) {

		auto& material = asset.materials[i];
		auto info = getAssetInfo(cfg, convertHelper, "materials", material.name, material.name + ".akmat.akres");
		materials.push_back(info);

		if ((skipExisting) && (akfs::exists(info.destination) && akfs::exists(akfs::Path(info.destination).clearExtension()))) continue;

		convertHelper.registerMaterial(
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
		for(auto& nodeID : meshNodes) {
			auto& node = asset.nodes[nodeID];
			auto& mesh = asset.meshes[node.meshID];

			auto info = getAssetInfo(cfg, convertHelper, "meshes", mesh.name, mesh.name + ".akmesh.akres");

			if ((skipExisting) && (akfs::exists(info.destination)) && akfs::exists(akfs::Path(info.destination).clearExtension())) continue;

			convertHelper.registerMesh(
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

	// /////////////////////////// //
	// // Processing Animations // //
	// /////////////////////////// //
	for(auto& animation : asset.animations) {
		auto info = getAssetInfo(cfg, convertHelper, "animations", animation.name, animation.name + ".akanim.akres");
		if ((skipExisting) && (akfs::exists(info.destination)) && akfs::exists(akfs::Path(info.destination).clearExtension())) continue;

		convertHelper.registerAnimation(
			info,
			gltf::proccessGLTFAnimation(asset, animation),
			{}
		);

		addedConversion = true;
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


