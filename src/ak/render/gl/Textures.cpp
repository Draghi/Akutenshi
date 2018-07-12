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

#include <ak/Log.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/render/gl/Textures.hpp>
#include <ak/Traits.hpp>
#include <ext/type_traits.h>
#include <glm/common.hpp>
#include <cmath>
#include <optional>
#include <stdexcept>

#include "GL/gl4.h"

using namespace akr::gl;

// Helper fwd decl
static GLenum akToGLTarget(TexTarget target);
static GLenum cubemapTargetToGlTarget(CubemapTarget akTarget);
static uint32 akToGLTexStorage(TexFormat format, TexStorage storage);
static uint32 akToGLComponents(TexFormat format);
static uint akToGLType(DataType dType);
static akSize getDataTypeSize(DataType dType);

// Texture object

Texture::Texture() : m_id(0), m_type() {}
Texture::Texture(TexTarget target) : m_id(0), m_type(target) { glCreateTextures(akToGLTarget(target), 1, &m_id); }
Texture::Texture(Texture&& other) : m_id(other.m_id), m_type(other.m_type) { other.m_id = 0; }
Texture::~Texture() { if (m_id != 0) glDeleteTextures(1, &m_id); }
Texture& Texture::operator=(Texture&& other) {
	if (m_id != 0) glDeleteTextures(1, &m_id);
	m_id = other.m_id; other.m_id = 0; m_type = other.m_type;
	return *this;
}

// Create Texture

std::optional<Texture> akr::gl::createTex1D(uint32 unit, TexFormat format, TexStorage storageType, DataType dataType, const void* data, akSize width, akSize maxMipmapLevels) {
	if ((data == nullptr) || (width == 0)) return std::optional<Texture>();

	auto mipmapLevels = std::min(maxMipmapLevels, calcTexMaxMipmaps(width));

	Texture tex(TexTarget::Tex1D);
	bindTexture(unit, tex);
	setActiveTexUnit(unit);
	newTexStorage1D(format, storageType, width, mipmapLevels);
	loadTexData1D(0, format, dataType, data, width);

	return std::optional<Texture>{std::move(tex)};
}

std::optional<Texture> akr::gl::createTex2D(uint32 unit, TexFormat format, TexStorage storageType, DataType dataType, const void* data, akSize width, akSize height, akSize maxMipmapLevels) {
	if ((data == nullptr) || (width == 0) || (height == 0)) return std::optional<Texture>();

	auto mipmapLevels = std::min(maxMipmapLevels, calcTexMaxMipmaps(width, height));

	Texture tex(TexTarget::Tex2D);
	bindTexture(unit, tex);
	setActiveTexUnit(unit);
	newTexStorage2D(format, storageType, width, height, mipmapLevels);
	loadTexData2D(0, format, dataType, data, width, height);

	return std::optional<Texture>{std::move(tex)};
}

std::optional<Texture> akr::gl::createTex3D(uint32 unit, TexFormat format, TexStorage storageType, DataType dataType, const void* data, akSize width, akSize height, akSize depth, akSize maxMipmapLevels) {
	if ((data == nullptr) || (width == 0) || (height == 0) || (depth == 0)) return std::optional<Texture>();

	auto mipmapLevels = std::min(maxMipmapLevels, calcTexMaxMipmaps(width, height, depth));

	Texture tex(TexTarget::Tex3D);
	bindTexture(unit, tex);
	setActiveTexUnit(unit);
	newTexStorage3D(format, storageType, width, height, depth, mipmapLevels);
	loadTexData3D(0, format, dataType, data, width, height, depth);

	return std::optional<Texture>{std::move(tex)};
}

std::optional<Texture> akr::gl::createTexCubemap(uint32 unit, TexFormat format, TexStorage storageType, DataType dataType, const void* data, akSize width, akSize height, akSize maxMipmapLevels) {
	if ((data == nullptr) || (width == 0) || (height == 0)) return std::optional<Texture>();

	auto mipmapLevels = std::min(maxMipmapLevels, calcTexMaxMipmaps(width, height));
	akSize layerSize = width*height*getDataTypeSize(dataType)*getTexComponentsFromFormat(format);
	mipmapLevels = std::min<akSize>(1, mipmapLevels);

	Texture tex(TexTarget::TexCubemap);
	bindTexture(unit, tex);
	setActiveTexUnit(unit);
	newTexStorageCubemap(format, storageType, width, height, mipmapLevels);
	loadTexDataCubemap(CubemapTarget::PosX, 0, format, dataType, reinterpret_cast<const uint8*>(data) + layerSize*0, width, height);
	loadTexDataCubemap(CubemapTarget::PosY, 0, format, dataType, reinterpret_cast<const uint8*>(data) + layerSize*1, width, height);
	loadTexDataCubemap(CubemapTarget::PosZ, 0, format, dataType, reinterpret_cast<const uint8*>(data) + layerSize*2, width, height);
	loadTexDataCubemap(CubemapTarget::NegX, 0, format, dataType, reinterpret_cast<const uint8*>(data) + layerSize*3, width, height);
	loadTexDataCubemap(CubemapTarget::NegY, 0, format, dataType, reinterpret_cast<const uint8*>(data) + layerSize*4, width, height);
	loadTexDataCubemap(CubemapTarget::NegZ, 0, format, dataType, reinterpret_cast<const uint8*>(data) + layerSize*5, width, height);

	if (mipmapLevels > 1) genTexMipmaps(TexTarget::TexCubemap);

	return std::optional<Texture>{std::move(tex)};
}

std::optional<Texture> akr::gl::createTex1DArray(uint32 /*unit*/, TexFormat /*format*/, TexStorage /*storageType*/, DataType /*dataType*/, const void* /*data*/, akSize /*width*/, akSize /*layers*/, akSize /*maxMipmapLevels*/) {
	throw std::logic_error("Not implemented");
}

std::optional<Texture> akr::gl::createTex2DArray(uint32 /*unit*/, TexFormat /*format*/, TexStorage /*storageType*/, DataType /*dataType*/, const void* /*data*/, akSize /*width*/, akSize /*height*/, akSize /*layers*/, akSize /*maxMipmapLevels*/) {
	throw std::logic_error("Not implemented");
}

// State management

void akr::gl::setActiveTexUnit(uint32 unit) {
	glActiveTexture(GL_TEXTURE0 + unit);
}

void akr::gl::bindTexture(uint32 unit, const Texture& texture) {
	glBindTextureUnit(unit, texture.id());
}

// Create storage

void akr::gl::newTexStorage1D(TexFormat format, TexStorage storage, akSize width, akSize mipLevels) {
	glTexStorage1D(GL_TEXTURE_1D, mipLevels, akToGLTexStorage(format, storage), width);
}

void akr::gl::newTexStorage2D(TexFormat format, TexStorage storage, akSize width, akSize height, akSize mipLevels) {
	glTexStorage2D(GL_TEXTURE_2D, mipLevels, akToGLTexStorage(format, storage), width, height);
}

void akr::gl::newTexStorage3D(TexFormat format, TexStorage storage, akSize width, akSize height, akSize depth, akSize mipLevels) {
	glTexStorage3D(GL_TEXTURE_3D, mipLevels, akToGLTexStorage(format, storage), width, height, depth);
}

void akr::gl::newTexStorageCubemap(TexFormat format, TexStorage storage, akSize width, akSize height, akSize mipLevels) {
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, mipLevels, akToGLTexStorage(format, storage), width, height);
}

void akr::gl::newTexStorage1DArray(TexFormat format, TexStorage storage, akSize width, akSize layers, akSize mipLevels) {
	glTexStorage2D(GL_TEXTURE_1D_ARRAY, mipLevels, akToGLTexStorage(format, storage), width, layers);
}

void akr::gl::newTexStorage2DArray(TexFormat format, TexStorage storage, akSize width, akSize height, akSize layers, akSize mipLevels) {
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, akToGLTexStorage(format, storage), width, height, layers);
}

// Replace data

void akr::gl::loadTexData1D(akSize miplevel, TexFormat format, DataType dataType, const void* data, akSize width, akSize xOff) {
	glTexSubImage1D(GL_TEXTURE_1D, miplevel, xOff, width, akToGLComponents(format), akToGLType(dataType), data);
}

void akr::gl::loadTexData2D(akSize miplevel, TexFormat format, DataType dataType, const void* data, akSize width, akSize height, akSize xOff, akSize yOff) {
	glTexSubImage2D(GL_TEXTURE_2D, miplevel, xOff, yOff, width, height, akToGLComponents(format), akToGLType(dataType), data);
}

void akr::gl::loadTexData3D(akSize miplevel, TexFormat format, DataType dataType, const void* data, akSize width, akSize height, akSize depth, akSize xOff, akSize yOff, akSize zOff) {
	glTexSubImage3D(GL_TEXTURE_3D, miplevel, xOff, yOff, zOff, width, height, depth, akToGLComponents(format), akToGLType(dataType), data);
}

void akr::gl::loadTexDataCubemap(CubemapTarget cubemap, akSize miplevel, TexFormat format, DataType dataType, const void* data, akSize width, akSize height, akSize xOff, akSize yOff) {
	glTexSubImage2D(cubemapTargetToGlTarget(cubemap), miplevel, xOff, yOff, width, height, akToGLComponents(format), akToGLType(dataType), data);
}

void akr::gl::loadTexData1DArray(akSize miplevel, TexFormat format, DataType dataType, const void* data, akSize width, akSize layer, akSize layers, akSize xOff) {
	glTexSubImage2D(GL_TEXTURE_1D_ARRAY, miplevel, xOff, layer, width, layers, akToGLComponents(format), akToGLType(dataType), data);
}

void akr::gl::loadTexData2DArray(akSize miplevel, TexFormat format, DataType dataType, const void* data, akSize width, akSize height, akSize layer, akSize layers, akSize xOff, akSize yOff) {
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, miplevel, xOff, yOff, layer, width, height, layers, akToGLComponents(format), akToGLType(dataType), data);
}

// Set properties

void akr::gl::setTexFilters(TexTarget target, FilterType minFilter, FilterType magFilter) {
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

	glTexParameteri(akToGLTarget(target), GL_TEXTURE_MIN_FILTER, glMinFilter);
	glTexParameteri(akToGLTarget(target), GL_TEXTURE_MAG_FILTER, glMagFilter);
}

void akr::gl::setTexFilters(TexTarget target, FilterType minFilter, MipFilterType minMipFilter, FilterType magFilter) {
	if (minMipFilter == MipFilterType::None) {
		setTexFilters(target, minFilter, magFilter);
		return;
	}

	int32 glMinFilter;
	switch(minFilter) {
		case FilterType::Nearest: glMinFilter = (minMipFilter == MipFilterType::Nearest) ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_LINEAR; break;
		case FilterType::Linear:  glMinFilter = (minMipFilter == MipFilterType::Nearest) ? GL_LINEAR_MIPMAP_NEAREST  : GL_LINEAR_MIPMAP_LINEAR;  break;
	}

	int32 glMagFilter;
	switch(magFilter) {
		case FilterType::Nearest: glMagFilter = GL_NEAREST; break;
		case FilterType::Linear:  glMagFilter = GL_LINEAR; break;
	}

	glTexParameteri(akToGLTarget(target), GL_TEXTURE_MIN_FILTER, glMinFilter);
	glTexParameteri(akToGLTarget(target), GL_TEXTURE_MAG_FILTER, glMagFilter);
}

void akr::gl::setTexClamping(TexTarget target, ClampDir clampDir, ClampType clampType) {
	uint32 glClampDir;
	switch(clampDir) {
		case ClampDir::Horz: glClampDir = GL_TEXTURE_WRAP_S; break;
		case ClampDir::Vert: glClampDir = GL_TEXTURE_WRAP_T; break;
		case ClampDir::Depth: glClampDir = GL_TEXTURE_WRAP_R; break;
	}

	int32 glClampType;
	switch(clampType) {
		case ClampType::Repeat:     glClampType = GL_REPEAT; break;
		case ClampType::Mirror:     glClampType = GL_MIRRORED_REPEAT; break;
		case ClampType::Edge:       glClampType = GL_CLAMP_TO_EDGE; break;
		case ClampType::Border:     glClampType = GL_CLAMP_TO_BORDER; break;
	}

	glTexParameteri(akToGLTarget(target), glClampDir, glClampType);
}

void akr::gl::setTexBorder(TexTarget target, akm::Vec4 colour) {
	glTexParameterfv(akToGLTarget(target), GL_TEXTURE_BORDER_COLOR, &colour[0]);
}

void akr::gl::setTexAnisotropy(TexTarget target, fpSingle amount) {
	glTexParameterf(akToGLTarget(target), GL_TEXTURE_MAX_ANISOTROPY_EXT, akm::min(getTexMaxAnsiotropy(), amount));
}

// Processes

void akr::gl::genTexMipmaps(TexTarget target) {
	glGenerateMipmap(akToGLTarget(target));
}

// Query

akSize akr::gl::calcTexMaxMipmaps(akSize width, akSize height, akSize depth) {
	fpSingle maxDim = akm::max(akm::max(width, height), depth);
	return static_cast<akSize>(akm::floor(akm::log2(maxDim))+1);
}


fpSingle akr::gl::getTexMaxAnsiotropy() {
	static fpSingle maxAnsio = []() {
		fpSingle fLargest;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
		return fLargest;
	}();
	return maxAnsio;
}

akSize akr::gl::getTexComponentsFromFormat(TexFormat format) {
	switch(format) {
		case TexFormat::R: return 1;
		case TexFormat::RG: return 2;
		case TexFormat::RGB: return 3;
		case TexFormat::RGBA: return 4;
	}
}

TexFormat akr::gl::getTexFormatFromComponents(akSize components) {
	switch(components) {
		default: throw std::runtime_error("Invalid image");
		case 1: return TexFormat::R;
		case 2: return TexFormat::RG;
		case 3: return TexFormat::RGB;
		case 4: return TexFormat::RGBA;
	}
}












// Helper methods

static GLenum akToGLTarget(TexTarget target) {
	switch(target) {
		case TexTarget::Tex1D:       return GL_TEXTURE_1D;
		case TexTarget::Tex1D_Array: return GL_TEXTURE_1D_ARRAY;
		case TexTarget::Tex2D:       return GL_TEXTURE_2D;
		case TexTarget::Tex2D_Array: return GL_TEXTURE_2D_ARRAY;
		case TexTarget::Tex3D:       return GL_TEXTURE_3D;
		case TexTarget::TexCubemap:  return GL_TEXTURE_CUBE_MAP;
	}
}

static GLenum cubemapTargetToGlTarget(CubemapTarget akTarget) {
	switch(akTarget) {
		case CubemapTarget::PosX: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		case CubemapTarget::PosY: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
		case CubemapTarget::PosZ: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
		case CubemapTarget::NegX: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
		case CubemapTarget::NegY: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
		case CubemapTarget::NegZ: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
	}
	throw std::runtime_error("Invalid cubemap target");
}

static uint32 akToGLTexStorage(TexFormat format, TexStorage storage) {
	switch(format) {
		case TexFormat::R: {
			switch(storage) {
				case TexStorage::Byte:      return GL_R8;
				case TexStorage::Byte_sRGB: throw std::logic_error("Texture storage format 'sR' is not supported.");
				case TexStorage::Half:      return GL_R16F;
				case TexStorage::Single:    return GL_R32F;
			}
		}

		case TexFormat::RG: {
			switch(storage) {
				case TexStorage::Byte:      return GL_RG8;
				case TexStorage::Byte_sRGB: throw std::logic_error("Texture storage format 'sRG' is not supported.");
				case TexStorage::Half:      return GL_RG16F;
				case TexStorage::Single:    return GL_RG32F;
			}
		}

		case TexFormat::RGB: {
			switch(storage) {
				case TexStorage::Byte:      return GL_RGB8;
				case TexStorage::Byte_sRGB: return GL_SRGB8;
				case TexStorage::Half:      return GL_RGB16F;
				case TexStorage::Single:    return GL_RGB32F;
			}
		}

		case TexFormat::RGBA: {
			switch(storage) {
				case TexStorage::Byte:      return GL_RGBA8;
				case TexStorage::Byte_sRGB: return GL_SRGB8_ALPHA8;
				case TexStorage::Half:      return GL_RGBA16F;
				case TexStorage::Single:    return GL_RGBA32F;
			}
		}
	}
}

static uint32 akToGLComponents(TexFormat format) {
	switch(format) {
		case TexFormat::R:    return GL_RED;
		case TexFormat::RG:   return GL_RG;
		case TexFormat::RGB:  return GL_RGB;
		case TexFormat::RGBA: return GL_RGBA;
	}
}

static uint akToGLType(DataType dType) {
	switch(dType) {
		case DataType::Int8: return GL_BYTE;
		case DataType::Int16: return GL_SHORT;
		case DataType::Int32: return GL_INT;
		case DataType::UInt8: return GL_UNSIGNED_BYTE;
		case DataType::UInt16: return GL_UNSIGNED_SHORT;
		case DataType::UInt32: return GL_UNSIGNED_INT;
		case DataType::Single: return GL_FLOAT;
		case DataType::Double: return GL_DOUBLE;
	}
}

static akSize getDataTypeSize(DataType dType) {
	switch(dType) {
		case DataType::Int8: return 1;
		case DataType::Int16: return 2;
		case DataType::Int32: return 4;
		case DataType::UInt8: return 1;
		case DataType::UInt16: return 2;
		case DataType::UInt32: return 4;
		case DataType::Single: return 4;
		case DataType::Double: return 8;
	}
}
