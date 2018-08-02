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

#include <deque>
#include <functional>
#include <iterator>
#include <vector>

#include <ak/animation/Fwd.hpp>
#include <ak/animation/Serialize.hpp>
#include <ak/animation/Type.hpp>
#include <ak/assets/gltf/GLTF.hpp>
#include <ak/assets/Image.hpp>
#include <ak/assets/Serialize.hpp>
#include <ak/assets/Skin.hpp>
#include <ak/data/Json.hpp>
#include <ak/data/MsgPack.hpp>
#include <ak/data/PValue.hpp>
#include <ak/data/Serialize.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/math/Serialize.hpp>
#include <ak/render/gl/Textures.hpp>
#include <ak/render/gl/Types.hpp>
#include <ak/util/Timer.hpp>
#include <ak/window/WindowOptions.hpp>

using namespace akas;

static void writeMeshes(ConversionHelper& state);
static void writeMaterials(ConversionHelper& state);
static void writeImages(ConversionHelper& state);
static void writeTextures(ConversionHelper& state);

static bool convertTexture(ConversionHelper& state, const akfs::Path& cfgPath, akd::PValue& cfg);
static bool convertImage(ConversionHelper& state, const akfs::Path& cfgPath, akd::PValue& cfg);

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
	akSize modifiedCount = 0;
	akSize     meshCount = 0;
	akSize materialCount = 0;
	akSize    imageCount = 0;
	akSize  textureCount = 0;

	for(auto assetTypeIter = collectedAssets.begin(); assetTypeIter != collectedAssets.end(); assetTypeIter++) {
		for(auto assetPathIter = assetTypeIter->second.begin(); assetPathIter != assetTypeIter->second.end(); assetPathIter++) {
			ConversionHelper convertHelper(akfs::Path("data/"), true);

			akd::PValue convData = akd::fromJsonFile(*assetPathIter);
			auto iter = proccessFunctions.find(convData["type"].getStr());
			if (iter == proccessFunctions.end()) { akl::Logger("Convert").warn("Could not proccess type '",  convData["type"].getStr(), "' in file: ", (*assetPathIter).str()); continue; }

			const auto convDataTmp = convData;
			if (iter->second(convertHelper, *assetPathIter, convData)) proccessCount++;

			if (convDataTmp != convData) {
				if (!akd::toJsonFile(convData, *assetPathIter)) akl::Logger("Convert").warn("Could not save modified conversion file at '", assetPathIter->str(), "'. SUIDs may not persist between conversions!");
				modifiedCount++;
			}

			writeMeshes(convertHelper);
			writeMaterials(convertHelper);
			writeImages(convertHelper);
			writeTextures(convertHelper);

			meshCount     += convertHelper.meshCount();
			materialCount += convertHelper.materialCount();
			imageCount    += convertHelper.imageCount();
			textureCount  += convertHelper.textureCount();
		}
	}

	akl::Logger("Convert").info("Converted ", proccessCount, " out of ", fileCount, " assets in ", convertTimer.markAndReset().msecs() , "ms.");
	akl::Logger("Convert").info("Created ", meshCount, " meshes, ", materialCount, " materials, ", textureCount, " and ", imageCount, " images.");
	akl::Logger("Convert").info("Modified ", modifiedCount, "  of the *.akconv files.");
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
		if (!writeAssetFile(akfs::Path(material.first.destination).clearExtension(), material.second, true)) continue;
	}
}

static void writeImages(ConversionHelper& state) {
	for(auto& copy : state.copies()) {
		if (!writeAssetMetaFile(copy.first.destination, copy.first.identifier, akas::AssetType::Image, copy.first.displayName, copy.second)) continue;
		if (!akfs::copy(copy.second, akfs::Path(copy.first.destination).clearExtension())) continue;
	}
}

static void writeTextures(ConversionHelper& state) {
	for(auto& texture : state.textures()) {
		if (!writeAssetMetaFile(texture.first.destination, texture.first.identifier, akas::AssetType::Texture, texture.first.displayName, {})) continue;
		if (!writeAssetFile(akfs::Path(texture.first.destination).clearExtension(), texture.second, true)) continue;
	}
}

static bool convertTexture(ConversionHelper& convertHelper, const akfs::Path& cfgPath, akd::PValue& cfg) {
	auto root = akfs::Path(cfgPath).pop_back();

	auto& texCfg = cfg["config"];
	auto& levelsCfg = texCfg["levels"].getArr();
	for(auto levelCfg = levelsCfg.begin(); levelCfg != levelsCfg.end(); levelCfg++) {
		for(auto imgCfg = levelCfg->getArr().begin(); imgCfg != levelCfg->getArr().end(); imgCfg++) {
			const auto imgPath = root/akd::deserialize<akfs::Path>(imgCfg->at("source"));

			const auto tryGetIdentifier = [&]{
				auto imgConv = akd::fromJsonFile(imgPath);
				if (!imgConv.exists("identifier")) {
					akl::Logger("ConvTex").warn("Could not find source image file identifier: ", imgPath.str());
					return false;
				}
				imgCfg->atOrSet("identifier") = imgConv["identifier"];
				return true;
			};

			if ((!imgCfg->exists("identifier")) && (!tryGetIdentifier())) return false;
			akd::SUID id = akd::tryDeserialize<akd::SUID>(imgCfg->at("identifier")).value_or(akd::SUID());
			if (!convertHelper.tryFindAssetInfoByID(id).has_value()) {
				if (!tryGetIdentifier()) return false;
				id = akd::tryDeserialize<akd::SUID>(imgCfg->at("identifier")).value_or(akd::SUID());
			}

			akl::Logger("ConvTex").test_warn(convertHelper.tryFindAssetInfoByID(id).has_value(), "Could not locate referenced file, the texture may not load correctly.");
		}
	}

	auto result = akd::deserialize<akas::Texture>(texCfg);
	auto info = convertHelper.findConversionInfo(
		akfs::Path(cfgPath).clearExtension().filename(),
		akd::tryDeserialize<akd::SUID>(cfg.atOrDef("identifier")),
		cfgPath,
		cfg.atOrDef("destination").tryAs<std::string>(),
		convertHelper.getDestinationSuggestionFor(root, akfs::Path(cfgPath).clearExtension().filename())
	);

	akd::serialize(cfg.atOrSet("identifier"), info.identifier);
	if (cfg.atOrDef("remapPaths").asOrDef<bool>(false)) cfg.atOrSet("destination").setStr(info.destination.str());

	convertHelper.registerTexture(info, result, cfgPath);

	return true;
}

static bool convertImage(ConversionHelper& state, const akfs::Path& cfgPath, akd::PValue& cfg) {
	auto root = akfs::Path(cfgPath).pop_back();
	auto srcPath = root/akfs::Path(cfg.at("source").getStr());

	auto info = state.findConversionInfo(
		srcPath.filename() + ".akres",
		akd::tryDeserialize<akd::SUID>(cfg.atOrDef("identifier")),
		srcPath,
		cfg.atOrDef("destination").tryAs<std::string>(),
		state.getDestinationSuggestionFor(root, cfg.at("source").getStr())
	);

	akd::serialize(cfg.atOrSet("identifier"), info.identifier);
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

	state.registerImage(info, srcPath, srcPath);
	return true;
}
