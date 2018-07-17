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

#include <ak/render/gl/VertexArrays.hpp>

#include <GL/gl4.h>
#include <stdexcept>

#include <ak/render/gl/Buffers.hpp>

using namespace akr::gl;

static GLenum akDataTypeToOGL(DataType dataType) {
	switch(dataType) {
		case DataType::Int8: return GL_BYTE;
		case DataType::Int16: return GL_SHORT;
		case DataType::Int32: return GL_INT;
		case DataType::UInt8: return GL_UNSIGNED_BYTE;
		case DataType::UInt16: return GL_UNSIGNED_SHORT;
		case DataType::UInt32: return GL_UNSIGNED_INT;
		case DataType::Single: return GL_FLOAT;
		case DataType::Double: return GL_DOUBLE;
	}
	throw std::logic_error("Unhandled datatype!");
}

static GLenum akDataTypeToOGL(IDataType dataType) {
	switch(dataType) {
		case IDataType::Int8: return GL_BYTE;
		case IDataType::Int16: return GL_SHORT;
		case IDataType::Int32: return GL_INT;
		case IDataType::UInt8: return GL_UNSIGNED_BYTE;
		case IDataType::UInt16: return GL_UNSIGNED_SHORT;
		case IDataType::UInt32: return GL_UNSIGNED_INT;
	}
	throw std::logic_error("Unhandled datatype!");
}

VertexArray::VertexArray() { glCreateVertexArrays(1, &m_id); }
VertexArray::~VertexArray() { if (m_id) glDeleteVertexArrays(1, &m_id); }

void VertexArray::enableVAttrib(uint32 index, bool state) {
	if (state) glEnableVertexArrayAttrib(m_id, index);
	else glDisableVertexArrayAttrib(m_id, index);
}

void VertexArray::enableVAttribs(const std::vector<uint32>& indicies, bool state) {
	for(auto index : indicies) this->enableVAttrib(index, state);
}

void VertexArray::setVAttribFormat(uint32 index, uint32 sizePerVert, DataType dType, bool normalize, akSize offset) {
	glVertexArrayAttribFormat(m_id, index, sizePerVert, akDataTypeToOGL(dType), normalize, offset);
}

void VertexArray::setVAttribFormats(const std::vector<uint32>& indicies, uint32 sizePerVert, DataType dType, bool normalize, akSize offset) {
	for(auto index : indicies) this->setVAttribFormat(index, sizePerVert, dType, normalize, offset);
}

void VertexArray::setVAttribFormat(uint32 index, uint32 sizePerVert, IDataType dType, akSize offset) {
	glVertexArrayAttribIFormat(m_id, index, sizePerVert, akDataTypeToOGL(dType), offset);
}

void VertexArray::setVAttribFormats(const std::vector<uint32>& indicies, uint32 sizePerVert, IDataType dType, akSize offset) {
	for(auto index : indicies) this->setVAttribFormat(index, sizePerVert, dType, offset);
}

void VertexArray::bindVertexBuffer(uint32 index, const Buffer& buffer, akSize stride, akSize offset) {
	glVertexArrayVertexBuffer(m_id, index, buffer.id(), offset, stride);
}

void VertexArray::bindIndexBuffer(const Buffer& buffer) {
	glVertexArrayElementBuffer(m_id, buffer.id());
}

uint32 VertexArray::id() const {
	return m_id;
}

// //////////////////// //
// // NAMESPACE FUNC // //
// //////////////////// //

void akr::gl::bindVertexArray(const VertexArray& va) {
	glBindVertexArray(va.id());
}

void akr::gl::resetVertexArray() {
	glBindVertexArray(0);
}




