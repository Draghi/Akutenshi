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

#include <ak/render/gl/Buffers.hpp>
#include <GL/gl4.h>
#include <stdexcept>

using namespace akr::gl;

static GLbitfield akBufferFlagsToOGL(uint8 hint) {
	GLbitfield result = 0;
	result |= (hint & BufferHint_Dynamic)           == BufferHint_Dynamic           ? GL_DYNAMIC_STORAGE_BIT : 0;
	result |= (hint & BufferHint_Map_Read)          == BufferHint_Map_Read          ? GL_MAP_READ_BIT        : 0;
	result |= (hint & BufferHint_Map_Write)         == BufferHint_Map_Write         ? GL_MAP_WRITE_BIT       : 0;
	result |= (hint & BufferHint_Map_Coherent_IO)   == BufferHint_Map_Coherent_IO   ? GL_MAP_COHERENT_BIT    : 0;
	result |= (hint & BufferHint_Map_Persistant_IO) == BufferHint_Map_Persistant_IO ? GL_MAP_PERSISTENT_BIT  : 0;
	result |= (hint & BufferHint_Store_Client)      == BufferHint_Store_Client      ? GL_CLIENT_STORAGE_BIT  : 0;
	return result;
}

Buffer::Buffer() : m_id(0), m_hint(0) {}

Buffer::Buffer(const void* data, akSize size, uint8 hint) : m_id(0), m_hint(hint) {
	glCreateBuffers(1, &m_id);
	glNamedBufferStorage(m_id, size, data, akBufferFlagsToOGL(m_hint));
}

Buffer::Buffer(Buffer&& other) : m_id(other.m_id), m_hint(other.m_hint) {
	other.m_id = 0;
	other.m_hint = 0;
}

Buffer::~Buffer() {
	if (isValid()) glDeleteBuffers(1, &m_id);
}

bool Buffer::writeData(const void* data, akSize size, akSize offset) {
	if (!canWrite()) return false;
	glNamedBufferSubData(m_id, offset, size, data);
	return true;
}

bool Buffer::writeData(const Buffer& src, akSize size, akSize readOff, akSize writeOff) {
	if (!canWrite()) return false;
	glCopyNamedBufferSubData(src.id(), m_id, readOff, writeOff, size);
	return true;
}

bool Buffer::readData(void* dst, akSize size, akSize offset) const {
	if (!isValid()) return false;
	glGetNamedBufferSubData(m_id, offset, size, dst);
	return true;
}

uint32 Buffer::id() const { return m_id; }
bool Buffer::canWrite() const { return isValid() && ((m_hint & BufferHint_Dynamic) == BufferHint_Dynamic); }
bool Buffer::isValid() const { return m_id != 0; }

Buffer& Buffer::operator=(Buffer&& other) {
	if (m_id == other.m_id) return *this;
	if (isValid()) glDeleteBuffers(1, &m_id);
	m_id   = other.m_id;   other.m_id   = 0;
	m_hint = other.m_hint; other.m_hint = 0;
	return *this;
}

static GLenum akBufferTargetToOGL(BufferTarget target) {
	switch(target) {
		case BufferTarget::VARRYING: return GL_ARRAY_BUFFER;
		case BufferTarget::UNIFORM: return GL_UNIFORM_BUFFER;
		case BufferTarget::INDEX: return GL_ELEMENT_ARRAY_BUFFER;
	}
}

void akr::gl::bindBuffer(BufferTarget target, const Buffer& buffer, uint32 index, akSize offset, akSize size) {
	if (target != BufferTarget::UNIFORM) throw std::logic_error("Only uniform buffers have binding indicies.");
	glBindBufferRange(akBufferTargetToOGL(target), index, buffer.id(), offset, size);
}

void akr::gl::bindBuffer(BufferTarget target, const Buffer& buffer, uint32 index) {
	if (target != BufferTarget::UNIFORM) throw std::logic_error("Only uniform buffers have binding indicies.");
	glBindBufferBase(akBufferTargetToOGL(target), index, buffer.id());
}

void akr::gl::bindBuffer(BufferTarget target, const Buffer& buffer) {
	glBindBuffer(akBufferTargetToOGL(target), buffer.id());
}
