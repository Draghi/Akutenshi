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

#include <ak/filesystem/ImageLoader.hpp>
#include <ak/Log.hpp>
#include <stddef.h>
#include <cstring>
#include <experimental/filesystem>
#include <optional>
#include <stdexcept>

#include "stb_image.h"

using namespace akfs;

akd::Image1D akfs::load1DImage(SystemFolder folder, const stx::filesystem::path& path, uint32 layer, bool bottomUp) {
	auto fullpath = akfs::resolveFolder(folder).value()/path;

	stbi_set_flip_vertically_on_load(bottomUp);

	akl::Logger("load1D").info(fullpath.c_str());

	int w, h, comp;
	auto* image = stbi_loadf(fullpath.c_str(), &w, &h, &comp, 0);

	if (!image) throw std::runtime_error("Load1DImage: Failed to load image");
	if (layer >= static_cast<uint>(h)) throw std::out_of_range("Load1DImage: Out of range");
	akd::Image1D img(image + static_cast<uint32>(w*comp)*layer, static_cast<size_t>(comp), static_cast<size_t>(w));

	stbi_image_free(image);
	return img;
}

akd::Image2D akfs::load2DImage(SystemFolder folder, const stx::filesystem::path& path, bool bottomUp) {
	auto fullpath = akfs::resolveFolder(folder).value()/path;

	stbi_set_flip_vertically_on_load(bottomUp);

	int w, h, comp;
	auto* image = stbi_loadf(fullpath.c_str(), &w, &h, &comp, 0);

	if (!image) throw std::runtime_error("Load2DImage: Failed to load image (" + fullpath.string() + ")");
	akd::Image2D img(image, static_cast<size_t>(comp), static_cast<size_t>(w), static_cast<size_t>(h));

	stbi_image_free(image);
	return img;
}

akd::Image3D akfs::load3DImage(SystemFolder folder, const std::vector<stx::filesystem::path>& path, bool bottomUp) {
	std::vector<akd::Image2D> layers;

	size_t w = 0, h = 0, comp = 0;

	for(auto i = 0u; i < path.size(); i++) {
		layers.push_back(load2DImage(folder, path[i], bottomUp));
		auto& img = layers.back();

		if (w == 0) { w = img.width(); h = img.height(); comp = img.componentCount(); }
		if ((w != img.width()) || (h != img.height())) throw std::runtime_error("load3DImage: bad image size");

		if (comp < img.componentCount()) comp = img.componentCount();
	}

	size_t layerSize = static_cast<size_t>(w*h*comp);

	std::vector<fpSingle> img3D;
	img3D.resize(layerSize*layers.size());
	for(auto i = 0u; i < layers.size(); i++) {
		layers[i].setComponentCount(comp);
		std::memcpy(img3D.data() + i*layerSize , layers[i].data(), layerSize*sizeof(fpSingle));
	}

	return akd::Image3D(img3D.data(), comp, w, h, layers.size());
}



