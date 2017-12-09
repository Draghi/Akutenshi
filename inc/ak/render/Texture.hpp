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

#include <ak/PrimitiveTypes.hpp>

namespace ak {
	namespace render {

		enum class CubeMapTarget {
			Right,
			Left,
			Top,
			Bottom,
			Back,
			Front,
		};

		enum class TexTarget {
			Tex1D,
			Tex1D_Array,

			Tex2D,
			Tex2D_Array,

			Tex3D,

			TexCubeMap,
		};

		enum class TexFormat {
			R,
			RG,

			RGB,
			RGBA,

			BGR,
			BGRA,
		};

		class Texture {
			protected:
				uint32 m_id;
				TexTarget m_type;

			public:
				Texture(TexTarget target);
				~Texture();

				uint32 id() const { return m_id; }
				TexTarget type() const { return m_type; }
		};

		void setActiveTextureUnit(uint32 unit);
		void bind(uint32 unit, const Texture& texture);

		void createTextureStorage1D(TexFormat format, uint32 width, uint32 mipLevels);
		void createTextureStorage1D(TexFormat format, uint32 width, uint32 layers, uint32 mipLevels);
		void createTextureStorage2D(TexFormat format, uint32 width, uint32 height, uint32 mipLevels);
		void createTextureStorage2D(TexFormat format, uint32 width, uint32 height, uint32 layers, uint32 mipLevels);
		void createTextureStorage3D(TexFormat format, uint32 width, uint32 height, uint32 depth, uint32 mipLevels);

		void setTextureData1D(TexFormat format, uint32 width, const fpSingle* data, uint32 level = 0);
		void setTextureData1D(TexFormat format, uint32 width, uint32 layers, const fpSingle* data, uint32 level = 0);
		void setTextureData2D(TexFormat format, uint32 width, uint32 height, const fpSingle* data, uint32 level = 0);
		void setTextureData2D(TexFormat format, uint32 width, uint32 height, uint32 layers, const fpSingle* data, uint32 level = 0);
		void setTextureData3D(TexFormat format, uint32 width, uint32 height, uint32 depth, const fpSingle* data, uint32 level = 0);

		void replaceTextureData1D(TexFormat format, uint32 xOff, uint32 width, const fpSingle* data, uint32 level = 0);
		void replaceTextureData1D(TexFormat format, uint32 xOff, uint32 lOff, uint32 width, uint32 layers, const fpSingle* data, uint32 level = 0);
		void replaceTextureData2D(TexFormat format, uint32 xOff, uint32 yOff, uint32 width, uint32 height, const fpSingle* data, uint32 level = 0);
		void replaceTextureData2D(TexFormat format, uint32 xOff, uint32 yOff, uint32 lOff, uint32 width, uint32 height, uint32 layers, const fpSingle* data, uint32 level = 0);
		void replaceTextureData3D(TexFormat format, uint32 xOff, uint32 yOff, uint32 zOff, uint32 width, uint32 height, uint32 depth, const fpSingle* data, uint32 level = 0);

	}
}

namespace akr = ak::render;

#endif
