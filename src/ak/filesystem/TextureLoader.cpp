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

#include <ak/data/Image.hpp>
#include <ak/data/JsonParser.hpp>
#include <ak/data/PValue.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/ImageLoader.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ext/type_traits.h>
#include <ak/math/Scalar.hpp>
#include "ak/filesystem/TextureLoader.hpp"
#include <cmath>
#include <deque>
#include <experimental/filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <GL/gl4.h>


using namespace akfs;

static akr::TexFormat componentsToFormat(int32 comp) {
	switch(comp) {
		default:
		case 0: throw std::runtime_error("Invalid image");
		case 1: return akr::TexFormat::R;
		case 2: return akr::TexFormat::RG;
		case 3: return akr::TexFormat::RGB;
		case 4: return akr::TexFormat::RGBA;
	}
}

static int32 calcMipmapLevels(fpSingle w, fpSingle h = 0, fpSingle d = 0) {
	return static_cast<int32>(std::floor(std::log2(akm::max(akm::max(w, h), d)))+1);
}

static void load1DTexture(const stx::filesystem::path& path, const akd::PValue& resourceCfg, akr::Texture& tex);
static void load1DArrayTexture(const stx::filesystem::path& path, const akd::PValue& resourceCfg, akr::Texture& tex);
static void load2DTexture(const stx::filesystem::path& path, const akd::PValue& resourceCfg, akr::Texture& tex);
static void load2DArrayTexture(const stx::filesystem::path& path, const akd::PValue& resourceCfg, akr::Texture& tex);
static void load3DTexture(const stx::filesystem::path& path, const akd::PValue& resourceCfg, akr::Texture& tex);
static void loadCubemapTexture(const stx::filesystem::path& path, const akd::PValue& resourceCfg, akr::Texture& tex);

static akr::FilterType strToFilterType(const std::string& val) {
	if (val == "nearest") return akr::FilterType::Nearest;
	else if (val == "linear") return akr::FilterType::Linear;
	throw std::runtime_error("Bad value");
}

static akr::ClampType strToClampType(const std::string& val) {
	if (val == "repeat") return akr::ClampType::Repeat;
	else if (val == "mirror") return akr::ClampType::Mirror;
	else if (val == "edge") return akr::ClampType::Edge;
	else if (val == "border") return akr::ClampType::Border;
	throw std::runtime_error("Bad value");
}

akr::Texture akfs::loadTexture(SystemFolder folder, const stx::filesystem::path& path) {
	akr::Texture tex;
	auto baseDir = resolveFolder(folder).value()/path.parent_path();

	// Load Config
	auto cfgFile = open(folder, path, OpenFlags::In);
	if (!cfgFile) throw std::runtime_error("Failed to open texture resource config");
	std::string cfgFileContents;
	if (!cfgFile.readLine(cfgFileContents, false, {})) throw std::runtime_error("Failed to read texture resource config");

	// Parse Config
	akd::PValue resourceCfg;
	std::istringstream cfgFileStream(cfgFileContents);
	if (!akd::deserializeJson(resourceCfg, cfgFileStream)) throw std::runtime_error("Failed to parse texture resource config");

	// Load Texture
	akr::TexTarget glTarget = akr::TexTarget::Tex1D;
	std::string typeStr = resourceCfg["type"].as<std::string>();
	if (typeStr == "1D") {
		glTarget = akr::TexTarget::Tex1D;
		load1DTexture(baseDir, resourceCfg, tex);
	} else if (typeStr == "1D_array") {
		glTarget = akr::TexTarget::Tex1D_Array;
		load1DArrayTexture(baseDir, resourceCfg, tex);
	} else if (typeStr == "2D") {
		glTarget = akr::TexTarget::Tex2D;
		load2DTexture(baseDir, resourceCfg, tex);
	} else if (typeStr == "2D_array") {
		glTarget = akr::TexTarget::Tex2D_Array;
		load2DArrayTexture(baseDir, resourceCfg, tex);
	} else if (typeStr == "3D") {
		glTarget = akr::TexTarget::Tex3D;
		load3DTexture(baseDir, resourceCfg, tex);
	} else if (typeStr == "Cubemap") {
		glTarget = akr::TexTarget::TexCubemap;
		loadCubemapTexture(baseDir, resourceCfg, tex);
	} else {
		throw std::runtime_error("Unsupported texture type");
	}

	// Apply Filter
	auto mipFilter = resourceCfg["mipFilter"].asOrDef<std::string>("none");
	auto minFilter = strToFilterType(resourceCfg["minFilter"].asOrDef<std::string>("nearest"));
	auto magFilter = strToFilterType(resourceCfg["magFilter"].asOrDef<std::string>("nearest"));
	if (mipFilter != "none") {
		akr::generateMipmaps(glTarget);
		akr::setTextureFilters(glTarget, minFilter, strToFilterType(mipFilter), magFilter);
	} else {
		akr::setTextureFilters(glTarget, minFilter, magFilter);
	}

	// Apply Wrap
	auto sWrap = strToClampType(resourceCfg["wrap"]["s"].asOrDef<std::string>("repeat"));
	auto tWrap = strToClampType(resourceCfg["wrap"]["t"].asOrDef<std::string>("repeat"));
	auto rWrap = strToClampType(resourceCfg["wrap"]["r"].asOrDef<std::string>("repeat"));

	akr::setTextureClamping(glTarget, akr::ClampDir::S, sWrap);
	akr::setTextureClamping(glTarget, akr::ClampDir::T, tWrap);
	akr::setTextureClamping(glTarget, akr::ClampDir::R, rWrap);

	// Apply Border Colour
	auto red   = resourceCfg["wrap"]["border"][0].asOrDef<fpSingle>(0);
	auto green = resourceCfg["wrap"]["border"][1].asOrDef<fpSingle>(0);
	auto blue  = resourceCfg["wrap"]["border"][2].asOrDef<fpSingle>(0);
	auto alpha = resourceCfg["wrap"]["border"][3].asOrDef<fpSingle>(0);
	akr::setTextureBorder(glTarget, akm::Vec4(red, green, blue, alpha));

	// anisotropy
	if (glTarget != akr::TexTarget::Tex3D) akr::setAnisotropy(glTarget, resourceCfg["anisotropy"].asOrDef<fpSingle>(0));

	return tex;
}

static void load1DTexture(const stx::filesystem::path& path, const akd::PValue& resourceCfg, akr::Texture& tex) {
	std::string filename = path.string() + "/" + resourceCfg["filename"].as<std::string>();

	auto img = akfs::load1DImage(akfs::SystemFolder::none, filename, resourceCfg["row"].as<uint32>());

	akr::TexFormat format = componentsToFormat(static_cast<int32>(img.componentCount()));
	int32 mipmapLevels = (resourceCfg["mipFilter"].asOrDef<std::string>("none") == "none") ? 1 : calcMipmapLevels(img.width(), img.height(), img.depth());

	tex = akr::Texture(akr::TexTarget::Tex1D);
	akr::bind(0, tex);
	akr::createTextureStorage1D(format, static_cast<int32>(img.width()), mipmapLevels);
	akr::replaceTextureData1D(format, 0, static_cast<int32>(img.width()), img.data(), 0);
}

static void load1DArrayTexture(const stx::filesystem::path& path, const akd::PValue& resourceCfg, akr::Texture& tex) {
	std::vector<std::pair<stx::filesystem::path, uint32>> layers;
	std::map<stx::filesystem::path, akd::Image2D> images;

	int32 width = 0, comp = 1;

	auto layerArr = resourceCfg["layers"].as<akd::PValue::arr_t>();
	for(auto iter = layerArr.begin(); iter != layerArr.end(); iter++) {
		layers.push_back(std::make_pair(
			path/(*iter)["filename"].as<std::string>(),
			(*iter)["row"].asOrDef<uint32>(0)
		));

		auto& layer = layers.back();

		auto imgIter = images.find(layer.first);
		if (imgIter != images.end()) {
			if (imgIter->second.height() <= layer.second) throw std::out_of_range("load1DArrayTexture: Image row out of range");
			continue;
		}

		auto image = akfs::load2DImage(akfs::SystemFolder::none, layer.first);
		if (width == 0u) { width = static_cast<int32>(image.width()); comp = static_cast<int32>(image.componentCount()); }
		if (width != static_cast<int32>(image.width())) throw std::runtime_error("load1DArrayTexture: Mismatched image width");
		if (image.height() <= layer.second) throw std::out_of_range("load1DArrayTexture: Image row out of range");
		if (comp != static_cast<int32>(image.componentCount())) image.setComponentCount(static_cast<size_t>(comp));

		images.insert(std::make_pair(
			layer.first,
			std::move(image)
		));
	}

	akr::TexFormat format = componentsToFormat(comp);
	int32 mipmapLevels = (resourceCfg["mipFilter"].asOrDef<std::string>("none") == "none") ? 1 : calcMipmapLevels(width);

	akr::Texture tmpTex(akr::TexTarget::Tex1D_Array);
	akr::bind(0, tmpTex);
	akr::createTextureStorage1D(format, width, static_cast<int32>(layers.size()), mipmapLevels);

	for(auto i = 0u; i != layers.size(); i++) {
		auto& layer = layers[i];
		auto& image = images.at(layer.first);

		akr::replaceTextureData1D(format, 0, static_cast<int32>(i), static_cast<int32>(width), 1, image.data(), 0);
	}

	tex = std::move(tmpTex);
}

static void load2DTexture(const stx::filesystem::path& path, const akd::PValue& resourceCfg, akr::Texture& tex) {
	std::string filename = path.string() + "/" + resourceCfg["filename"].as<std::string>();

	auto img = akfs::load2DImage(akfs::SystemFolder::none, filename);

	akr::TexFormat format = componentsToFormat(static_cast<int32>(img.componentCount()));
	int32 mipmapLevels = (resourceCfg["mipFilter"].asOrDef<std::string>("none") == "none") ? 1 : calcMipmapLevels(img.width(), img.height(), img.depth());

	tex = akr::Texture(akr::TexTarget::Tex2D);
	akr::bind(0, tex);
	akr::createTextureStorage2D(format, static_cast<int32>(img.width()), static_cast<int32>(img.height()), mipmapLevels);
	akr::replaceTextureData2D(format, 0, 0, static_cast<int32>(img.width()), static_cast<int32>(img.height()), img.data(), 0);
}

static void load2DArrayTexture(const stx::filesystem::path& path, const akd::PValue& resourceCfg, akr::Texture& tex) {
	std::vector<stx::filesystem::path> layers;
	std::map<stx::filesystem::path, akd::Image2D> images;

	int32 width = 0, height = 0, comp = 1;

	auto layerArr = resourceCfg["layers"].as<akd::PValue::arr_t>();
	for(auto iter = layerArr.begin(); iter != layerArr.end(); iter++) {
		layers.push_back(path/(*iter).as<std::string>());

		auto& layer = layers.back();

		auto imgIter = images.find(layer);
		if (imgIter != images.end()) continue;

		auto image = akfs::load2DImage(akfs::SystemFolder::none, layer);

		if (width == 0u) {
			width = static_cast<int32>(image.width());
			height = static_cast<int32>(image.width());
			comp = static_cast<int32>(image.componentCount());
		}

		if ((width != static_cast<int32>(image.width())) || (height != static_cast<int32>(image.height()))) throw std::runtime_error("load1DArrayTexture: Mismatched image size");
		if (comp != static_cast<int32>(image.componentCount())) image.setComponentCount(static_cast<size_t>(comp));

		images.insert(std::make_pair(
			layer,
			std::move(image)
		));
	}

	akr::TexFormat format = componentsToFormat(comp);
	int32 mipmapLevels = (resourceCfg["mipFilter"].asOrDef<std::string>("none") == "none") ? 1 : calcMipmapLevels(width, height);

	akr::Texture tmpTex(akr::TexTarget::Tex2D_Array);
	akr::bind(0, tmpTex);
	akr::createTextureStorage2D(format, width, height, static_cast<int32>(layers.size()), mipmapLevels);

	for(auto i = 0u; i != layers.size(); i++) {
		auto& layer = layers[i];
		auto& image = images.at(layer);

		akr::replaceTextureData2D(format, 0, 0, static_cast<int32>(i), width, height, 1, image.data(), 0);
	}
	tex = std::move(tmpTex);
}

static void load3DTexture(const stx::filesystem::path& path, const akd::PValue& resourceCfg, akr::Texture& tex) {

	std::vector<stx::filesystem::path> filenames;
	auto filenameArr = resourceCfg["layers"].as<akd::PValue::arr_t>();
	for(auto iter = filenameArr.begin(); iter != filenameArr.end(); iter++) filenames.push_back(path/iter->as<std::string>());

	auto img = akfs::load3DImage(akfs::SystemFolder::none, filenames);

	akr::TexFormat format = componentsToFormat(static_cast<int32>(img.componentCount()));
	int32 mipmapLevels = (resourceCfg["mipFilter"].asOrDef<std::string>("none") == "none") ? 1 : calcMipmapLevels(img.width(), img.height(), img.depth());

	tex = akr::Texture(akr::TexTarget::Tex3D);
	akr::bind(0, tex);
	akr::createTextureStorage3D(format, static_cast<int32>(img.width()), static_cast<int32>(img.height()), static_cast<int32>(img.depth()), mipmapLevels);
	akr::replaceTextureData3D(format, 0, 0, 0, static_cast<int32>(img.width()), static_cast<int32>(img.height()), static_cast<int32>(img.depth()), img.data(), 0);
}

static void loadCubemapTexture(const stx::filesystem::path& path, const akd::PValue& resourceCfg, akr::Texture& tex) {
	throw std::logic_error("Not implemented");
}
