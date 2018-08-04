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
#include <ak/assets/ShaderProgram.hpp>
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
static void writeShaderStages(ConversionHelper& state);
static void writeShaderPrograms(ConversionHelper& state);
static void writeAnimations(ConversionHelper& state);
static void writeTextures(ConversionHelper& state);

template<auto func_f> static bool convertCopyOnly(const std::string& assetTypeName, ConversionHelper& state, const akfs::Path& cfgPath, akd::PValue& cfg);

static bool convertTexture(ConversionHelper& state, const akfs::Path& cfgPath, akd::PValue& cfg);
static bool convertShaderProgram(ConversionHelper& conversionHelper, const akfs::Path& cfgPath, akd::PValue& cfg);
static bool convertImage(ConversionHelper& state, const akfs::Path& cfgPath, akd::PValue& cfg) { return convertCopyOnly<&akas::ConversionHelper::registerImage>("image", state, cfgPath, cfg); }
static bool convertShaderStage(ConversionHelper& state, const akfs::Path& cfgPath, akd::PValue& cfg) { return convertCopyOnly<&akas::ConversionHelper::registerShaderStage>("shader stage", state, cfgPath, cfg); }

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
	{"Image", &convertImage},
	{"ShaderStage", &convertShaderStage},
	{"ShaderProgram", &convertShaderProgram}
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
	akSize animationCount = 0;
	akSize shaderStageCount = 0;
	akSize shaderProgramCount = 0;

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
			writeAnimations(convertHelper);
			writeShaderStages(convertHelper);
			writeShaderPrograms(convertHelper);
			writeTextures(convertHelper);

			meshCount     += convertHelper.meshCount();
			materialCount += convertHelper.materialCount();
			imageCount    += convertHelper.imageCount();
			animationCount += convertHelper.animationCount();
			shaderStageCount   += convertHelper.shaderStageCount();
			shaderProgramCount += convertHelper.shaderProgramCount();
			textureCount  += convertHelper.textureCount();
		}
	}

	akl::Logger("Convert").info("Converted ", proccessCount, " out of ", fileCount, " assets in ", convertTimer.markAndReset().msecs() , "ms.");
	akl::Logger("Convert").info("Created ", meshCount, " meshes, ", materialCount, " materials, ", textureCount, " textures, ", animationCount, " animations, ", shaderStageCount, " shader stages, ", shaderProgramCount, " shader programs and ", imageCount, " images.");
	akl::Logger("Convert").info("Modified ", modifiedCount, " *.akconv files.");
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
	for(auto& copy : state.images()) {
		if (!writeAssetMetaFile(copy.first.destination, copy.first.identifier, akas::AssetType::Image, copy.first.displayName, copy.second)) continue;
		if (!akfs::copy(copy.second, akfs::Path(copy.first.destination).clearExtension())) continue;
	}
}

static void writeShaderStages(ConversionHelper& state) {
	for(auto& copy : state.shaderStages()) {
		if (!writeAssetMetaFile(copy.first.destination, copy.first.identifier, akas::AssetType::ShaderStage, copy.first.displayName, copy.second)) continue;
		if (!akfs::copy(copy.second, akfs::Path(copy.first.destination).clearExtension())) continue;
	}
}

static void writeShaderPrograms(ConversionHelper& state) {
	for(auto& shader : state.shaderPrograms()) {
		if (!writeAssetMetaFile(shader.first.destination, shader.first.identifier, akas::AssetType::ShaderProgram, shader.first.displayName, {})) continue;
		if (!writeAssetFile(akfs::Path(shader.first.destination).clearExtension(), shader.second, true)) continue;
	}
}

static void writeAnimations(ConversionHelper& state) {
	for(auto& animation : state.animations()) {
		if (!writeAssetMetaFile(animation.first.destination, animation.first.identifier, akas::AssetType::Animation, animation.first.displayName, {})) continue;
		if (!writeAssetFile(akfs::Path(animation.first.destination).clearExtension(), animation.second, false)) continue;
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

	if (akfs::exists(info.destination) && akfs::exists(akfs::Path(info.destination).clearExtension())) {
		auto srcModifiedTime = akfs::modifiedTime(cfgPath);
		auto dstModifiedTime = akfs::modifiedTime(akfs::Path(info.destination).clearExtension());
		if (dstModifiedTime >= srcModifiedTime) return false;
	}

	akd::serialize(cfg.atOrSet("identifier"), info.identifier);
	if (cfg.atOrDef("remapPaths").asOrDef<bool>(false)) cfg.atOrSet("destination").setStr(info.destination.str());

	convertHelper.registerTexture(info, result, cfgPath);

	return true;
}


static bool convertShaderProgram(ConversionHelper& conversionHelper, const akfs::Path& cfgPath, akd::PValue& cfg) {
	auto root = akfs::Path(cfgPath).pop_back();

	auto& shaderCfg = cfg["config"];
	auto& stagesCfg = shaderCfg["stages"].getObj();
	for(auto stageCfg = stagesCfg.begin(); stageCfg != stagesCfg.end(); stageCfg++) {
		const auto shaderPath = root/akd::deserialize<akfs::Path>(stageCfg->second.at("source"));

		const auto tryGetIdentifier = [&]{
			auto imgConv = akd::fromJsonFile(shaderPath);
			if (!imgConv.exists("identifier")) {
				akl::Logger("ConvShaderProgram").warn("Could not find source shader stage file identifier: ", shaderPath.str());
				return false;
			}
			stageCfg->second.atOrSet("identifier") = imgConv["identifier"];
			return true;
		};

		if ((!stageCfg->second.exists("identifier")) && (!tryGetIdentifier())) return false;
		akd::SUID id = akd::tryDeserialize<akd::SUID>(stageCfg->second.at("identifier")).value_or(akd::SUID());
		if (!conversionHelper.tryFindAssetInfoByID(id).has_value()) {
			if (!tryGetIdentifier()) return false;
			id = akd::tryDeserialize<akd::SUID>(stageCfg->second.at("identifier")).value_or(akd::SUID());
		}

		akl::Logger("ConvTex").test_warn(conversionHelper.tryFindAssetInfoByID(id).has_value(), "Could not locate referenced file, the shader may not load correctly.");
	}

	auto result = akd::deserialize<akas::ShaderProgram>(shaderCfg);
	auto info = conversionHelper.findConversionInfo(
		akfs::Path(cfgPath).clearExtension().filename(),
		akd::tryDeserialize<akd::SUID>(cfg.atOrDef("identifier")),
		cfgPath,
		cfg.atOrDef("destination").tryAs<std::string>(),
		conversionHelper.getDestinationSuggestionFor(root, akfs::Path(cfgPath).clearExtension().filename())
	);

	if (akfs::exists(info.destination) && akfs::exists(akfs::Path(info.destination).clearExtension())) {
		auto srcModifiedTime = akfs::modifiedTime(cfgPath);
		auto dstModifiedTime = akfs::modifiedTime(akfs::Path(info.destination).clearExtension());
		if (dstModifiedTime >= srcModifiedTime) return false;
	}

	akd::serialize(cfg.atOrSet("identifier"), info.identifier);
	if (cfg.atOrDef("remapPaths").asOrDef<bool>(false)) cfg.atOrSet("destination").setStr(info.destination.str());

	conversionHelper.registerShaderProgram(info, result, cfgPath);

	return true;
}

template<auto func_f> static bool convertCopyOnly(const std::string& assetTypeName, ConversionHelper& state, const akfs::Path& cfgPath, akd::PValue& cfg) {
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
		akl::Logger("Image").warn("Missing ", assetTypeName, " asset: ", srcPath.str());
		return false;
	}

	if (akfs::exists(info.destination) && akfs::exists(akfs::Path(info.destination).clearExtension())) {
		auto srcModifiedTime = akfs::modifiedTime(srcPath);
		auto dstModifiedTime = akfs::modifiedTime(akfs::Path(info.destination).clearExtension());
		if (dstModifiedTime >= srcModifiedTime) return false;
	}

	(state.*func_f)(info, srcPath, srcPath);
	return true;
}
