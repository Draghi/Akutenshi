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

#include <ak/assets/Convert.hpp>

#include <functional>

#include <ak/assets/gltf/GLTF.hpp>
#include <ak/data/Json.hpp>
#include <ak/data/MsgPack.hpp>
#include <ak/data/PValue.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/util/Timer.hpp>

using namespace akas;

static void writeMeshes(ConversionHelper& state);
static void writeMaterials(ConversionHelper& state);
static void writeCopies(ConversionHelper& state);

static bool convertTexture(ConversionHelper& state, const akfs::Path& root, akd::PValue& cfg);
static bool convertImage(ConversionHelper& state, const akfs::Path& root, akd::PValue& cfg);

static auto writeAssetMetaFile = [](const akfs::Path& dst, const akd::SUID& suid, akas::AssetType assetType, const std::string& name, const akfs::Path& source){
	akd::PValue dstData; akd::serialize(dstData, akas::AssetInfo{suid, assetType, name, source});
	return akl::Logger("Convert").test_warn(akd::toJsonFile(dstData, dst), "Failed to write to file: ", dst.str());
};

static auto writeAssetFile = [](const akfs::Path& filename, const auto& data, bool asJson) {
	akd::PValue dstData; akd::serialize(dstData, data);
	if (asJson) return akl::Logger("Convert").test_warn(akd::toJsonFile(   dstData, filename),       "Failed to write to file: ", filename.str());
	else        return akl::Logger("Convert").test_warn(akd::toMsgPackFile(dstData, filename, true), "Failed to write to file: ", filename.str());
};

using callback_t = bool(ConversionHelper&, const akfs::Path&, akd::PValue&);
static const std::map<std::string, callback_t*> proccessFunctions{{
	{"GLTF", &akas::gltf::convertGLTFFile},
	{"Texture", &convertTexture},
	{"Image", &convertImage}
}};

void akas::convertDirectory(const akfs::Path& dir) {

	aku::Timer convertTimer;

	akSize fileCount = 0;

	std::map<akas::AssetSourceType, std::vector<akfs::Path>> collectedAssets;
	akfs::iterateDirectory(dir, [&](const akfs::Path& path, bool isDir){
		if ((isDir) || (path.extension() != ".akconv")) return true;
		akd::PValue convData = akd::fromJsonFile(path);
		collectedAssets[akd::deserialize<akas::AssetSourceType>(convData["type"])].push_back(path);
		fileCount     += 1;
		return true;
	}, true);

	akl::Logger("Convert").info("Found ", fileCount, " asset conversion definition files in ", convertTimer.markAndReset().msecs() , "ms.");

	akSize proccessCount = 0;
	akSize     meshCount = 0;
	akSize materialCount = 0;
	akSize    imageCount = 0;

	for(auto assetTypeIter = collectedAssets.begin(); assetTypeIter != collectedAssets.end(); assetTypeIter++) {
		for(auto assetPathIter = assetTypeIter->second.begin(); assetPathIter != assetTypeIter->second.end(); assetPathIter++) {
			ConversionHelper convertHelper(akfs::Path("data/"), true);

			akd::PValue convData = akd::fromJsonFile(*assetPathIter);
			auto iter = proccessFunctions.find(convData["type"].asStr());
			if (iter == proccessFunctions.end()) { akl::Logger("Convert").warn("Could not proccess type '",  convData["type"].asStr(), "' in file: ", (*assetPathIter).str()); continue; }

			if (iter->second(convertHelper, akfs::Path(*assetPathIter).pop_back(), convData)) proccessCount++;

			akl::Logger("Convert").test_warn(akd::toJsonFile(convData, *assetPathIter), "Could not resave conversion file: ", assetPathIter->str());

			writeMeshes(convertHelper);
			writeMaterials(convertHelper);
			writeCopies(convertHelper);

			meshCount     += convertHelper.meshCount();
			materialCount += convertHelper.materialCount();
			imageCount    += convertHelper.imageCount();
		}
	}

	akl::Logger("Convert").info("Converted ", proccessCount, " out of ", fileCount, " assets in ", convertTimer.markAndReset().msecs() , "ms. Resulting in ", meshCount, " meshes, ", materialCount, " materials, ", imageCount, " images.");
}

static void writeMeshes(ConversionHelper& state) {
	for(auto& mesh : state.meshes()) {
		if (!writeAssetMetaFile(mesh.first.destination, mesh.first.identifier, akas::AssetType::Mesh, mesh.first.displayName, {})) continue;
		if (!writeAssetFile(akfs::Path(mesh.first.destination).clearExtension(), mesh.second, false)) continue;
	}
}

static void writeMaterials(ConversionHelper& state) {
	for(auto& material : state.materials()) {
		if (!writeAssetMetaFile(material.first.destination, material.first.identifier, akas::AssetType::Material, material.first.displayName, {})) continue;
		if (!writeAssetFile(akfs::Path(material.first.destination).clearExtension(), material.second, false)) continue;
	}
}

static void writeCopies(ConversionHelper& state) {
	for(auto& copy : state.copies()) {
		if (!writeAssetMetaFile(copy.first.destination, copy.first.identifier, akas::AssetType::Image, copy.first.displayName, copy.second)) continue;
		if (!akfs::copy(copy.second, akfs::Path(copy.first.destination).clearExtension())) continue;
	}
}

static bool convertTexture(ConversionHelper& convertHelper, const akfs::Path& root, akd::PValue& cfg) {
	auto& texCfg = cfg["config"];

	auto& levelsCfg = texCfg["levels"].asArr();
	for(auto levelCfg = levelsCfg.begin(); levelCfg != levelsCfg.end(); levelCfg++) {
		for(auto imgCfg = levelCfg->asArr().begin(); imgCfg != levelCfg->asArr().end(); imgCfg++) {
			auto& source = imgCfg->at("source").asStr();

		}
	}


	return true;
}

static bool convertImage(ConversionHelper& state, const akfs::Path& root, akd::PValue& cfg) {
	auto srcPath = root/akfs::Path(cfg.at("source").asStr());

	auto info = state.findConversionInfo(
		srcPath.filename() + ".akres",
		akd::tryDeserialize<akd::SUID>(cfg.atOrDef("identifier")),
		srcPath,
		cfg["destination"].tryAs<std::string>(),
		state.getDestinationSuggestionFor(root, cfg.at("source").asStr())
	);

	cfg.atOrSet("identifier").setPValue(akd::serialize(info.identifier));
	if (cfg.atOrDef("remapPaths").asOrDef<bool>(false)) cfg.atOrSet("destination").setStr(info.destination.str());

	if (!akfs::exists(srcPath)) {
		akl::Logger("Image").warn("Missing image asset: ", srcPath.str());
		return false;
	}

	if (akfs::exists(info.destination) && akfs::exists(akfs::Path(info.destination).clearExtension())) {
		auto srcModifiedTime = akfs::modifiedTime(srcPath);
		auto dstModifiedTime = akfs::modifiedTime(akfs::Path(info.destination).clearExtension());
		if (dstModifiedTime >= srcModifiedTime) return false;
	}

	state.registerAsset(info, srcPath, srcPath);
	return true;
}
