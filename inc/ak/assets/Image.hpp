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
#ifndef AK_ASSETS_IMAGE_HPP_
#define AK_ASSETS_IMAGE_HPP_

#include <stb_image.h>
#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/Path.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/gl/Textures.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/data/SmartEnum.hpp>

namespace akas {

	AK_SMART_ENUM_CLASS_KV(ImageRotation,
		None, 0,
		Right, 3,
		Upsidedown, 2,
		Left, 1
	)

	class Image {
		private:
			std::unique_ptr<uint8[]> m_data;
			akr::gl::TexFormat m_format;
			akr::gl::TexStorage m_storage;
			akSize m_width, m_height, m_depth;

		public:
			Image(uint8* data, akr::gl::TexFormat format, akr::gl::TexStorage storage, akSize width, akSize height, akSize depth) : m_data(), m_format(format), m_storage(storage), m_width(width), m_height(height), m_depth(depth) {
				m_data.reset(new uint8[dataSize()]);
				std::memcpy(m_data.get(), data, dataSize());
			}

			const uint8* data() const { return m_data.get(); }

			akr::gl::TexFormat format() const { return m_format; }
			akr::gl::TexStorage storage() const { return m_storage; }

			akSize width() const { return m_width; }
			akSize height() const { return m_height; }
			akSize depth() const { return m_depth; }

			akSize pixelSize() const { return pixelSize(m_format, m_storage); }
			akSize  dataSize() const { return pixelSize() * m_width * m_height * m_depth; }

			static akSize pixelSize(akr::gl::TexFormat format, akr::gl::TexStorage storage) {
				switch(storage) {
					case akr::gl::TexStorage::Byte:      return 1 * static_cast<akSize>(format);
					case akr::gl::TexStorage::Byte_sRGB: return 1 * static_cast<akSize>(format);
					case akr::gl::TexStorage::Short:     return 2 * static_cast<akSize>(format);
					case akr::gl::TexStorage::Half:      return 2 * static_cast<akSize>(format);
					case akr::gl::TexStorage::Single:    return 4 * static_cast<akSize>(format);
				}
			}
	};

	inline std::pair<void*, ak::ScopeGuard> loadImageDataFromFile(const akfs::Path& path, akr::gl::TexFormat format, akr::gl::TexStorage storage, int& w, int& h) {
		akfs::CFile imageFile(path); if (!imageFile) return {};
		std::vector<uint8> fileData; if (!imageFile.readAll(fileData)) return {};

		stbi_set_flip_vertically_on_load(true);

		switch(storage) {
			case akr::gl::TexStorage::Byte: [[fallthrough]];
			case akr::gl::TexStorage::Byte_sRGB: {
				uint8* imageData  = stbi_load_from_memory(fileData.data(), fileData.size(), &w, &h, nullptr, static_cast<int>(format));
				return std::make_pair<void*, ak::ScopeGuard>(imageData, ak::ScopeGuard([=](){if (imageData) stbi_image_free(imageData);}));
			} break;

			case akr::gl::TexStorage::Short: {
				uint16* imageData  = stbi_load_16_from_memory(fileData.data(), fileData.size(), &w, &h, nullptr, static_cast<int>(format));
				return std::make_pair<void*, ak::ScopeGuard>(imageData, ak::ScopeGuard([=](){if (imageData) stbi_image_free(imageData);}));
			} break;

			case akr::gl::TexStorage::Half: {
				throw std::logic_error("Loading half precision images is not yet supported.");
			} break;

			case akr::gl::TexStorage::Single: {
				fpSingle* imageData  = stbi_loadf_from_memory(fileData.data(), fileData.size(), &w, &h, nullptr, static_cast<int>(format));
				return std::make_pair<void*, ak::ScopeGuard>(imageData, ak::ScopeGuard([=](){if (imageData) stbi_image_free(imageData);}));
			} break;
		}
	}

	inline std::unique_ptr<uint8[]> transformImageData(const void* data, akSize pixelSize, akSize w, akSize h, ImageRotation rotate, uint32 offsetX, uint32 offsetY, uint32 cropX, uint32 cropY, bool flipX, bool flipY) {
		const uint8* dataPtr = static_cast<const uint8*>(data);

		if (cropX == 0) cropX = w;
		if (cropY == 0) cropY = h;

		offsetX = offsetX % w;
		offsetY = offsetY % h;

		auto getSourcePosition = [&](akSize x, akSize y){
			return std::make_pair((offsetX + x) % w, (offsetY + y) % h);
		};

		auto getDstPosition = [&](akSize x, akSize y){
			if (flipX) x = cropX - (x + 1);
			if (flipY) y = cropY - (y + 1);
			switch(rotate) {
				case ImageRotation::None:       return std::make_pair(x % cropX,                                                       y % cropY);
				case ImageRotation::Left:       return std::make_pair(y % cropX,                                                       (static_cast<akSSize>(cropY) - static_cast<akSSize>(x + 1)) % cropY);
				case ImageRotation::Upsidedown: return std::make_pair((static_cast<akSSize>(cropY) - static_cast<akSSize>(x)) % cropX, (static_cast<akSSize>(cropY) - static_cast<akSSize>(y + 1)) % cropY);
				case ImageRotation::Right:      return std::make_pair((static_cast<akSSize>(cropX) - static_cast<akSSize>(y)) % cropX, x % cropY);
				default: throw std::logic_error("Unhandled rotation");
			}
		};

		std::unique_ptr<uint8[]> croppedData(new uint8[pixelSize*cropX*cropY]);
		for(akSize y = 0; y < cropY; y++) {
			for(akSize x = 0; x < cropX; x++) {
				auto src = getSourcePosition(x, y);
				auto dst = getDstPosition(x, y);
				std::memcpy(croppedData.get() + (dst.second*cropX+dst.first)*pixelSize, dataPtr + (src.second*w+src.first)*pixelSize, pixelSize);
			}
		}

		return croppedData;
	}

	inline std::optional<Image> loadImageAndTransform(const akfs::Path& path, akr::gl::TexFormat format, akr::gl::TexStorage storage, ImageRotation rotate, uint32 offsetX, uint32 offsetY, uint32 cropX, uint32 cropY, bool flipX, bool flipY) {
		int w, h;
		auto imageData = loadImageDataFromFile(path, format, storage, w, h);
		if (!imageData.first) return {};

		if (cropX == 0) cropX = w;
		if (cropY == 0) cropY = h;

		auto croppedData = transformImageData(imageData.first, Image::pixelSize(format, storage), w, h, rotate, offsetX, offsetY, cropX, cropY, flipX, flipY);
		return Image(croppedData.get(), format, storage, cropX, cropY, 1);
	}
}

AK_SMART_ENUM_SERIALIZE(akas, ImageRotation)

#endif /* AK_ASSETS_IMAGE_HPP_ */
