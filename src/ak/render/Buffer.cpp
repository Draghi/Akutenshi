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

#include <ak/render/Buffer.hpp>
#include "GL/gl4.h"

using namespace ak::render;

Buffer::Buffer() : m_id(0) { glGenBuffers(1, &m_id); }
Buffer::~Buffer() { if (m_id) glDeleteBuffers(1, &m_id); }

void ak::render::bind(BufferTarget target, const Buffer& buffer) {
	switch(target) {
		case BufferTarget::VARRYING: glBindBuffer(GL_ARRAY_BUFFER,         buffer.id()); break;
		case BufferTarget::UNIFORM:  glBindBuffer(GL_UNIFORM_BUFFER,       buffer.id()); break;
		case BufferTarget::INDEX:    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.id()); break;
	}
}

void ak::render::setDataRaw(BufferTarget target, const void* data, uint32 size) {
	switch(target) {
		case BufferTarget::VARRYING: glBufferData(GL_ARRAY_BUFFER,         size, data, GL_STATIC_DRAW); break;
		case BufferTarget::UNIFORM:  glBufferData(GL_UNIFORM_BUFFER,       size, data, GL_STATIC_DRAW); break;
		case BufferTarget::INDEX:    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW); break;
	}
}

void ak::render::replaceDataRaw(BufferTarget target, uint32 offset, const void* data, uint32 size) {
	switch(target) {
		case BufferTarget::VARRYING: glBufferSubData(GL_ARRAY_BUFFER,         offset, size, data); break;
		case BufferTarget::UNIFORM:  glBufferSubData(GL_UNIFORM_BUFFER,       offset, size, data); break;
		case BufferTarget::INDEX:    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data); break;
	}
}


