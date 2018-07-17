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
static void writeImages(ConversionHelper& state);

static bool convertTexture(ConversionHelper& state, const akfs::Path& root, akd::PValue& cfg);

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
	{"TEXTURE", &convertTexture}
}};

void akas::convertDirectory(const akfs::Path& dir) {

	aku::Timer convertTimer;

	akSize     fileCount = 0;
	akSize proccessCount = 0;
	akSize     meshCount = 0;
	akSize materialCount = 0;
	akSize    imageCount = 0;

	akfs::iterateDirectory(dir, [&](const akfs::Path& path, bool isDir){
		if ((isDir) || (path.extension() != ".akconv")) return true;

		ConversionHelper convertHelper(akfs::Path("data/"), true);

		akd::PValue convData = akd::fromJsonFile(path);
		auto iter = proccessFunctions.find(convData["type"].asStr());
		if (iter == proccessFunctions.end()) { akl::Logger("Convert").warn("Could not proccess type '",  convData["type"].asStr(), "' in file: ", path.str()); return true; }

		if (iter->second(convertHelper, akfs::Path(path).pop_back(), convData)) proccessCount++;

		akl::Logger("Convert").test_warn(akd::toJsonFile(convData, path), "Could not resave conversion file: ", path.str());

		writeMeshes(convertHelper);
		writeMaterials(convertHelper);
		writeImages(convertHelper);

		fileCount     += 1;
		meshCount     += convertHelper.meshCount();
		materialCount += convertHelper.materialCount();
		imageCount    += convertHelper.imageCount();

		return true;
	}, true);

	akl::Logger("Convert").info("Converted ", proccessCount, " out of ", fileCount, " resources in ", convertTimer.mark().msecs() , "ms. Resulting in ", meshCount, " meshes, ", materialCount, " materials, ", imageCount, " images.");
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

static void writeImages(ConversionHelper& state) {
	for(auto& image : state.images()) {
		if (!writeAssetMetaFile(image.first.destination, image.first.identifier, akas::AssetType::Image, image.first.displayName, image.second)) continue;
		if (!akfs::copy(image.second, akfs::Path(image.first.destination).clearExtension())) continue;
	}
}

static bool convertTexture(ConversionHelper& /*convertHelper*/, const akfs::Path& /*root*/, akd::PValue& /*cfg*/) {
	return true;
}
