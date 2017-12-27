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
#include <ak/math/Scalar.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <stddef.h>
#include "ak/filesystem/TextureLoader.hpp"
#include <algorithm>
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

using namespace akfs;

static akr::TexFormat componentsToFormat(size_t comp) {
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

static void applyConfigSettings(akr::TexTarget glTarget, const akd::PValue& resourceCfg) {
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
	if ((glTarget != akr::TexTarget::Tex2D_Array) && (glTarget != akr::TexTarget::Tex3D)) {
		akr::setAnisotropy(glTarget, resourceCfg["anisotropy"].asOrDef<fpSingle>(0));
	}
}

static stx::filesystem::path loadResourceConfig(akd::PValue& dest, SystemFolder folder, const stx::filesystem::path& path) {
	// Load Config
	auto cfgFile = open(folder, path, OpenFlags::In);
	if (!cfgFile) throw std::runtime_error("Failed to open texture resource config");
	std::string cfgFileContents;
	if (!cfgFile.readLine(cfgFileContents, false, {})) throw std::runtime_error("Failed to read texture resource config");

	// Parse Config
	std::istringstream cfgFileStream(cfgFileContents);
	if (!akd::deserializeJson(dest, cfgFileStream)) throw std::runtime_error("Failed to parse texture resource config");

	return resolveFolder(folder).value()/path.parent_path();
}

TexLoadResult akfs::loadTexture1D(SystemFolder folder, const stx::filesystem::path& cfgPath) {
	akd::PValue resourceCfg;
	auto baseDir = loadResourceConfig(resourceCfg, folder, cfgPath);
	std::string imageFilename = baseDir/resourceCfg["filename"].as<std::string>();

	// Load image
	auto img = akfs::load1DImage(akfs::SystemFolder::none, imageFilename, resourceCfg["row"].as<uint32>());

	akr::TexFormat format = componentsToFormat(img.componentCount());
	int32 mipmapLevels = (resourceCfg["mipFilter"].asOrDef<std::string>("none") == "none") ? 1 : calcMipmapLevels(img.width(), img.height(), img.depth());

	TexLoadResult result{
		akr::Texture(akr::TexTarget::Tex1D),
		img.width(), 0, 0, 1,
		img.componentCount()
	};

	akr::bind(0, result.texture);
	akr::createTextureStorage1D(format, static_cast<int32>(img.width()), mipmapLevels);
	akr::replaceTextureData1D(format, 0, static_cast<int32>(img.width()), img.data(), 0);

	applyConfigSettings(akr::TexTarget::Tex1D, resourceCfg);

	return result;
}

TexLoadResult akfs::loadTextureArray1D(SystemFolder folder, const stx::filesystem::path& cfgPath) {
	akd::PValue resourceCfg;
	auto baseDir = loadResourceConfig(resourceCfg, folder, cfgPath);

	std::vector<std::pair<stx::filesystem::path, uint32>> layers;
	std::map<stx::filesystem::path, akd::Image2D> images;

	size_t width = 0, comp = 1;

	auto layerArr = resourceCfg["layers"].as<akd::PValue::arr_t>();
	for(auto iter = layerArr.begin(); iter != layerArr.end(); iter++) {
		layers.push_back(std::make_pair(
			baseDir/(*iter)["filename"].as<std::string>(),
			(*iter)["row"].asOrDef<uint32>(0)
		));

		auto& layer = layers.back();

		auto imgIter = images.find(layer.first);
		if (imgIter != images.end()) {
			if (imgIter->second.height() <= layer.second) throw std::out_of_range("load1DArrayTexture: Image row out of range");
			continue;
		}

		auto image = akfs::load2DImage(akfs::SystemFolder::none, layer.first);
		if (width == 0u) { width = image.width(); comp = image.componentCount(); }
		if (width !=image.width()) throw std::runtime_error("load1DArrayTexture: Mismatched image width");
		if (image.height() <= layer.second) throw std::out_of_range("load1DArrayTexture: Image row out of range");
		if (comp < image.componentCount()) comp = image.componentCount();

		images.insert(std::make_pair(
			layer.first,
			std::move(image)
		));
	}

	akr::TexFormat format = componentsToFormat(comp);
	int32 mipmapLevels = (resourceCfg["mipFilter"].asOrDef<std::string>("none") == "none") ? 1 : calcMipmapLevels(width);

	TexLoadResult result{
		akr::Texture(akr::TexTarget::Tex1D_Array),
		width, 0, 0, layers.size(),
		comp
	};

	akr::bind(0, result.texture);
	akr::createTextureStorage1D(format, static_cast<int32>(width), static_cast<int32>(layers.size()), mipmapLevels);

	for(auto i = 0u; i != layers.size(); i++) {
		auto& layer = layers[i];
		auto& image = images.at(layer.first);

		image.setComponentCount(comp);
		akr::replaceTextureData1D(format, 0, static_cast<int32>(i), static_cast<int32>(width), 1, image.data(), 0);
	}

	applyConfigSettings(akr::TexTarget::Tex1D_Array, resourceCfg);

	return result;
}

TexLoadResult akfs::loadTexture2D(SystemFolder folder, const stx::filesystem::path& cfgPath) {
	akd::PValue resourceCfg;
	auto baseDir = loadResourceConfig(resourceCfg, folder, cfgPath);
	std::string imageFilename = baseDir/(resourceCfg["filename"].as<std::string>());

	auto img = akfs::load2DImage(akfs::SystemFolder::none, imageFilename);

	akr::TexFormat format = componentsToFormat(img.componentCount());
	int32 mipmapLevels = (resourceCfg["mipFilter"].asOrDef<std::string>("none") == "none") ? 1 : calcMipmapLevels(img.width(), img.height(), img.depth());

	TexLoadResult result{
		akr::Texture(akr::TexTarget::Tex2D),
		img.width(), img.height(), 0, 1,
		img.componentCount()
	};

	akr::bind(0, result.texture);
	akr::createTextureStorage2D(format, static_cast<int32>(img.width()), static_cast<int32>(img.height()), mipmapLevels);
	akr::replaceTextureData2D(format, 0, 0, static_cast<int32>(img.width()), static_cast<int32>(img.height()), img.data(), 0);

	applyConfigSettings(akr::TexTarget::Tex2D, resourceCfg);

	return result;
}

TexLoadResult akfs::loadTextureArray2D(SystemFolder folder, const stx::filesystem::path& cfgPath) {
	akd::PValue resourceCfg;
	auto baseDir = loadResourceConfig(resourceCfg, folder, cfgPath);

	std::vector<stx::filesystem::path> layers;
	std::map<stx::filesystem::path, akd::Image2D> images;

	size_t width = 0, height = 0, comp = 1;

	auto layerArr = resourceCfg["layers"].as<akd::PValue::arr_t>();
	for(auto iter = layerArr.begin(); iter != layerArr.end(); iter++) {
		layers.push_back(baseDir/(*iter).as<std::string>());

		auto& layer = layers.back();

		auto imgIter = images.find(layer);
		if (imgIter != images.end()) continue;

		auto image = akfs::load2DImage(akfs::SystemFolder::none, layer);

		if (width == 0u) {
			width = image.width();
			height = image.width();
			comp = image.componentCount();
		}

		if ((width != image.width()) || (height != image.height())) throw std::runtime_error("load1DArrayTexture: Mismatched image size");
		if (comp != image.componentCount()) image.setComponentCount(comp);

		images.insert(std::make_pair(
			layer,
			std::move(image)
		));
	}

	akr::TexFormat format = componentsToFormat(comp);
	int32 mipmapLevels = (resourceCfg["mipFilter"].asOrDef<std::string>("none") == "none") ? 1 : calcMipmapLevels(width, height);

	TexLoadResult result{
		akr::Texture(akr::TexTarget::Tex2D_Array),
		width, height, 0, images.size(),
		comp
	};

	akr::bind(0, result.texture);
	akr::createTextureStorage2D(format, static_cast<int32>(width), static_cast<int32>(height), static_cast<int32>(layers.size()), mipmapLevels);

	for(auto i = 0u; i != layers.size(); i++) {
		auto& layer = layers[i];
		auto& image = images.at(layer);

		akr::replaceTextureData2D(format, 0, 0, static_cast<int32>(i), static_cast<int32>(width), static_cast<int32>(height), 1, image.data(), 0);
	}

	applyConfigSettings(akr::TexTarget::Tex2D_Array, resourceCfg);

	return result;
}

TexLoadResult akfs::loadTexture3D(SystemFolder folder, const stx::filesystem::path& cfgPath) {
	akd::PValue resourceCfg;
	auto baseDir = loadResourceConfig(resourceCfg, folder, cfgPath);

	std::vector<stx::filesystem::path> filenames;
	auto filenameArr = resourceCfg["layers"].as<akd::PValue::arr_t>();
	for(auto iter = filenameArr.begin(); iter != filenameArr.end(); iter++) filenames.push_back(cfgPath/iter->as<std::string>());

	auto img = akfs::load3DImage(akfs::SystemFolder::none, filenames);

	akr::TexFormat format = componentsToFormat(img.componentCount());
	int32 mipmapLevels = (resourceCfg["mipFilter"].asOrDef<std::string>("none") == "none") ? 1 : calcMipmapLevels(img.width(), img.height(), img.depth());

	TexLoadResult result{
		akr::Texture(akr::TexTarget::Tex3D),
		img.width(), img.height(), img.depth(), 1,
		img.componentCount()
	};

	akr::bind(0, result.texture);
	akr::createTextureStorage3D(format, static_cast<int32>(img.width()), static_cast<int32>(img.height()), static_cast<int32>(img.depth()), mipmapLevels);
	akr::replaceTextureData3D(format, 0, 0, 0, static_cast<int32>(img.width()), static_cast<int32>(img.height()), static_cast<int32>(img.depth()), img.data(), 0);

	applyConfigSettings(akr::TexTarget::Tex2D_Array, resourceCfg);

	return result;
}

TexLoadResult akfs::loadTextureCubemap(SystemFolder folder, const stx::filesystem::path& cfgPath) {
	akd::PValue resourceCfg;
	auto baseDir = loadResourceConfig(resourceCfg, folder, cfgPath);
	auto& layers = resourceCfg["layers"];

	auto pXImg = akfs::load2DImage(akfs::SystemFolder::none, baseDir/layers["pX"].as<std::string>(), false);
	size_t maxComp = pXImg.componentCount();
	size_t width = pXImg.width();
	size_t height = pXImg.height();

	auto pYImg = akfs::load2DImage(akfs::SystemFolder::none, baseDir/layers["pY"].as<std::string>(), false);
	if ((width != pYImg.width()) || (height != pYImg.height())) throw std::runtime_error("loadCubemapTexture: Mismatched image size.");
	maxComp = akm::max(maxComp, pYImg.componentCount());

	auto pZImg = akfs::load2DImage(akfs::SystemFolder::none, baseDir/layers["pZ"].as<std::string>(), false);
	if ((width != pZImg.width()) || (height != pZImg.height())) throw std::runtime_error("loadCubemapTexture: Mismatched image size.");
	maxComp = akm::max(maxComp, pZImg.componentCount());

	auto nXImg = akfs::load2DImage(akfs::SystemFolder::none, baseDir/layers["nX"].as<std::string>(), false);
	if ((width != nXImg.width()) || (height != nXImg.height())) throw std::runtime_error("loadCubemapTexture: Mismatched image size.");
	maxComp = akm::max(maxComp, nXImg.componentCount());

	auto nYImg = akfs::load2DImage(akfs::SystemFolder::none, baseDir/layers["nY"].as<std::string>(), false);
	if ((width != nYImg.width()) || (height != nYImg.height())) throw std::runtime_error("loadCubemapTexture: Mismatched image size.");
	maxComp = akm::max(maxComp, nYImg.componentCount());

	auto nZImg = akfs::load2DImage(akfs::SystemFolder::none, baseDir/layers["nZ"].as<std::string>(), false);
	if ((width != nZImg.width()) || (height != nZImg.height())) throw std::runtime_error("loadCubemapTexture: Mismatched image size.");
	maxComp = akm::max(maxComp, nZImg.componentCount());

	pXImg.setComponentCount(maxComp);
	pYImg.setComponentCount(maxComp);
	pZImg.setComponentCount(maxComp);
	pXImg.setComponentCount(maxComp);
	nYImg.setComponentCount(maxComp);
	nZImg.setComponentCount(maxComp);

	akr::TexFormat format = componentsToFormat(pXImg.componentCount());
	int32 mipmapLevels = (resourceCfg["mipFilter"].asOrDef<std::string>("none") == "none") ? 1 : calcMipmapLevels(pXImg.width(), pXImg.height(), pXImg.depth());

	TexLoadResult result{
		akr::Texture(akr::TexTarget::TexCubemap),
		width, height, 0, 0,
		maxComp
	};

	akr::bind(0, result.texture);
	akr::createTextureStorageCube(format, static_cast<int32>(width), static_cast<int32>(height), mipmapLevels);
	akr::replaceTextureDataCubemap(akr::CubemapTarget::PosX, format, 0, 0, static_cast<int32>(width), static_cast<int32>(height), pXImg.data(), 0);
	akr::replaceTextureDataCubemap(akr::CubemapTarget::PosY, format, 0, 0, static_cast<int32>(width), static_cast<int32>(height), pYImg.data(), 0);
	akr::replaceTextureDataCubemap(akr::CubemapTarget::PosZ, format, 0, 0, static_cast<int32>(width), static_cast<int32>(height), pZImg.data(), 0);
	akr::replaceTextureDataCubemap(akr::CubemapTarget::NegX, format, 0, 0, static_cast<int32>(width), static_cast<int32>(height), nXImg.data(), 0);
	akr::replaceTextureDataCubemap(akr::CubemapTarget::NegY, format, 0, 0, static_cast<int32>(width), static_cast<int32>(height), nYImg.data(), 0);
	akr::replaceTextureDataCubemap(akr::CubemapTarget::NegZ, format, 0, 0, static_cast<int32>(width), static_cast<int32>(height), nZImg.data(), 0);

	applyConfigSettings(akr::TexTarget::TexCubemap, resourceCfg);

	return result;
}
