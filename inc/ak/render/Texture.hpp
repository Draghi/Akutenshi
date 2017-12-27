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

#ifndef AK_RENDER_TEXTURE_HPP_
#define AK_RENDER_TEXTURE_HPP_

#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>

namespace akr {

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
		Tex1D_Array,

		Tex2D,
		Tex2D_Array,

		Tex3D,

		TexCubemap,
	};

	enum class TexFormat {
		R,
		RG,

		RGB,
		RGBA,

		BGR,
		BGRA,
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
		S,
		T,
		R
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

	void setActiveTextureUnit(uint32 unit);
	void bind(uint32 unit, const Texture& texture);

	void setTextureFilters(TexTarget target, FilterType minFilter, FilterType magFilter);
	void setTextureFilters(TexTarget target, FilterType minFilter, FilterType minMipFilter, FilterType magFilter);
	void setTextureClamping(TexTarget target, ClampDir clampDir, ClampType clampType);
	void setTextureBorder(TexTarget target, akm::Vec4 colour);

	void generateMipmaps(TexTarget target);

	void setAnisotropy(TexTarget target, fpSingle amount);
	fpSingle maxAnsiotropy();

	void createTextureStorage1D(TexFormat format, int32 width, int32 mipLevels);
	void createTextureStorage1D(TexFormat format, int32 width, int32 layers, int32 mipLevels);
	void createTextureStorage2D(TexFormat format, int32 width, int32 height, int32 mipLevels);
	void createTextureStorage2D(TexFormat format, int32 width, int32 height, int32 layers, int32 mipLevels);
	void createTextureStorage3D(TexFormat format, int32 width, int32 height, int32 depth,  int32 mipLevels);
	void createTextureStorageCube(TexFormat format, int32 width, int32 height, int32 mipLevels);

	void replaceTextureData1D(TexFormat format, int32 xOff, int32 width, const fpSingle* data, int32 level = 0);
	void replaceTextureData1D(TexFormat format, int32 xOff, int32 lOff, int32 width, int32 layers, const fpSingle* data, int32 level = 0);
	void replaceTextureData2D(TexFormat format, int32 xOff, int32 yOff, int32 width, int32 height, const fpSingle* data, int32 level = 0);
	void replaceTextureData2D(TexFormat format, int32 xOff, int32 yOff, int32 lOff,  int32 width, int32 height, int32 layers, const fpSingle* data, int32 level = 0);
	void replaceTextureData3D(TexFormat format, int32 xOff, int32 yOff, int32 zOff,  int32 width, int32 height, int32 depth,  const fpSingle* data, int32 level = 0);
	void replaceTextureDataCubemap(CubemapTarget cubemap, TexFormat format, int32 xOff, int32 yOff, int32 width, int32 height, const fpSingle* data, int32 level = 0);

}

#endif
