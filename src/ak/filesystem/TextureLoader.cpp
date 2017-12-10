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

#include <ak/data/JsonParser.hpp>
#include <ak/data/PValue.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/Log.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <glm/detail/func_common.hpp>
#include <glm/detail/type_vec4.hpp>
#include <GL/gl.h>
#include <GL/glext.h>
#include "ak/filesystem/TextureLoader.hpp"
#include <cmath>
#include <experimental/filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include "stb_image.h"

using namespace ak::filesystem;

static void load2DTexture(const stx::filesystem::path& path, const ak::data::PValue& resourceCfg, ak::render::Texture& tex);

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

void ak::filesystem::loadTexture(SystemFolder folder, const stx::filesystem::path& path, ak::render::Texture& tex) {
	auto baseDir = resolveFolder(folder).value()/path.parent_path();

	// Load Config
	auto cfgFile = open(folder, path, OpenFlags::In);
	if (!cfgFile) throw std::runtime_error("Failed to open texture resource config");
	std::string cfgFileContents;
	if (!cfgFile.readLine(cfgFileContents, false, {})) throw std::runtime_error("Failed to read texture resource config");

	// Parse Config
	ak::data::PValue resourceCfg;
	std::istringstream cfgFileStream(cfgFileContents);
	if (!ak::data::deserializeJson(resourceCfg, cfgFileStream)) throw std::runtime_error("Failed to parse texture resource config");

	// Load Texture
	akr::TexTarget glTarget = akr::TexTarget::Tex1D;
	std::string typeStr = resourceCfg["type"].as<std::string>();
	if (typeStr == "1D") {
		throw std::runtime_error("Not implemented");
	} else if (typeStr == "2D") {
		glTarget = akr::TexTarget::Tex2D;
		load2DTexture(baseDir, resourceCfg, tex);
	}

	// Apply Filter
	auto mipFilter = resourceCfg["mipFilter"].asOrDef<std::string>("none");
	auto minFilter = strToFilterType(resourceCfg["minFilter"].asOrDef<std::string>("nearest"));
	auto magFilter = strToFilterType(resourceCfg["magFilter"].asOrDef<std::string>("nearest"));
	if (mipFilter != "none") {
		akr::setTextureFilters(glTarget, minFilter, strToFilterType(mipFilter), magFilter);
		akr::generateMipmaps(glTarget);
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
	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, akm::min(fLargest, resourceCfg["anisotropy"].asOrDef<fpSingle>(0)));
}

static void load2DTexture(const stx::filesystem::path& path, const ak::data::PValue& resourceCfg, ak::render::Texture& tex) {
	std::string filename = path.string() + "/" + resourceCfg["filename"].as<std::string>();

	int w, h, comp;

	stbi_set_flip_vertically_on_load(true);
	fpSingle* image = stbi_loadf(filename.c_str(), &w, &h, &comp, 0);

	if (!image) throw std::runtime_error("Failed to load image");

	akr::TexFormat format;
	switch(comp) {
		default:
		case 0: throw std::runtime_error("Invalid image");
		case 1: format = akr::TexFormat::R;    break;
		case 2: format = akr::TexFormat::RG;   break;
		case 3: format = akr::TexFormat::RGB;  break;
		case 4: format = akr::TexFormat::RGBA; break;
	}

	int32 mipmapLevels = static_cast<int32>(std::floor(std::log2(akm::max(w,h)))+1);
	if (resourceCfg["mipFilter"].asOrDef<std::string>("none") == "none") mipmapLevels = 1;

	akr::bind(0, tex);
	akr::createTextureStorage2D(format, w, h, mipmapLevels);
	akr::replaceTextureData2D(format, 0, 0, w, h, image, 0);
	stbi_image_free(image);
}
