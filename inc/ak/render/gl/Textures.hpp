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

#ifndef AK_RENDER_TEXTURES_HPP_
#define AK_RENDER_TEXTURES_HPP_

#include <ak/data/Image.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/gl/Types.hpp>
#include <optional>

namespace akr {
	namespace gl {

		enum class CubemapTarget {
			PosX,
			PosY,
			PosZ,
			NegX,
			NegY,
			NegZ,
		};

		enum class TexTarget {
			Tex1D,
			Tex2D,
			Tex3D,
			Tex1D_Array,
			Tex2D_Array,
			TexCubemap,
		};

		enum class TexFormat {
			R,
			RG,
			RGB,
			RGBA
		};

		enum class TexStorage {
			Byte,
			Byte_sRGB,
			Half,
			Single,
		};

		enum class FilterType {
			Nearest,
			Linear,
		};

		enum class ClampType {
			Repeat,
			Mirror,
			Edge,
			Border,
		};

		enum class ClampDir {
			Horz,
			Vert,
			Depth
		};

		class Texture final {
			Texture(const Texture& other) = delete;
			Texture& operator=(const Texture& other) = delete;
			protected:
				uint32 m_id;
				TexTarget m_type;

			public:
				Texture();
				Texture(TexTarget target);
				Texture(Texture&& other);
				~Texture();

				uint32 id() const { return m_id; }
				TexTarget type() const { return m_type; }
				bool isValid() { return m_id == 0; }

				Texture& operator=(Texture&& other);
		};

		std::optional<Texture> createTex1D(uint32 unit, TexFormat format, TexStorage storageType, DataType dataType, const void* data, akSize width, akSize maxMipmapLevels = std::numeric_limits<akSize>::max());
		std::optional<Texture> createTex2D(uint32 unit, TexFormat format, TexStorage storageType, DataType dataType, const void* data, akSize width, akSize height, akSize maxMipmapLevels = std::numeric_limits<akSize>::max());
		std::optional<Texture> createTex3D(uint32 unit, TexFormat format, TexStorage storageType, DataType dataType, const void* data, akSize width, akSize height, akSize depth, akSize maxMipmapLevels = std::numeric_limits<akSize>::max());
		std::optional<Texture> createTexCubemap(uint32 unit, TexFormat format, TexStorage storageType, DataType dataType, const void* data, akSize width, akSize height, akSize maxMipmapLevels = std::numeric_limits<akSize>::max());
		std::optional<Texture> createTex1DArray(uint32 unit, TexFormat format, TexStorage storageType, DataType dataType, const void* data, akSize width, akSize layers, akSize maxMipmapLevels = std::numeric_limits<akSize>::max());
		std::optional<Texture> createTex2DArray(uint32 unit, TexFormat format, TexStorage storageType, DataType dataType, const void* data, akSize width, akSize height, akSize layers, akSize maxMipmapLevels = std::numeric_limits<akSize>::max());


		void setActiveTexUnit(uint32 unit);
		void bindTexture(uint32 unit, const Texture& texture);

		void newTexStorage1D(TexFormat format, TexStorage storage, akSize width, akSize mipLevels);
		void newTexStorage2D(TexFormat format, TexStorage storage, akSize width, akSize height, akSize mipLevels);
		void newTexStorage3D(TexFormat format, TexStorage storage, akSize width, akSize height, akSize depth, akSize mipLevels);
		void newTexStorageCubemap(TexFormat format, TexStorage storage, akSize width, akSize height, akSize mipLevels);
		void newTexStorage1DArray(TexFormat format, TexStorage storage, akSize width, akSize layers, akSize mipLevels);
		void newTexStorage2DArray(TexFormat format, TexStorage storage, akSize width, akSize height, akSize layers, akSize mipLevels);

		void loadTexData1D(akSize miplevel, TexFormat format, DataType dataType, const void* data, akSize width, akSize xOff = 0);
		void loadTexData2D(akSize miplevel, TexFormat format, DataType dataType, const void* data, akSize width, akSize height, akSize xOff = 0, akSize yOff = 0);
		void loadTexData3D(akSize miplevel, TexFormat format, DataType dataType, const void* data, akSize width, akSize height, akSize depth, akSize xOff = 0, akSize yOff = 0, akSize zOff = 0);
		void loadTexDataCubemap(CubemapTarget cubemap, akSize miplevel, TexFormat format, DataType dataType, const void* data, akSize width, akSize height, akSize xOff = 0, akSize yOff = 0);
		void loadTexData1DArray(akSize miplevel, TexFormat format, DataType dataType, const void* data, akSize width, akSize layer, akSize layers = 0, akSize xOff = 0);
		void loadTexData2DArray(akSize miplevel, TexFormat format, DataType dataType, const void* data, akSize width, akSize height, akSize layer, akSize layers = 0, akSize xOff = 0, akSize yOff = 0);

		void setTexAnisotropy(TexTarget target, fpSingle amount);
		void setTexFilters(TexTarget target, FilterType minFilter, FilterType magFilter);
		void setTexFilters(TexTarget target, FilterType minFilter, FilterType minMipFilter, FilterType magFilter);
		void setTexClamping(TexTarget target, ClampDir clampDir, ClampType clampType);
		void setTexBorder(TexTarget target, akm::Vec4 colour);

		void genTexMipmaps(TexTarget target);

		akSize calcTexMaxMipmaps(akSize width, akSize height = 0, akSize depth = 0);

		fpSingle getTexMaxAnsiotropy();
		akSize getTexComponentsFromFormat(TexFormat format);
		TexFormat getTexFormatFromComponents(akSize components);

		template<typename type_t> std::optional<Texture> createTex1D(uint32 unit, TexStorage storageType, const akd::Image<type_t>& image, akSize maxMipmapLevels = std::numeric_limits<akSize>::max()) {
			auto format = getTexFormatFromComponents(image.components());
			auto dataType = getDataTypeOf<type_t>();
			return createTex1D(unit, format, storageType, dataType, image.data(), image.width(), maxMipmapLevels);
		}

		template<typename type_t> std::optional<Texture> createTex2D(uint32 unit, TexStorage storageType, const akd::Image<type_t>& image, akSize maxMipmapLevels = std::numeric_limits<akSize>::max()) {
			auto format = getTexFormatFromComponents(image.components());
			auto dataType = getDataTypeOf<type_t>();
			return createTex2D(unit, format, storageType, dataType, image.data(), image.width(), image.height(), maxMipmapLevels);
		}

		template<typename type_t> std::optional<Texture> createTex3D(uint32 unit, TexStorage storageType, const akd::Image<type_t>& image, akSize maxMipmapLevels = std::numeric_limits<akSize>::max()) {
			auto format = getTexFormatFromComponents(image.components());
			auto dataType = getDataTypeOf<type_t>();
			return createTex3D(unit, format, storageType, dataType, image.data(), image.width(), image.height(), image.depth(), maxMipmapLevels);
		}

		template<typename type_t> std::optional<Texture> createTexCubemap(uint32 unit, TexStorage storageType, const akd::Image<type_t>& image, akSize maxMipmapLevels = std::numeric_limits<akSize>::max()) {
			auto format = getTexFormatFromComponents(image.components());
			auto dataType = getDataTypeOf<type_t>();
			return createTexCubemap(unit, format, storageType, dataType, image.data(), image.width(), image.height(), maxMipmapLevels);
		}

		template<typename type_t> std::optional<Texture> createTex1DArray(uint32 unit, TexStorage storageType, const akd::Image<type_t>& image, akSize maxMipmapLevels = std::numeric_limits<akSize>::max()) {
			auto format = getTexFormatFromComponents(image.components());
			auto dataType = getDataTypeOf<type_t>();
			return createTex1DArray(unit, format, storageType, dataType, image.data(), image.width(), image.height(), maxMipmapLevels);
		}

		template<typename type_t> std::optional<Texture> createTex2DArray(uint32 unit, TexStorage storageType, const akd::Image<type_t>& image, akSize maxMipmapLevels = std::numeric_limits<akSize>::max()) {
			auto format = getTexFormatFromComponents(image.components());
			auto dataType = getDataTypeOf<type_t>();
			return createTex2DArray(unit, format, storageType, dataType, image.data(), image.width(), image.height(), image.depth(), maxMipmapLevels);
		}
	}
}

#endif
