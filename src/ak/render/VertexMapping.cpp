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

#include <ak/PrimitiveTypes.hpp>
#include <ak/render/VertexMapping.hpp>

#include "GL/gl4.h"

using namespace akr;

VertexMapping::VertexMapping() { glGenVertexArrays(1, &m_id); }
VertexMapping::~VertexMapping() { if (m_id) glDeleteVertexArrays(1, &m_id); }

bool akr::mapVertexBufferF(uint32 index, uint32 sizePerVert, DataType dType, bool normalize, uint32 offset, uint32 stride, bool autoEnable) {
	switch(dType) {
		case DataType::Int8: glVertexAttribPointer(index, sizePerVert, GL_BYTE, normalize, stride, static_cast<char*>(nullptr) + offset); break;
		case DataType::Int16: glVertexAttribPointer(index, sizePerVert, GL_SHORT, normalize, stride, static_cast<char*>(nullptr) + offset); break;
		case DataType::Int32: glVertexAttribPointer(index, sizePerVert, GL_INT, normalize, stride, static_cast<char*>(nullptr) + offset); break;

		case DataType::UInt8: glVertexAttribPointer(index, sizePerVert, GL_UNSIGNED_BYTE, normalize, stride, static_cast<char*>(nullptr) + offset); break;
		case DataType::UInt16: glVertexAttribPointer(index, sizePerVert, GL_UNSIGNED_SHORT, normalize, stride, static_cast<char*>(nullptr) + offset); break;
		case DataType::UInt32: glVertexAttribPointer(index, sizePerVert, GL_UNSIGNED_INT, normalize, stride, static_cast<char*>(nullptr) + offset); break;

		case DataType::Double: glVertexAttribPointer(index, sizePerVert, GL_DOUBLE, normalize, stride, static_cast<char*>(nullptr) + offset); break;
		case DataType::Single: glVertexAttribPointer(index, sizePerVert, GL_FLOAT, normalize, stride, static_cast<char*>(nullptr) + offset); break;
	}
	if (autoEnable) enableVertexMapping(index);
	return true;
}

bool akr::mapVertexBufferI(uint32 index, uint32 sizePerVert, IDataType dType, uint32 offset, uint32 stride, bool autoEnable) {
	switch(dType) {
		case IDataType::Int8: glVertexAttribIPointer(index, sizePerVert, GL_BYTE, stride, static_cast<char*>(nullptr) + offset); break;
		case IDataType::Int16: glVertexAttribIPointer(index, sizePerVert, GL_SHORT, stride, static_cast<char*>(nullptr) + offset); break;
		case IDataType::Int32: glVertexAttribIPointer(index, sizePerVert, GL_INT, stride, static_cast<char*>(nullptr) + offset); break;

		case IDataType::UInt8: glVertexAttribIPointer(index, sizePerVert, GL_UNSIGNED_BYTE, stride, static_cast<char*>(nullptr) + offset); break;
		case IDataType::UInt16: glVertexAttribIPointer(index, sizePerVert, GL_UNSIGNED_SHORT, stride, static_cast<char*>(nullptr) + offset); break;
		case IDataType::UInt32: glVertexAttribIPointer(index, sizePerVert, GL_UNSIGNED_INT, stride, static_cast<char*>(nullptr) + offset); break;
	}
	if (autoEnable) enableVertexMapping(index);
	return true;
}

void akr::enableVertexMapping(uint32 index) {
	glEnableVertexAttribArray(index);
}

void akr::disableVertexMapping(uint32 index) {
	glDisableVertexAttribArray(index);
}

void akr::bind(const VertexMapping& mapping) {
	glBindVertexArray(mapping.id());
}



