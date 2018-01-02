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

#include <ak/data/Brotli.hpp>
#include <ak/data/Image.hpp>
#include <ak/data/Json.hpp>
#include <ak/data/MsgPack.hpp>
#include <ak/data/PValue.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/Path.hpp>
#include <ak/math/Serialize.hpp>
#include <ak/math/Transform.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/window/WindowOptions.hpp>
#include <deque>
#include <experimental/filesystem>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

int akResourceMain();

static void packTexture();
static void packTextureDirectory();
static bool doPackTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& outPath, const std::string& cfgName);

int akResourceMain() {

	while(true) {
		std::cout << "Welcome to the Akutenshi resource packer. Please select an option:" << std::endl
		          << "1) Pack Texture (Single)" << std::endl
		          << "2) Pack Texture (Directory)" << std::endl
		          << "Q) Quit" << std::endl;

		std::string inStr;
		std::getline(std::cin, inStr);

		if (inStr.length() != 1) {
			std::cout << "Invalid selection." << std::endl << std::endl;
			continue;
		}

		char cmdSel = inStr.front();

		try {
			switch(cmdSel) {
				case '1': {
					packTexture();
					break;
				}

				case '2': {
					packTextureDirectory();
					break;
				}

				case 'Q': [[fallthrough]];
				case 'q': {
					return 0;
				}

				default: {
					std::cout << "Invalid selection." << std::endl << std::endl;
					continue;
				}
			}
		} catch(const std::exception& ex) {
			std::cout << "Exception occured. Operation aborted. Message:" << ex.what() << std::endl;
		} catch(...) {
			std::cout << "Exception occured. Operation aborted. No information." << std::endl;
		}

		std::cout << std::endl;
	}
}


static void packTexture() {
		std::cout << std::endl << "Type filename of texture to pack (enter '/\\' to exit): ";

		std::string filenameIn;
		std::getline(std::cin, filenameIn);
		std::cout << std::endl;

		if (filenameIn == "/\\") return;

		stx::filesystem::path filename(filenameIn);

		try {
			if (!doPackTexture(filename.parent_path(), filename.parent_path(), filename.filename())) {
				std::cout << "Operation failed. Returning to menu." << std::endl;
			}
		} catch(const std::runtime_error& ex) {
			std::cout << "Operation aborted. Returning to menu. Message: " << ex.what() << std::endl;
		}
}

static void packTextureDirectory() {
	bool recursive;
	while(true) {
		std::cout << std::endl << "Recursively? y/n (enter '/\\' to exit): ";

		std::string inLine;
		std::getline(std::cin, inLine);

		if (inLine == "/\\") return;
		if ((inLine == "Y") || (inLine == "y")) { recursive = true; break; }
		else if ((inLine == "N") || (inLine == "n")) { recursive = false; break; }

		std::cout << std::endl << "Please enter 'Y' or 'N'";
	}

	stx::filesystem::path srcDir;
	while(true) {
		std::cout << "Type directory to pack (enter '/\\' to exit): ";

		std::string directoryIn;
		std::getline(std::cin, directoryIn);

		if (directoryIn == "/\\") return;

		srcDir = directoryIn;

		std::error_code e;
		if (stx::filesystem::exists(srcDir, e)) break;

		std::cout << "Directory does not exist: " << srcDir.string() << std::endl;
	}

	stx::filesystem::path dstDir;
	while(true) {
		std::cout << "Type directory to output packed textures (enter '/\\' to exit): ";

		std::string directoryOut;
		std::getline(std::cin, directoryOut);

		if (directoryOut == "/\\") return;

		dstDir = directoryOut;

		std::error_code e;
		if (stx::filesystem::exists(dstDir, e)) break;
		if (stx::filesystem::create_directories(dstDir, e)) break;

		std::cout << "Directory does not exist and could not be created: " << dstDir.string() << std::endl;
	}

	std::cout << std::endl << "Starting directory operation." << std::endl << std::endl;

	auto processDirectory = [&](auto iterator) {
		for(auto& iter : iterator) {
			if (stx::filesystem::is_directory(iter)) continue;
			if (iter.path().extension() == ".json") {
				try {
					std::cout << "Processing: " << iter.path() << std::endl;

					auto pathDiff = akfs::pathDifference(srcDir, iter.path().parent_path());
					if (pathDiff) {
						doPackTexture(srcDir, dstDir/(*pathDiff), iter.path().filename());
					} else {
						doPackTexture(srcDir, dstDir, iter.path().filename());
					}
				} catch(const std::runtime_error& ex) {
					std::cout << "Operation aborted. Attempting to continue. Message: " << ex.what() << std::endl;
				}
				std::cout << std::endl;
			} else {
				std::cout << "Skipped: " << iter.path() << std::endl;
			}
		}
	};

	if (recursive) processDirectory(stx::filesystem::recursive_directory_iterator(srcDir));
	else processDirectory(stx::filesystem::directory_iterator(srcDir));
}

// //////////////////// //
// // Texture Packer // //
// //////////////////// //

// Should we move this elsewhere? Maybe akres (ak/resources?)

template<typename type_t> static akd::Image<type_t> load1DTexture(const stx::filesystem::path& srcPath, const akd::PValue& config);
template<typename type_t> static akd::Image<type_t> load2DTexture(const stx::filesystem::path& srcPath, const akd::PValue& config);
template<typename type_t> static akd::Image<type_t> load3DTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& cfgName, const akd::PValue& config);
template<typename type_t> static akd::Image<type_t> load1DArrayTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& cfgName, const akd::PValue& config);
template<typename type_t> static akd::Image<type_t> load2DArrayTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& cfgName, const akd::PValue& config);
template<typename type_t> static akd::Image<type_t> loadCubemapTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& cfgName, const akd::PValue& config);

static bool doPackTexture(const stx::filesystem::path& srcPath, const stx::filesystem::path& outPath, const std::string& cfgName) {

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

	// @HACK Dirty hack for templated lambda. Param is only for type. Other option is to break this out into a template function or into a cascade of if statements.
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

	if (config["hdr"].asOrDef<bool>(false)) {
		akd::serialize(loadTexFunc(static_cast<fpSingle>(0)), outStruct["image"]);
	} else {
		akd::serialize(loadTexFunc(static_cast<uint8>(0)), outStruct["image"]);
	}

	// Reserialize
	std::cout << "Serializing texture data..." << std::endl;
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
	auto result = akd::loadImage<type_t>(fileData.data(), static_cast<akSize>(fileData.size()));
	if (result) return akd::Image<type_t>(result->row(row), result->components(), result->width(), 1, 1);

	std::stringstream sstream;
	sstream << "Could not parse as image file: " << filename.string();
	throw std::runtime_error(sstream.str());
}

template<typename type_t> static akd::Image<type_t> load2DTexture(const stx::filesystem::path& srcPath, const akd::PValue& config) {
	auto filename = srcPath/config["filename"].asStr();
	auto fileData = loadImageFile(filename);
	auto result = akd::loadImage<type_t>(fileData.data(), static_cast<akSize>(fileData.size()));
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
