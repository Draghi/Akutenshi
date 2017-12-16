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

#include <ak/render/Texture.hpp>
#include <utility>

#include "GL/gl4.h"

using namespace akr;


Texture::Texture(TexTarget target) : m_id(0), m_type(target) {
	switch(m_type) {
		case TexTarget::Tex1D:       glCreateTextures(GL_TEXTURE_1D,       1, &m_id); break;
		case TexTarget::Tex1D_Array: glCreateTextures(GL_TEXTURE_1D_ARRAY, 1, &m_id); break;
		case TexTarget::Tex2D:       glCreateTextures(GL_TEXTURE_2D,       1, &m_id); break;
		case TexTarget::Tex2D_Array: glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_id); break;
		case TexTarget::Tex3D:       glCreateTextures(GL_TEXTURE_3D,       1, &m_id); break;
		case TexTarget::TexCubeMap:  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_id); break;
	}
}

Texture::~Texture() {
	if (m_id != 0) glDeleteTextures(1, &m_id);
}

void akr::setActiveTextureUnit(uint32 unit) {
	glActiveTexture(GL_TEXTURE0 + unit);
}

void akr::bind(uint32 unit, const Texture& texture) {
	glBindTextureUnit(unit, texture.id());
}

void akr::setTextureFilters(TexTarget target, FilterType minFilter, FilterType magFilter) {
	int32 glMinFilter;
	switch(minFilter) {
		case FilterType::Nearest: glMinFilter = GL_NEAREST; break;
		case FilterType::Linear:  glMinFilter = GL_LINEAR;  break;
	}

	int32 glMagFilter;
	switch(magFilter) {
		case FilterType::Nearest: glMagFilter = GL_NEAREST; break;
		case FilterType::Linear:  glMagFilter = GL_LINEAR;  break;
	}

	uint32 glTarget;
	switch(target) {
		case TexTarget::Tex1D: glTarget = GL_TEXTURE_1D; break;
		case TexTarget::Tex1D_Array: glTarget = GL_TEXTURE_1D_ARRAY; break;
		case TexTarget::Tex2D: glTarget = GL_TEXTURE_2D; break;
		case TexTarget::Tex2D_Array: glTarget = GL_TEXTURE_2D_ARRAY; break;
		case TexTarget::Tex3D: glTarget = GL_TEXTURE_3D; break;
		case TexTarget::TexCubeMap: glTarget = GL_TEXTURE_CUBE_MAP; break;
	}

	glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, glMinFilter);
	glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, glMagFilter);
}

void akr::setTextureFilters(TexTarget target, FilterType minFilter, FilterType minMipFilter, FilterType magFilter) {
	int32 glMinFilter;
	switch(minFilter) {
		case FilterType::Nearest: glMinFilter = (minMipFilter == FilterType::Nearest) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR; break;
		case FilterType::Linear:  glMinFilter = (minMipFilter == FilterType::Nearest) ? GL_LINEAR_MIPMAP_NEAREST  : GL_LINEAR_MIPMAP_LINEAR;  break;
	}

	int32 glMagFilter;
	switch(magFilter) {
		case FilterType::Nearest: glMagFilter = GL_NEAREST; break;
		case FilterType::Linear:  glMagFilter = GL_LINEAR; break;
	}

	uint32 glTarget;
	switch(target) {
		case TexTarget::Tex1D:       glTarget = GL_TEXTURE_1D;       break;
		case TexTarget::Tex1D_Array: glTarget = GL_TEXTURE_1D_ARRAY; break;
		case TexTarget::Tex2D:       glTarget = GL_TEXTURE_2D;       break;
		case TexTarget::Tex2D_Array: glTarget = GL_TEXTURE_2D_ARRAY; break;
		case TexTarget::Tex3D:       glTarget = GL_TEXTURE_3D;       break;
		case TexTarget::TexCubeMap:  glTarget = GL_TEXTURE_CUBE_MAP; break;
	}

	glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, glMagFilter);
	glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, glMinFilter);
}

void akr::setTextureClamping(TexTarget target, ClampDir clampDir, ClampType clampType) {
	uint32 glClampDir;
	switch(clampDir) {
		case ClampDir::S: glClampDir = GL_TEXTURE_WRAP_S; break;
		case ClampDir::T: glClampDir = GL_TEXTURE_WRAP_T; break;
		case ClampDir::R: glClampDir = GL_TEXTURE_WRAP_R; break;
	}

	int32 glClampType;
	switch(clampType) {
		case ClampType::Repeat:     glClampType = GL_REPEAT; break;
		case ClampType::Mirror:     glClampType = GL_MIRRORED_REPEAT; break;
		case ClampType::Edge:       glClampType = GL_CLAMP_TO_EDGE; break;
		case ClampType::Border:     glClampType = GL_CLAMP_TO_BORDER; break;
	}

	uint32 glTarget;
	switch(target) {
		case TexTarget::Tex1D: glTarget = GL_TEXTURE_1D; break;
		case TexTarget::Tex1D_Array: glTarget = GL_TEXTURE_1D_ARRAY; break;
		case TexTarget::Tex2D: glTarget = GL_TEXTURE_2D; break;
		case TexTarget::Tex2D_Array: glTarget = GL_TEXTURE_2D_ARRAY; break;
		case TexTarget::Tex3D: glTarget = GL_TEXTURE_3D; break;
		case TexTarget::TexCubeMap: glTarget = GL_TEXTURE_CUBE_MAP; break;
	}

	glTexParameteri(glTarget, glClampDir, glClampType);
}

void akr::setTextureBorder(TexTarget target, akm::Vec4 colour) {
	uint32 glTarget;
	switch(target) {
		case TexTarget::Tex1D: glTarget = GL_TEXTURE_1D; break;
		case TexTarget::Tex1D_Array: glTarget = GL_TEXTURE_1D_ARRAY; break;
		case TexTarget::Tex2D: glTarget = GL_TEXTURE_2D; break;
		case TexTarget::Tex2D_Array: glTarget = GL_TEXTURE_2D_ARRAY; break;
		case TexTarget::Tex3D: glTarget = GL_TEXTURE_3D; break;
		case TexTarget::TexCubeMap: glTarget = GL_TEXTURE_CUBE_MAP; break;
	}
	glTexParameterfv(glTarget, GL_TEXTURE_BORDER_COLOR, &colour[0]);
}

void akr::generateMipmaps(TexTarget target) {
	uint32 glTarget;
	switch(target) {
		case TexTarget::Tex1D: glTarget = GL_TEXTURE_1D; break;
		case TexTarget::Tex1D_Array: glTarget = GL_TEXTURE_1D_ARRAY; break;
		case TexTarget::Tex2D: glTarget = GL_TEXTURE_2D; break;
		case TexTarget::Tex2D_Array: glTarget = GL_TEXTURE_2D_ARRAY; break;
		case TexTarget::Tex3D: glTarget = GL_TEXTURE_3D; break;
		case TexTarget::TexCubeMap: glTarget = GL_TEXTURE_CUBE_MAP; break;
	}
	glGenerateMipmap(glTarget);
}

static std::pair<uint32, uint32> texFormatToGLFormats(TexFormat format) {
	switch(format) {
		case TexFormat::R:    return {GL_R16F,    GL_RED };
		case TexFormat::RG:   return {GL_RG16F,   GL_RG  };
		case TexFormat::RGB:  return {GL_RGB16F,  GL_RGB };
		case TexFormat::RGBA: return {GL_RGBA16F, GL_RGBA};
		case TexFormat::BGR:  return {GL_RGB16F,  GL_BGR };
		case TexFormat::BGRA: return {GL_RGBA16F, GL_BGRA};
	}
}

void akr::createTextureStorage1D(TexFormat format, int32 width, int32 mipLevels) {
	glTexStorage1D(GL_TEXTURE_1D, mipLevels, texFormatToGLFormats(format).first, width);
}

void akr::createTextureStorage1D(TexFormat format, int32 width, int32 layers, int32 mipLevels) {
	glTexStorage2D(GL_TEXTURE_1D_ARRAY, mipLevels, texFormatToGLFormats(format).first, width, layers);
}

void akr::createTextureStorage2D(TexFormat format, int32 width, int32 height, int32 mipLevels) {
	glTexStorage2D(GL_TEXTURE_2D, mipLevels, texFormatToGLFormats(format).first, width, height);
}

void akr::createTextureStorage2D(TexFormat format, int32 width, int32 height, int32 layers, int32 mipLevels) {
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, texFormatToGLFormats(format).first, width, height, layers);
}

void akr::createTextureStorage3D(TexFormat format, int32 width, int32 height, int32 depth, int32 mipLevels) {
	glTexStorage3D(GL_TEXTURE_3D, mipLevels, texFormatToGLFormats(format).first, width, height, depth);
}



void akr::setTextureData1D(TexFormat format, int32 width, const fpSingle* data, int32 level) {
	auto [glFormat, usrFormat] = texFormatToGLFormats(format);
	glTexImage1D(GL_TEXTURE_1D, level, static_cast<int32>(glFormat), width, 0, usrFormat, GL_FLOAT, data);
}

void akr::setTextureData1D(TexFormat format, int32 width, int32 layers, const fpSingle* data, int32 level) {
	auto [glFormat, usrFormat] = texFormatToGLFormats(format);
	glTexImage2D(GL_TEXTURE_1D_ARRAY, level, static_cast<int32>(glFormat), width, layers, 0, usrFormat, GL_FLOAT, data);
}

void akr::setTextureData2D(TexFormat format, int32 width, int32 height, const fpSingle* data, int32 level) {
	auto [glFormat, usrFormat] = texFormatToGLFormats(format);
	glTexImage2D(GL_TEXTURE_2D, level, static_cast<int32>(glFormat), width, height, 0, usrFormat, GL_FLOAT, data);
}

void akr::setTextureData2D(TexFormat format, int32 width, int32 height, int32 layers, const fpSingle* data, int32 level) {
	auto [glFormat, usrFormat] = texFormatToGLFormats(format);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, level, static_cast<int32>(glFormat), width, height, layers, 0, usrFormat, GL_FLOAT, data);
}

void akr::setTextureData3D(TexFormat format, int32 width, int32 height, int32 depth, const fpSingle* data, int32 level) {
	auto [glFormat, usrFormat] = texFormatToGLFormats(format);
	glTexImage3D(GL_TEXTURE_3D, level, static_cast<int32>(glFormat), width, height, depth, 0, usrFormat, GL_FLOAT, data);
}



void akr::replaceTextureData1D(TexFormat format, int32 xOff, int32 width, const fpSingle* data, int32 level) {
	glTexSubImage1D(GL_TEXTURE_1D, level, xOff, width, texFormatToGLFormats(format).second, GL_FLOAT, data);
}

void akr::replaceTextureData1D(TexFormat format, int32 xOff, int32 lOff, int32 width, int32 layers, const fpSingle* data, int32 level) {
	glTexSubImage2D(GL_TEXTURE_1D_ARRAY, level, xOff, lOff, width, layers, texFormatToGLFormats(format).second, GL_FLOAT, data);
}

void akr::replaceTextureData2D(TexFormat format, int32 xOff, int32 yOff, int32 width, int32 height, const fpSingle* data, int32 level) {
	glTexSubImage2D(GL_TEXTURE_2D, level, xOff, yOff, width, height, texFormatToGLFormats(format).second, GL_FLOAT, data);
}

void akr::replaceTextureData2D(TexFormat format, int32 xOff, int32 yOff, int32 lOff, int32 width, int32 height, int32 layers, const fpSingle* data, int32 level) {
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, level, xOff, yOff, lOff, width, height, layers, texFormatToGLFormats(format).second, GL_FLOAT, data);
}

void akr::replaceTextureData3D(TexFormat format, int32 xOff, int32 yOff, int32 zOff, int32 width, int32 height, int32 depth, const fpSingle* data, int32 level) {
	glTexSubImage3D(GL_TEXTURE_3D, level, xOff, yOff, zOff, width, height, depth, texFormatToGLFormats(format).second, GL_FLOAT, data);
}

