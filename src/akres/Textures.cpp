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

#include "akres/Textures.hpp"

#include <ak/animation/Mesh.hpp>
#include <ak/animation/Skeleton.hpp>
#include <ak/data/Brotli.hpp>
#include <ak/data/Image.hpp>
#include <ak/data/Json.hpp>
#include <ak/data/MsgPack.hpp>
#include <ak/data/PValue.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/math/Serialize.hpp>
#include <ak/math/Transform.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/window/WindowOptions.hpp>
#include <deque>
#include <experimental/filesystem>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

using namespace akres;

template<typename type_t> static akd::Image<type_t> load1DTexture(const stx::filesystem::path& srcPath, const akd::PValue& config);
template<typename type_t> static akd::Image<type_t> load2DTexture(const stx::filesystem::path& srcPath, const akd::PValue& config);
template<typename type_t> static akd::Image<type_t> load3DTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& cfgName, const akd::PValue& config);
template<typename type_t> static akd::Image<type_t> load1DArrayTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& cfgName, const akd::PValue& config);
template<typename type_t> static akd::Image<type_t> load2DArrayTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& cfgName, const akd::PValue& config);
template<typename type_t> static akd::Image<type_t> loadCubemapTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& cfgName, const akd::PValue& config);

bool akres::doPackTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& outPath, const std::string& cfgName) {

	// Open Config
	akfs::CFile cfgFile(srcPath/cfgName, akfs::OpenFlags::In);
	if (!cfgFile) {
		std::cout << "Failed to open '" << (srcPath/cfgName).string() << "' ensure that the file exists and that you have permission to access it." << std::endl;
		return false;
	}

	// Open output
	auto outFilename = (outPath/cfgName).replace_extension(".aktex");
	auto outputFile = akfs::CFile(outFilename, akfs::OpenFlags::Out | akfs::OpenFlags::Truncate);
	if (!outputFile) {
		std::cout << "Could not create output file as: " << outFilename.string() << std::endl;
		return false;
	} else {
		std::cout << "Opened '" << outFilename.string() << "' for pack output." << std::endl;
	}

	// Load and parse config
	std::cout << "Reading texture data..." << std::endl;
	std::string cfgData;
	if (!cfgFile.readLine(cfgData, false, {})) {
		std::cout << "Failed to read data from '" << (srcPath/cfgName).string() << "' ensure that the file exists and that you have permission to access it." << std::endl;
		return false;
	}

	akd::PValue config;
	if (!akd::fromJson(config, cfgData)) {
		std::cout << "Failed to parse json in file '" << (srcPath/cfgName).string() << "' please ensure that it conforms to the JSON standard." << std::endl;
		return false;
	}

	// Process Data
	std::cout << "Processing config data..." << std::endl;
	akd::PValue outStruct;
	outStruct["type"] = config["type"];
	outStruct["mipFilter"] = config["mipFilter"];
	outStruct["minFilter"] = config["minFilter"];
	outStruct["magFilter"] = config["magFilter"];
	outStruct["wrap"]["border"] = config["wrap"]["border"];
	outStruct["wrap"]["s"] = config["wrap"]["s"];
	outStruct["wrap"]["t"] = config["wrap"]["t"];
	outStruct["wrap"]["r"] = config["wrap"]["r"];
	outStruct["anisotropy"] = config["anisotropy"];
	outStruct["hdr"].set<bool>(config["hdr"].asOrDef(false));

	std::cout << "Processing texture data..." << std::endl;
	std::string texType = config["type"].asOrDef<std::string>("Unset");

	// @HACK Dirty hack for templated lambda. Param is only for type. Other option is to break this out into a template function or into a cascade of if statements (ew).
	auto loadTexFunc = [&](auto a) {
		if (texType == "1D") return load1DTexture<decltype(a)>(srcPath, config);
		else if (texType == "2D") return load2DTexture<decltype(a)>(srcPath, config);
		else if (texType == "3D") return load3DTexture<decltype(a)>(srcPath, cfgName, config);
		else if (texType == "1D_array") return load1DArrayTexture<decltype(a)>(srcPath, cfgName, config);
		else if (texType == "2D_array") return load2DArrayTexture<decltype(a)>(srcPath, cfgName, config);
		else if (texType == "cubemap") return loadCubemapTexture<decltype(a)>(srcPath, cfgName, config);
		else {
			std::stringstream sstream;
			sstream << "Invalid texture type '" << texType << "'. Please ensure that the texture config conforms to the Akutenshi texture standard.";
			throw std::runtime_error(sstream.str());
		}
	};

	std::cout << "Serializing texture data..." << std::endl;
	if (config["hdr"].asOrDef<bool>(false)) {
		akd::serialize(loadTexFunc(static_cast<fpSingle>(0)), outStruct["image"]);
	} else {
		akd::serialize(loadTexFunc(static_cast<uint8>(0)), outStruct["image"]);
	}

	// Reserialize
	std::cout << "Converting texture data to disk format..." << std::endl;
	auto data = akd::toMsgPack(outStruct);

	// Compress
	std::cout << "Compressing texture data..." << std::endl;
	data = akd::compressBrotli(data, config["compressionLevel"].asOrDef<uint8>(8));

	// Write
	std::cout << "Writing texture data..." << std::endl;
	outputFile.write(data.data(), data.size());

	std::cout << "Done." << std::endl;

	return true;
}

static std::vector<uint8> loadImageFile(const stx::filesystem::path& filename);
static std::pair<std::deque<std::vector<uint8>>, std::vector<std::pair<const uint8*, akSize>>> loadImageFiles(const std::vector<stx::filesystem::path>& filenames);

template<typename type_t> static akd::Image<type_t> load1DTexture(const stx::filesystem::path& srcPath, const akd::PValue& config) {
	auto filename = srcPath/config["filename"].asStr();
	auto fileData = loadImageFile(filename);

	akSize row = config["row"].asOrDef<akSize>(0);
	auto result = akd::loadImage<type_t>(fileData.data(), fileData.size());
	if (result) return akd::Image<type_t>(result->row(row), result->components(), result->width(), 1, 1);

	std::stringstream sstream;
	sstream << "Could not parse as image file: " << filename.string();
	throw std::runtime_error(sstream.str());
}

template<typename type_t> static akd::Image<type_t> load2DTexture(const stx::filesystem::path& srcPath, const akd::PValue& config) {
	auto filename = srcPath/config["filename"].asStr();
	auto fileData = loadImageFile(filename);
	auto result = akd::loadImage<type_t>(fileData.data(), fileData.size());
	if (result) return *result;

	std::stringstream sstream;
	sstream << "Could not parse as image file: " << filename.string();
	throw std::runtime_error(sstream.str());
}

template<typename type_t> static akd::Image<type_t> load3DTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& cfgName, const akd::PValue& config) {
	std::vector<stx::filesystem::path> filenames;
	filenames.reserve(config["layers"].size());

	for(auto iter = config["layers"].asArr().begin(); iter != config["layers"].asArr().end(); iter++) {
		filenames.push_back(srcPath/iter->asStr());
	}

	auto fileData = loadImageFiles(filenames);
	auto result = akd::loadImage<type_t>(fileData.second);
	if (result) return *result;

	std::stringstream sstream;
	sstream << "Could not parse as image file: " << (srcPath/cfgName).string();
	throw std::runtime_error(sstream.str());
}

template<typename type_t> static akd::Image<type_t> load1DArrayTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& cfgName, const akd::PValue& config) {
	std::vector<stx::filesystem::path> filenames;
	std::vector<akSize> layers;
	filenames.reserve(config["layers"].size());
	layers.reserve(config["layers"].size());

	for(auto iter = config["layers"].asArr().begin(); iter != config["layers"].asArr().end(); iter++) {
		filenames.push_back(srcPath/((*iter)["filename"].asStr()));
		layers.push_back((*iter)["row"].as<akSize>());
	}

	auto fileData = loadImageFiles(filenames);
	auto result = akd::loadImage<type_t>(fileData.second);
	if (!result) {
		std::stringstream sstream;
		sstream << "Could not parse as image file: " << (srcPath/cfgName).string();
		throw std::runtime_error(sstream.str());
	}

	std::vector<type_t> processedData;
	processedData.resize(result->width()*result->depth()*result->components());
	for(akSize i = 0; i < result->depth(); i++) {
		std::memcpy(processedData.data() + i*result->rowSize(), result->layer(i), result->rowSize()*sizeof(type_t));
	}

	return akd::Image<type_t>(processedData.data(), result->components(), result->width(), result->depth(), 1);
}

template<typename type_t> static akd::Image<type_t> load2DArrayTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& cfgName, const akd::PValue& config) {
	std::vector<stx::filesystem::path> filenames;
	filenames.reserve(config["layers"].size());

	for(auto iter = config["layers"].asArr().begin(); iter != config["layers"].asArr().end(); iter++) {
		filenames.push_back(srcPath/iter->asStr());
	}

	auto fileData = loadImageFiles(filenames);
	auto result = akd::loadImage<type_t>(fileData.second);
	if (result) return *result;

	std::stringstream sstream;
	sstream << "Could not parse as image file: " << (srcPath/cfgName).string();
	throw std::runtime_error(sstream.str());
}

template<typename type_t> static akd::Image<type_t> loadCubemapTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& cfgName, const akd::PValue& config) {
	auto fileData = loadImageFiles({
		srcPath/config["layers"]["pX"].asStr(),
		srcPath/config["layers"]["pY"].asStr(),
		srcPath/config["layers"]["pZ"].asStr(),
		srcPath/config["layers"]["nX"].asStr(),
		srcPath/config["layers"]["nY"].asStr(),
		srcPath/config["layers"]["nZ"].asStr()
	});

	auto result = akd::loadImage<type_t>(fileData.second, false);
	if (result) return *result;

	std::stringstream sstream;
	sstream << "Could not parse as image file: " << (srcPath/cfgName).string();
	throw std::runtime_error(sstream.str());
}

static std::vector<uint8> loadImageFile(const stx::filesystem::path& filename) {
	auto imgFile = akfs::open(filename, akfs::OpenFlags::In);
	if (!imgFile) { std::stringstream sstream; sstream << "Could not open image file: " << filename; throw std::runtime_error(sstream.str()); }

	std::vector<uint8> fileData;
	if (!imgFile.readAll(fileData)) { std::stringstream sstream; sstream << "Could not read image file: " << filename; throw std::runtime_error(sstream.str()); }

	return fileData;
}

static std::pair<std::deque<std::vector<uint8>>, std::vector<std::pair<const uint8*, akSize>>> loadImageFiles(const std::vector<stx::filesystem::path>& filenames) {
	std::pair<std::deque<std::vector<uint8>>, std::vector<std::pair<const uint8*, akSize>>> fileData;
	for(auto iter = filenames.begin(); iter != filenames.end(); iter++) {
		fileData.first.push_back(loadImageFile(*iter));
		fileData.second.push_back(std::make_pair(fileData.first.back().data(), fileData.first.back().size()));
	}

	return fileData;
}


