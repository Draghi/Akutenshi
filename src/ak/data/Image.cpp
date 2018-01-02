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
#include <ak/ScopeGuard.hpp>
#include <ak/Traits.hpp>
#include <stddef.h>
#include <optional>
#include <stdexcept>
#include <utility>

#include "stb_image.h"

template<typename type_t, auto load_f> static std::optional<akd::Image<type_t>> loadImageInternal(const uint8* data, akSize len, bool bottomUp) {
	stbi_set_flip_vertically_on_load(bottomUp);
	stbi_hdr_to_ldr_gamma(2.2f);
	stbi_ldr_to_hdr_gamma(1.0f);

	int w, h, comp;
	auto* imgData = load_f(data, static_cast<int>(len), &w, &h, &comp, 0);
	if (!imgData) throw std::runtime_error("Failed to parse image file.");
	auto imgGuard = ak::ScopeGuard([&]{stbi_image_free(imgData);});

	return akd::Image<type_t>(imgData, static_cast<akSize>(comp), static_cast<akSize>(w), static_cast<akSize>(h), 0);
}

std::optional<akd::ImageF32> akd::loadImageF32(const uint8* data, akSize len, bool bottomUp) {
	return loadImageInternal<fpSingle, stbi_loadf_from_memory>(data, len, bottomUp);
}

std::optional<akd::ImageF32> akd::loadImageF32(const std::vector<std::pair<const uint8*, akSize>>& data, bool bottomUp) {
	std::vector<akd::ImageF32> layers;
	layers.reserve(data.size());

	akSize width = 0, height = 0, comp = 0;
	for(size_t i = 0; i < data.size(); i++) {
		auto image = loadImageInternal<fpSingle, stbi_loadf_from_memory>(data[i].first, data[i].second, bottomUp);
		if (!image) return std::optional<akd::ImageF32>();
		if (width == 0) { width = image->width(); height = image->height(); comp = akm::max(comp, image->components()); }
		layers.push_back(*image);
	}

	size_t layerSize = width*height*comp;
	std::vector<fpSingle> imageData;
	imageData.resize(layerSize*layers.size());
	for(size_t i = 0; i < layers.size(); i++) {
		layers[i].setComponents(comp);
		std::memcpy(imageData.data() + i*layerSize, layers[i].data(), layerSize*sizeof(fpSingle));
	}

	return akd::ImageF32(imageData.data(), comp, width, height, static_cast<akSize>(layers.size()));
}

std::optional<akd::ImageU8> akd::loadImageU8(const uint8* data, akSize len, bool bottomUp) {
	return loadImageInternal<uint8, stbi_load_from_memory>(data, len, bottomUp);
}

std::optional<akd::ImageU8> akd::loadImageU8(const std::vector<std::pair<const uint8*, akSize>>& data, bool bottomUp) {
	std::vector<akd::ImageU8> layers;
	layers.reserve(data.size());

	akSize width = 0, height = 0, comp = 0;
	for(size_t i = 0; i < data.size(); i++) {
		auto image = loadImageInternal<uint8, stbi_load_from_memory>(data[i].first, data[i].second, bottomUp);
		if (!image) return std::optional<akd::ImageU8>();
		if (width == 0) { width = image->width(); height = image->height(); comp = akm::max(comp, image->components()); }
		layers.push_back(*image);
	}

	size_t layerSize = width*height*comp;
	std::vector<uint8> imageData;
	imageData.resize(layerSize*layers.size());
	for(size_t i = 0; i < layers.size(); i++) {
		layers[i].setComponents(comp);
		std::memcpy(imageData.data() + i*layerSize, layers[i].data(), layerSize*sizeof(uint8));
	}

	return akd::ImageU8(imageData.data(), comp, width, height, static_cast<akSize>(layers.size()));
}



