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

#include "GL/gl4.h"

using namespace ak::render;


Texture::Texture(TexTarget target) : m_id(0), m_type(target) {
	switch(m_type) {
		case TexTarget::Tex1D: glCreateTextures(GL_TEXTURE_1D, 1, &m_id); break;
		case TexTarget::Tex1D_Array: glCreateTextures(GL_TEXTURE_1D_ARRAY, 1, &m_id); break;

		case TexTarget::Tex2D: glCreateTextures(GL_TEXTURE_2D, 1, &m_id); break;
		case TexTarget::Tex2D_Array: glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_id); break;

		case TexTarget::Tex3D: glCreateTextures(GL_TEXTURE_3D, 1, &m_id); break;

		case TexTarget::TexCubeMap: glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_id); break;
	}
}

Texture::~Texture() {
	if (m_id != 0) glDeleteTextures(1, &m_id);
}

void ak::render::setActiveTextureUnit(uint32 unit) {
	glActiveTexture(GL_TEXTURE0 + unit);
}

void ak::render::bind(uint32 unit, const Texture& texture) {
	glBindTextureUnit(unit, texture.id());
}



void ak::render::createTextureStorage1D(TexFormat format, uint32 width, uint32 mipLevels) {
	switch(format) {
		case TexFormat::R:    glTextureStorage1D(GL_TEXTURE_1D, mipLevels, GL_RGBA32F,  width); break;
		case TexFormat::RG:   glTextureStorage1D(GL_TEXTURE_1D, mipLevels, GL_RGBA32F,   width); break;
		case TexFormat::RGB:  glTextureStorage1D(GL_TEXTURE_1D, mipLevels, GL_RGBA32F,  width); break;
		case TexFormat::RGBA: glTextureStorage1D(GL_TEXTURE_1D, mipLevels, GL_RGBA32F, width); break;
		case TexFormat::BGR:  glTextureStorage1D(GL_TEXTURE_1D, mipLevels, GL_RGBA32F,  width); break;
		case TexFormat::BGRA: glTextureStorage1D(GL_TEXTURE_1D, mipLevels, GL_RGBA32F, width); break;
	}
}

void ak::render::createTextureStorage1D(TexFormat format, uint32 width, uint32 layers, uint32 mipLevels) {
	switch(format) {
		case TexFormat::R:    glTextureStorage2D(GL_TEXTURE_1D_ARRAY, mipLevels, GL_RGBA32F,  width, layers); break;
		case TexFormat::RG:   glTextureStorage2D(GL_TEXTURE_1D_ARRAY, mipLevels, GL_RGBA32F,   width, layers); break;
		case TexFormat::RGB:  glTextureStorage2D(GL_TEXTURE_1D_ARRAY, mipLevels, GL_RGBA32F,  width, layers); break;
		case TexFormat::RGBA: glTextureStorage2D(GL_TEXTURE_1D_ARRAY, mipLevels, GL_RGBA32F, width, layers); break;
		case TexFormat::BGR:  glTextureStorage2D(GL_TEXTURE_1D_ARRAY, mipLevels, GL_RGBA32F,  width, layers); break;
		case TexFormat::BGRA: glTextureStorage2D(GL_TEXTURE_1D_ARRAY, mipLevels, GL_RGBA32F, width, layers); break;
	}
}

void ak::render::createTextureStorage2D(TexFormat format, uint32 width, uint32 height, uint32 mipLevels) {
	switch(format) {
		case TexFormat::R:    glTextureStorage2D(GL_TEXTURE_2D, mipLevels, GL_RGBA32F,  width, height); break;
		case TexFormat::RG:   glTextureStorage2D(GL_TEXTURE_2D, mipLevels, GL_RGBA32F,   width, height); break;
		case TexFormat::RGB:  glTextureStorage2D(GL_TEXTURE_2D, mipLevels, GL_RGBA32F,  width, height); break;
		case TexFormat::RGBA: glTextureStorage2D(GL_TEXTURE_2D, mipLevels, GL_RGBA32F, width, height); break;
		case TexFormat::BGR:  glTextureStorage2D(GL_TEXTURE_2D, mipLevels, GL_RGBA32F,  width, height); break;
		case TexFormat::BGRA: glTextureStorage2D(GL_TEXTURE_2D, mipLevels, GL_RGBA32F, width, height); break;
	}
}

void ak::render::createTextureStorage2D(TexFormat format, uint32 width, uint32 height, uint32 layers, uint32 mipLevels) {
	switch(format) {
		case TexFormat::R:    glTextureStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, GL_RGBA32F,  width, height, layers); break;
		case TexFormat::RG:   glTextureStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, GL_RGBA32F,   width, height, layers); break;
		case TexFormat::RGB:  glTextureStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, GL_RGBA32F,  width, height, layers); break;
		case TexFormat::RGBA: glTextureStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, GL_RGBA32F, width, height, layers); break;
		case TexFormat::BGR:  glTextureStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, GL_RGBA32F,  width, height, layers); break;
		case TexFormat::BGRA: glTextureStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, GL_RGBA32F, width, height, layers); break;
	}
}

void ak::render::createTextureStorage3D(TexFormat format, uint32 width, uint32 height, uint32 depth, uint32 mipLevels) {
	switch(format) {
		case TexFormat::R:    glTextureStorage3D(GL_TEXTURE_3D, mipLevels, GL_RGBA32F,  width, height, depth); break;
		case TexFormat::RG:   glTextureStorage3D(GL_TEXTURE_3D, mipLevels, GL_RGBA32F,   width, height, depth); break;
		case TexFormat::RGB:  glTextureStorage3D(GL_TEXTURE_3D, mipLevels, GL_RGBA32F,  width, height, depth); break;
		case TexFormat::RGBA: glTextureStorage3D(GL_TEXTURE_3D, mipLevels, GL_RGBA32F, width, height, depth); break;
		case TexFormat::BGR:  glTextureStorage3D(GL_TEXTURE_3D, mipLevels, GL_RGBA32F,  width, height, depth); break;
		case TexFormat::BGRA: glTextureStorage3D(GL_TEXTURE_3D, mipLevels, GL_RGBA32F, width, height, depth); break;
	}
}



void ak::render::setTextureData1D(TexFormat format, uint32 width, const fpSingle* data, uint32 level) {
	switch(format) {
		case TexFormat::R:    glTexImage1D(GL_TEXTURE_1D, level, GL_RGBA32F,  width, 0, GL_RED,  GL_FLOAT, data); break;
		case TexFormat::RG:   glTexImage1D(GL_TEXTURE_1D, level, GL_RGBA32F,   width, 0, GL_RG,   GL_FLOAT, data); break;
		case TexFormat::RGB:  glTexImage1D(GL_TEXTURE_1D, level, GL_RGBA32F,  width, 0, GL_RGB,  GL_FLOAT, data); break;
		case TexFormat::RGBA: glTexImage1D(GL_TEXTURE_1D, level, GL_RGBA32F, width, 0, GL_RGBA, GL_FLOAT, data); break;
		case TexFormat::BGR:  glTexImage1D(GL_TEXTURE_1D, level, GL_RGBA32F,  width, 0, GL_BGR,  GL_FLOAT, data); break;
		case TexFormat::BGRA: glTexImage1D(GL_TEXTURE_1D, level, GL_RGBA32F, width, 0, GL_BGRA, GL_FLOAT, data); break;
	}
}

void ak::render::setTextureData1D(TexFormat format, uint32 width, uint32 layers, const fpSingle* data, uint32 level) {
	switch(format) {
		case TexFormat::R:    glTexImage2D(GL_TEXTURE_1D_ARRAY, level, GL_RGBA32F,  width, layers, 0, GL_RED,  GL_FLOAT, data); break;
		case TexFormat::RG:   glTexImage2D(GL_TEXTURE_1D_ARRAY, level, GL_RGBA32F,   width, layers, 0, GL_RG,   GL_FLOAT, data); break;
		case TexFormat::RGB:  glTexImage2D(GL_TEXTURE_1D_ARRAY, level, GL_RGBA32F,  width, layers, 0, GL_RGB,  GL_FLOAT, data); break;
		case TexFormat::RGBA: glTexImage2D(GL_TEXTURE_1D_ARRAY, level, GL_RGBA32F, width, layers, 0, GL_RGBA, GL_FLOAT, data); break;
		case TexFormat::BGR:  glTexImage2D(GL_TEXTURE_1D_ARRAY, level, GL_RGBA32F,  width, layers, 0, GL_BGR,  GL_FLOAT, data); break;
		case TexFormat::BGRA: glTexImage2D(GL_TEXTURE_1D_ARRAY, level, GL_RGBA32F, width, layers, 0, GL_BGRA, GL_FLOAT, data); break;
	}
}

void ak::render::setTextureData2D(TexFormat format, uint32 width, uint32 height, const fpSingle* data, uint32 level) {
	switch(format) {
		case TexFormat::R:    glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA32F,  width, height, 0, GL_RED,  GL_FLOAT, data); break;
		case TexFormat::RG:   glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA32F,   width, height, 0, GL_RG,   GL_FLOAT, data); break;
		case TexFormat::RGB:  glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA32F,  width, height, 0, GL_RGB,  GL_FLOAT, data); break;
		case TexFormat::RGBA: glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data); break;
		case TexFormat::BGR:  glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA32F,  width, height, 0, GL_BGR,  GL_FLOAT, data); break;
		case TexFormat::BGRA: glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA32F, width, height, 0, GL_BGRA, GL_FLOAT, data); break;
	}
}

void ak::render::setTextureData2D(TexFormat format, uint32 width, uint32 height, uint32 layers, const fpSingle* data, uint32 level) {
	switch(format) {
		case TexFormat::R:    glTexImage3D(GL_TEXTURE_2D_ARRAY, level, GL_RGBA32F,  width, height, layers, 0, GL_RED,  GL_FLOAT, data); break;
		case TexFormat::RG:   glTexImage3D(GL_TEXTURE_2D_ARRAY, level, GL_RGBA32F,   width, height, layers, 0, GL_RG,   GL_FLOAT, data); break;
		case TexFormat::RGB:  glTexImage3D(GL_TEXTURE_2D_ARRAY, level, GL_RGBA32F,  width, height, layers, 0, GL_RGB,  GL_FLOAT, data); break;
		case TexFormat::RGBA: glTexImage3D(GL_TEXTURE_2D_ARRAY, level, GL_RGBA32F, width, height, layers, 0, GL_RGBA, GL_FLOAT, data); break;
		case TexFormat::BGR:  glTexImage3D(GL_TEXTURE_2D_ARRAY, level, GL_RGBA32F,  width, height, layers, 0, GL_BGR,  GL_FLOAT, data); break;
		case TexFormat::BGRA: glTexImage3D(GL_TEXTURE_2D_ARRAY, level, GL_RGBA32F, width, height, layers, 0, GL_BGRA, GL_FLOAT, data); break;
	}
}

void ak::render::setTextureData3D(TexFormat format, uint32 width, uint32 height, uint32 depth, const fpSingle* data, uint32 level) {
	switch(format) {
		case TexFormat::R:    glTexImage3D(GL_TEXTURE_3D, level, GL_RGBA32F,  width, height, depth, 0, GL_RED,  GL_FLOAT, data); break;
		case TexFormat::RG:   glTexImage3D(GL_TEXTURE_3D, level, GL_RGBA32F,   width, height, depth, 0, GL_RG,   GL_FLOAT, data); break;
		case TexFormat::RGB:  glTexImage3D(GL_TEXTURE_3D, level, GL_RGBA32F,  width, height, depth, 0, GL_RGB,  GL_FLOAT, data); break;
		case TexFormat::RGBA: glTexImage3D(GL_TEXTURE_3D, level, GL_RGBA32F, width, height, depth, 0, GL_RGBA, GL_FLOAT, data); break;
		case TexFormat::BGR:  glTexImage3D(GL_TEXTURE_3D, level, GL_RGBA32F,  width, height, depth, 0, GL_BGR,  GL_FLOAT, data); break;
		case TexFormat::BGRA: glTexImage3D(GL_TEXTURE_3D, level, GL_RGBA32F, width, height, depth, 0, GL_BGRA, GL_FLOAT, data); break;
	}
}



void ak::render::replaceTextureData1D(TexFormat format, uint32 xOff, uint32 width, const fpSingle* data, uint32 level) {
	switch(format) {
		case TexFormat::R:    glTexSubImage1D(GL_TEXTURE_1D, level, xOff, width, GL_RED,  GL_FLOAT, data); break;
		case TexFormat::RG:   glTexSubImage1D(GL_TEXTURE_1D, level, xOff, width, GL_RG,   GL_FLOAT, data); break;
		case TexFormat::RGB:  glTexSubImage1D(GL_TEXTURE_1D, level, xOff, width, GL_RGB,  GL_FLOAT, data); break;
		case TexFormat::RGBA: glTexSubImage1D(GL_TEXTURE_1D, level, xOff, width, GL_RGBA, GL_FLOAT, data); break;
		case TexFormat::BGR:  glTexSubImage1D(GL_TEXTURE_1D, level, xOff, width, GL_BGR,  GL_FLOAT, data); break;
		case TexFormat::BGRA: glTexSubImage1D(GL_TEXTURE_1D, level, xOff, width, GL_BGRA, GL_FLOAT, data); break;
	}
}

void ak::render::replaceTextureData1D(TexFormat format, uint32 xOff, uint32 lOff, uint32 width, uint32 layers, const fpSingle* data, uint32 level) {
	switch(format) {
		case TexFormat::R:    glTexSubImage2D(GL_TEXTURE_1D_ARRAY, level, xOff, lOff, width, layers, GL_RED,  GL_FLOAT, data); break;
		case TexFormat::RG:   glTexSubImage2D(GL_TEXTURE_1D_ARRAY, level, xOff, lOff, width, layers, GL_RG,   GL_FLOAT, data); break;
		case TexFormat::RGB:  glTexSubImage2D(GL_TEXTURE_1D_ARRAY, level, xOff, lOff, width, layers, GL_RGB,  GL_FLOAT, data); break;
		case TexFormat::RGBA: glTexSubImage2D(GL_TEXTURE_1D_ARRAY, level, xOff, lOff, width, layers, GL_RGBA, GL_FLOAT, data); break;
		case TexFormat::BGR:  glTexSubImage2D(GL_TEXTURE_1D_ARRAY, level, xOff, lOff, width, layers, GL_BGR,  GL_FLOAT, data); break;
		case TexFormat::BGRA: glTexSubImage2D(GL_TEXTURE_1D_ARRAY, level, xOff, lOff, width, layers, GL_BGRA, GL_FLOAT, data); break;
	}
}

void ak::render::replaceTextureData2D(TexFormat format, uint32 xOff, uint32 yOff, uint32 width, uint32 height, const fpSingle* data, uint32 level) {
	switch(format) {
		case TexFormat::R:    glTexSubImage2D(GL_TEXTURE_2D, level, xOff, yOff, width, height, GL_RED,  GL_FLOAT, data); break;
		case TexFormat::RG:   glTexSubImage2D(GL_TEXTURE_2D, level, xOff, yOff, width, height, GL_RG,   GL_FLOAT, data); break;
		case TexFormat::RGB:  glTexSubImage2D(GL_TEXTURE_2D, level, xOff, yOff, width, height, GL_RGB,  GL_FLOAT, data); break;
		case TexFormat::RGBA: glTexSubImage2D(GL_TEXTURE_2D, level, xOff, yOff, width, height, GL_RGBA, GL_FLOAT, data); break;
		case TexFormat::BGR:  glTexSubImage2D(GL_TEXTURE_2D, level, xOff, yOff, width, height, GL_BGR,  GL_FLOAT, data); break;
		case TexFormat::BGRA: glTexSubImage2D(GL_TEXTURE_2D, level, xOff, yOff, width, height, GL_BGRA, GL_FLOAT, data); break;
	}
}

void ak::render::replaceTextureData2D(TexFormat format, uint32 xOff, uint32 yOff, uint32 lOff, uint32 width, uint32 height, uint32 layers, const fpSingle* data, uint32 level) {
	switch(format) {
		case TexFormat::R:    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, level, xOff, yOff, lOff, width, height, layers, GL_RED,  GL_FLOAT, data); break;
		case TexFormat::RG:   glTexSubImage3D(GL_TEXTURE_2D_ARRAY, level, xOff, yOff, lOff, width, height, layers, GL_RG,   GL_FLOAT, data); break;
		case TexFormat::RGB:  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, level, xOff, yOff, lOff, width, height, layers, GL_RGB,  GL_FLOAT, data); break;
		case TexFormat::RGBA: glTexSubImage3D(GL_TEXTURE_2D_ARRAY, level, xOff, yOff, lOff, width, height, layers, GL_RGBA, GL_FLOAT, data); break;
		case TexFormat::BGR:  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, level, xOff, yOff, lOff, width, height, layers, GL_BGR,  GL_FLOAT, data); break;
		case TexFormat::BGRA: glTexSubImage3D(GL_TEXTURE_2D_ARRAY, level, xOff, yOff, lOff, width, height, layers, GL_BGRA, GL_FLOAT, data); break;
	}
}

void ak::render::replaceTextureData3D(TexFormat format, uint32 xOff, uint32 yOff, uint32 zOff, uint32 width, uint32 height, uint32 depth, const fpSingle* data, uint32 level) {
	switch(format) {
		case TexFormat::R:    glTexSubImage3D(GL_TEXTURE_3D, level, xOff, yOff, zOff, width, height, depth, GL_RED,  GL_FLOAT, data); break;
		case TexFormat::RG:   glTexSubImage3D(GL_TEXTURE_3D, level, xOff, yOff, zOff, width, height, depth, GL_RG,   GL_FLOAT, data); break;
		case TexFormat::RGB:  glTexSubImage3D(GL_TEXTURE_3D, level, xOff, yOff, zOff, width, height, depth, GL_RGB,  GL_FLOAT, data); break;
		case TexFormat::RGBA: glTexSubImage3D(GL_TEXTURE_3D, level, xOff, yOff, zOff, width, height, depth, GL_RGBA, GL_FLOAT, data); break;
		case TexFormat::BGR:  glTexSubImage3D(GL_TEXTURE_3D, level, xOff, yOff, zOff, width, height, depth, GL_BGR,  GL_FLOAT, data); break;
		case TexFormat::BGRA: glTexSubImage3D(GL_TEXTURE_3D, level, xOff, yOff, zOff, width, height, depth, GL_BGRA, GL_FLOAT, data); break;
	}
}

