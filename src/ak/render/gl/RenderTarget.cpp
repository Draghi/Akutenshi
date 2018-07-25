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

#include <ak/render/gl/RenderTarget.hpp>

#include <GL/gl4.h>
#include <algorithm>
#include <stdexcept>

#include <ak/BitManipulaton.hpp>

using namespace akr::gl;

static GLenum akToGLTexStorage(TexFormat format, TexStorage storage);
static GLenum akToGLDepthStorage(DepthStorage depthStorage);
static GLenum akToGLStencilStorage(StencilStorage stencilStorage);
static GLenum akToGLDepthStencilStorage(DepthStencilStorage depthStencilStorage);

RenderBuffer::RenderBuffer() : m_id(0) { glCreateRenderbuffers(1, &m_id); }
RenderBuffer::RenderBuffer(RenderBuffer&& other) : m_id(std::move(other.m_id)) { other.m_id = 0; }

RenderBuffer::~RenderBuffer() { if (m_id) glDeleteRenderbuffers(1, &m_id); }

uint32 RenderBuffer::id() const { return m_id; }
bool RenderBuffer::isValid() const { return m_id != 0; }

RenderBuffer& RenderBuffer::operator=(RenderBuffer&& other) { m_id = std::move(other.m_id); other.m_id = 0; return *this;}



void akr::gl::newRenderBufferColourStorage(RenderBuffer& renderBuffer, TexFormat format, TexStorage storage, akSize width, akSize height, akSize multisampling) {
	glNamedRenderbufferStorageMultisample(renderBuffer.id(), multisampling, akToGLTexStorage(format, storage), width, height);
}

void akr::gl::newRenderBufferDepthStorage(RenderBuffer& renderBuffer, DepthStorage depthStorage, akSize width, akSize height, akSize multisampling) {
	glNamedRenderbufferStorageMultisample(renderBuffer.id(), multisampling, akToGLDepthStorage(depthStorage), width, height);
}

void akr::gl::newRenderBufferStencilStorage(RenderBuffer& renderBuffer, StencilStorage stencilStorage, akSize width, akSize height, akSize multisampling) {
	glNamedRenderbufferStorageMultisample(renderBuffer.id(), multisampling, akToGLStencilStorage(stencilStorage), width, height);
}

void akr::gl::newRenderBufferDepthStencilStorage(RenderBuffer& renderBuffer, DepthStencilStorage depthStencilStorage, akSize width, akSize height, akSize multisampling) {
	glNamedRenderbufferStorageMultisample(renderBuffer.id(), multisampling, akToGLDepthStencilStorage(depthStencilStorage), width, height);
}





RenderTarget::RenderTarget() : m_id(0) { glCreateFramebuffers(1, &m_id); }
RenderTarget::RenderTarget(RenderTarget&& other) : m_id(std::move(other.m_id)) { other.m_id = 0; }

RenderTarget::~RenderTarget() { if (m_id) glDeleteFramebuffers(1, &m_id); }

uint32 RenderTarget::id() const { return m_id; }
bool RenderTarget::isValid() const { return m_id != 0; }

RenderTarget& RenderTarget::operator=(RenderTarget&& other) { m_id = std::move(other.m_id); other.m_id = 0; return *this;}








void akr::gl::bindRenderTarget(const RenderTarget& target) {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.id());
}

void akr::gl::bindDisplayRenderTarget() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

static GLbitfield akToGLBlitMask(BlitMask mask) {
	GLbitfield result = 0;
	result |= ak::hasBitFlag(mask, BlitMask::Colour)  ? GL_COLOR_BUFFER_BIT : 0;
	result |= ak::hasBitFlag(mask, BlitMask::Depth)   ? GL_DEPTH_BUFFER_BIT : 0;
	result |= ak::hasBitFlag(mask, BlitMask::Stencil) ? GL_STENCIL_BUFFER_BIT : 0;
	return result;
}

void akr::gl::blitRenderTargets(const RenderTarget& src, const RenderTarget& dst, const akm::Vec2& srcStart, const akm::Vec2& srcEnd, const akm::Vec2& dstStart, const akm::Vec2& dstEnd, BlitMask mask, FilterType filter) {
	glBlitNamedFramebuffer(src.id(), dst.id(),
			srcStart.x, srcStart.y, srcEnd.x, srcEnd.y,
			dstStart.x, dstStart.y, dstEnd.x, dstEnd.y,
			akToGLBlitMask(mask),
			filter == FilterType::Nearest ? GL_NEAREST : GL_LINEAR
	);
}

void akr::gl::blitRenderTargetToDisplay(const RenderTarget& src, const akm::Vec2& srcStart, const akm::Vec2& srcEnd, const akm::Vec2& dstStart, const akm::Vec2& dstEnd, BlitMask mask, FilterType filter) {
	glBlitNamedFramebuffer(src.id(), 0,
			srcStart.x, srcStart.y, srcEnd.x, srcEnd.y,
			dstStart.x, dstStart.y, dstEnd.x, dstEnd.y,
			akToGLBlitMask(mask),
			filter == FilterType::Nearest ? GL_NEAREST : GL_LINEAR
	);
}

void akr::gl::blitDisplayToRenderTarget(const RenderTarget& dst, const akm::Vec2& srcStart, const akm::Vec2& srcEnd, const akm::Vec2& dstStart, const akm::Vec2& dstEnd, BlitMask mask, FilterType filter) {
	glBlitNamedFramebuffer(0, dst.id(),
			srcStart.x, srcStart.y, srcEnd.x, srcEnd.y,
			dstStart.x, dstStart.y, dstEnd.x, dstEnd.y,
			akToGLBlitMask(mask),
			filter == FilterType::Nearest ? GL_NEAREST : GL_LINEAR
	);
}


void akr::gl::invalidateColourAttachment(const RenderTarget& target, uint32 index) {
	uint32 enumID = GL_COLOR_ATTACHMENT0 + index;
	glInvalidateNamedFramebufferData(target.id(), 1, &enumID);
}

void akr::gl::invalidateColourAttachments(const RenderTarget& target, const std::vector<uint32>& colourIndexes) {
	std::vector<uint32> enumIDs = colourIndexes;
	for(auto& id : enumIDs) id += GL_COLOR_ATTACHMENT0;
	glInvalidateNamedFramebufferData(target.id(), enumIDs.size(), enumIDs.data());
}

void akr::gl::invalidateDepthAttachment(const RenderTarget& target) {
	uint32 enumID = GL_DEPTH_ATTACHMENT;
	glInvalidateNamedFramebufferData(target.id(), 1, &enumID);
}

void akr::gl::invalidateStencilAttachment(const RenderTarget& target) {
	uint32 enumID = GL_STENCIL_ATTACHMENT;
	glInvalidateNamedFramebufferData(target.id(), 1, &enumID);
}

void akr::gl::invalidateDepthStencilAttachment(const RenderTarget& target) {
	uint32 enumID = GL_DEPTH_STENCIL_ATTACHMENT;
	glInvalidateNamedFramebufferData(target.id(), 1, &enumID);
}


void akr::gl::attachColourBuffer(RenderTarget& target, uint32 index, RenderBuffer& renderBuffer) {
	glNamedFramebufferRenderbuffer(target.id(), GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, renderBuffer.id());
}

void akr::gl::attachDepthBuffer(RenderTarget& target, RenderBuffer& renderBuffer) {
	glNamedFramebufferRenderbuffer(target.id(), GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer.id());
}

void akr::gl::attachStencilBuffer(RenderTarget& target, RenderBuffer& renderBuffer) {
	glNamedFramebufferRenderbuffer(target.id(), GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer.id());
}

void akr::gl::attachDepthStencilBuffer(RenderTarget& target, RenderBuffer& renderBuffer) {
	glNamedFramebufferRenderbuffer(target.id(), GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer.id());
}


void akr::gl::attachColourTexture(RenderTarget& target, uint32 index, Texture& texture) {
	glNamedFramebufferTexture(target.id(), GL_COLOR_ATTACHMENT0 + index, texture.id(), 0);
}

void akr::gl::attachDepthTexture(RenderTarget& target, Texture& texture) {
	glNamedFramebufferTexture(target.id(), GL_DEPTH_ATTACHMENT, texture.id(), 0);
}

void akr::gl::attachStencilTexture(RenderTarget& target, Texture& texture) {
	glNamedFramebufferTexture(target.id(), GL_STENCIL_ATTACHMENT, texture.id(), 0);
}

void akr::gl::attachDepthStencilTexture(RenderTarget& target, Texture& texture) {
	glNamedFramebufferTexture(target.id(), GL_DEPTH_STENCIL_ATTACHMENT, texture.id(), 0);
}

void akr::gl::attachColourTexture(RenderTarget& target, uint32 index, uint32 layer, Texture& texture) {
	glNamedFramebufferTextureLayer(target.id(), GL_COLOR_ATTACHMENT0 + index, texture.id(), 0, layer);
}

void akr::gl::attachDepthTexture(RenderTarget& target, uint32 layer, Texture& texture) {
	glNamedFramebufferTextureLayer(target.id(), GL_DEPTH_ATTACHMENT, texture.id(), 0, layer);
}

void akr::gl::attachStencilTexture(RenderTarget& target, uint32 layer, Texture& texture) {
	glNamedFramebufferTextureLayer(target.id(), GL_STENCIL_ATTACHMENT, texture.id(), 0, layer);
}

void akr::gl::attachDepthStencilTexture(RenderTarget& target, uint32 layer, Texture& texture) {
	glNamedFramebufferTextureLayer(target.id(), GL_DEPTH_STENCIL_ATTACHMENT, texture.id(), 0, layer);
}


void akr::gl::detachColour(RenderTarget& target, uint32 index) {
	glNamedFramebufferTexture(target.id(), GL_COLOR_ATTACHMENT0 + index, 0, 0);
}

void akr::gl::detachDepth(RenderTarget& target) {
	glNamedFramebufferTexture(target.id(), GL_DEPTH_ATTACHMENT, 0, 0);
}

void akr::gl::detachStencil(RenderTarget& target) {
	glNamedFramebufferTexture(target.id(), GL_STENCIL_ATTACHMENT, 0, 0);
}

void akr::gl::detachDepthStencil(RenderTarget& target) {
	glNamedFramebufferTexture(target.id(), GL_DEPTH_STENCIL_ATTACHMENT, 0, 0);
}











static uint32 akToGLTexStorage(TexFormat format, TexStorage storage) {
	switch(format) {
		case TexFormat::R: {
			switch(storage) {
				case TexStorage::Byte:      return GL_R8;
				case TexStorage::Byte_sRGB: throw std::logic_error("Texture storage format 'sR' is not supported.");
				case TexStorage::Short:     return GL_R16;
				case TexStorage::Half:      return GL_R16F;
				case TexStorage::Single:    return GL_R32F;
			}
		}

		case TexFormat::RG: {
			switch(storage) {
				case TexStorage::Byte:      return GL_RG8;
				case TexStorage::Byte_sRGB: throw std::logic_error("Texture storage format 'sRG' is not supported.");
				case TexStorage::Short:     return GL_RG16;
				case TexStorage::Half:      return GL_RG16F;
				case TexStorage::Single:    return GL_RG32F;
			}
		}

		case TexFormat::RGB: {
			switch(storage) {
				case TexStorage::Byte:      return GL_RGB8;
				case TexStorage::Byte_sRGB: return GL_SRGB8;
				case TexStorage::Short:     return GL_RGB16;
				case TexStorage::Half:      return GL_RGB16F;
				case TexStorage::Single:    return GL_RGB32F;
			}
		}

		case TexFormat::RGBA: {
			switch(storage) {
				case TexStorage::Byte:      return GL_RGBA8;
				case TexStorage::Byte_sRGB: return GL_SRGB8_ALPHA8;
				case TexStorage::Short:     return GL_RGBA16;
				case TexStorage::Half:      return GL_RGBA16F;
				case TexStorage::Single:    return GL_RGBA32F;
			}
		}
	}
}

static GLenum akToGLDepthStorage(DepthStorage depthStorage) {
	switch(depthStorage) {
		case DepthStorage::SIZE_16:  return GL_DEPTH_COMPONENT16;
		case DepthStorage::SIZE_24:  return GL_DEPTH_COMPONENT24;
		case DepthStorage::SIZE_32:  return GL_DEPTH_COMPONENT32;
		case DepthStorage::SIZE_32F: return GL_DEPTH_COMPONENT32F;
	}
}

static GLenum akToGLStencilStorage(StencilStorage stencilStorage) {
	switch(stencilStorage) {
		case StencilStorage::SIZE_1:  return GL_STENCIL_INDEX1;
		case StencilStorage::SIZE_4:  return GL_STENCIL_INDEX4;
		case StencilStorage::SIZE_8:  return GL_STENCIL_INDEX8;
		case StencilStorage::SIZE_16: return GL_STENCIL_INDEX16;
	}
}

static GLenum akToGLDepthStencilStorage(DepthStencilStorage depthStencilStorage) {
	switch(depthStencilStorage) {
		case DepthStencilStorage::SIZE_D24_S8:  return GL_DEPTH24_STENCIL8;
		case DepthStencilStorage::SIZE_D32F_S8: return GL_DEPTH32F_STENCIL8;
	}
}
