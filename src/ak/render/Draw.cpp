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
#include <ak/render/Draw.hpp>

#include "GL/gl4.h"

#if defined(__linux)
#define BACKWARD_HAS_BFD 1
#include <signal.h>
#ifndef SIGUNUSED
#define SIGUNUSED 31
#endif
#include "backward.hpp"
namespace backward { static backward::SignalHandling sh; }
#endif

using namespace akr;

static void APIENTRY ogl_logErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* /*userParam​*/) {
	akl::Logger glLog("OpenGL");
	std::stringstream sstream;

	switch(type) {
		case GL_DEBUG_TYPE_ERROR:               sstream << "[Error]"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: sstream << "[Deprecated]"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  sstream << "[Undefined]"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         sstream << "[Portability]"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         sstream << "[Performance]"; break;
		case GL_DEBUG_TYPE_MARKER:              sstream << "[Marker]"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          sstream << "[Push]"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           sstream << "[Pop]"; break;
		case GL_DEBUG_TYPE_OTHER:               sstream << "[Other]"; break;
	}

	switch(source) {
		case GL_DEBUG_SOURCE_API:             sstream << "[API]"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sstream << "[Window]"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: sstream << "[Shader]"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     sstream << "[3rd Party]"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     sstream << "[App]"; break;
		case GL_DEBUG_SOURCE_OTHER:           sstream << "[Other]"; break;
	}

	sstream << "[Error Code: " << id << "] " << std::string(message, static_cast<size_t>(length)) << std::endl;

	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		backward::StackTrace st; st.load_here(32);
		backward::Printer p; p.print(st, sstream);
	}

	switch(severity) {
		case GL_DEBUG_SEVERITY_HIGH:         glLog.error(sstream.str()); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       glLog.warn( sstream.str()); break;
		case GL_DEBUG_SEVERITY_LOW:          glLog.info( sstream.str()); break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: glLog.debug(sstream.str()); break;
	}
}

void akr::init() {
	constexpr ak::log::Logger log("Render::Init");

	static bool hasInit = false;
	if (hasInit) return;
	if (ogl_LoadFunctions() == ogl_LoadStatus::ogl_LOAD_FAILED) return;

	glDebugMessageCallback(ogl_logErrorCallback, nullptr);
	glEnable(GL_DEBUG_OUTPUT);

	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glClearDepth(1.0f);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if (!ogl_ext_EXT_texture_filter_anisotropic) log.warn("Failed to load EXT_texture_filter_anisotropic");

	hasInit = true;
}

void akr::draw(DrawType mode, uint32 vertexCount, uint32 offset) {
	switch(mode) {
		case DrawType::Points: glDrawArrays(GL_POINTS, offset, vertexCount); break;

		case DrawType::Lines:        glDrawArrays(GL_LINES, offset, vertexCount); break;
		case DrawType::LinesAdj:     glDrawArrays(GL_LINES_ADJACENCY, offset, vertexCount); break;
		case DrawType::LineStrip:    glDrawArrays(GL_LINE_STRIP, offset, vertexCount); break;
		case DrawType::LineStripAdj: glDrawArrays(GL_LINE_STRIP_ADJACENCY, offset, vertexCount); break;
		case DrawType::LineLoop:     glDrawArrays(GL_LINE_LOOP, offset, vertexCount); break;

		case DrawType::Triangles:        glDrawArrays(GL_TRIANGLES, offset, vertexCount); break;
		case DrawType::TrianglesAdj:     glDrawArrays(GL_TRIANGLES_ADJACENCY, offset, vertexCount); break;
		case DrawType::TriangleStrip:    glDrawArrays(GL_TRIANGLE_STRIP, offset, vertexCount); break;
		case DrawType::TriangleStripAdj: glDrawArrays(GL_TRIANGLE_STRIP_ADJACENCY, offset, vertexCount); break;
		case DrawType::TriangleFan:      glDrawArrays(GL_TRIANGLE_FAN, offset, vertexCount); break;
	}
}

void akr::drawIndexed(DrawType mode, IDataType indexType, uint32 vertexCount, uint32 offset) {

	uint32 dataType = 0;
	switch(indexType) {
		case IDataType::Int8: dataType = GL_BYTE; break;
		case IDataType::Int16: dataType = GL_SHORT; break;
		case IDataType::Int32: dataType = GL_INT; break;

		case IDataType::UInt8: dataType = GL_UNSIGNED_BYTE; break;
		case IDataType::UInt16: dataType = GL_UNSIGNED_SHORT; break;
		case IDataType::UInt32: dataType = GL_UNSIGNED_INT; break;
	}


	switch(mode) {
		case DrawType::Points:           glDrawElements(GL_POINTS,                   vertexCount, dataType, static_cast<char*>(nullptr) + offset); break;
		case DrawType::Lines:            glDrawElements(GL_LINES,                    vertexCount, dataType, static_cast<char*>(nullptr) + offset); break;
		case DrawType::LinesAdj:         glDrawElements(GL_LINES_ADJACENCY,          vertexCount, dataType, static_cast<char*>(nullptr) + offset); break;
		case DrawType::LineStrip:        glDrawElements(GL_LINE_STRIP,               vertexCount, dataType, static_cast<char*>(nullptr) + offset); break;
		case DrawType::LineStripAdj:     glDrawElements(GL_LINE_STRIP_ADJACENCY,     vertexCount, dataType, static_cast<char*>(nullptr) + offset); break;
		case DrawType::LineLoop:         glDrawElements(GL_LINE_LOOP,                vertexCount, dataType, static_cast<char*>(nullptr) + offset); break;
		case DrawType::Triangles:        glDrawElements(GL_TRIANGLES,                vertexCount, dataType, static_cast<char*>(nullptr) + offset); break;
		case DrawType::TrianglesAdj:     glDrawElements(GL_TRIANGLES_ADJACENCY,      vertexCount, dataType, static_cast<char*>(nullptr) + offset); break;
		case DrawType::TriangleStrip:    glDrawElements(GL_TRIANGLE_STRIP,           vertexCount, dataType, static_cast<char*>(nullptr) + offset); break;
		case DrawType::TriangleStripAdj: glDrawElements(GL_TRIANGLE_STRIP_ADJACENCY, vertexCount, dataType, static_cast<char*>(nullptr) + offset); break;
		case DrawType::TriangleFan:      glDrawElements(GL_TRIANGLE_FAN,             vertexCount, dataType, static_cast<char*>(nullptr) + offset); break;
	}
}

void akr::clear(ClearMode clearMode) {
	constexpr auto COLOUR_BUFFER  = static_cast<uint32>(ClearMode::Colour);
	constexpr auto DEPTH_BUFFER   = static_cast<uint32>(ClearMode::Depth);
	constexpr auto STENCIL_BUFFER = static_cast<uint32>(ClearMode::Stencil);

	uint32 iClearMode = static_cast<uint32>(clearMode);
	uint32 mask = 0x00;
	mask |= ((iClearMode & COLOUR_BUFFER)  == COLOUR_BUFFER)  ? GL_COLOR_BUFFER_BIT   : 0;
	mask |= ((iClearMode & DEPTH_BUFFER)   == DEPTH_BUFFER)   ? GL_DEPTH_BUFFER_BIT   : 0;
	mask |= ((iClearMode & STENCIL_BUFFER) == STENCIL_BUFFER) ? GL_STENCIL_BUFFER_BIT : 0;

	if (mask) glClear(mask);
}

void akr::setClearColour(fpSingle red, fpSingle green, fpSingle blue, fpSingle alpha) {
	glClearColor(red, green, blue, alpha);
}

void akr::setClearDepth(fpSingle depth) {
	glClearDepth(depth);
}

void akr::setClearStencil(int32 stencil) {
	glClearStencil(stencil);
}

void akr::setFillMode(FillMode fillMode, Face face) {
	uint32 glFillMode = GL_FILL;
	switch(fillMode) {
		case FillMode::Point: glFillMode = GL_POINT; break;
		case FillMode::Line:  glFillMode = GL_LINE;  break;
		case FillMode::Fill:  glFillMode = GL_FILL;  break;
	}

	switch(face) {
		case Face::Front:        glPolygonMode(GL_FRONT,          glFillMode); break;
		case Face::Back:         glPolygonMode(GL_BACK,           glFillMode); break;
		case Face::FrontAndBack: glPolygonMode(GL_FRONT_AND_BACK, glFillMode); break;
	}
}

void akr::enableDepthTest(bool state) {
	if (state) glEnable(GL_DEPTH_TEST);
	else glDisable(GL_DEPTH_TEST);
}

void akr::enableCullFace(bool state) {
	if (state) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
}

void akr::setDepthTestMode(DepthMode depthMode) {
	switch(depthMode) {
		case DepthMode::Never:        glDepthFunc(GL_NEVER);    break;
		case DepthMode::Always:       glDepthFunc(GL_ALWAYS);   break;
		case DepthMode::Less:         glDepthFunc(GL_LESS);     break;
		case DepthMode::LessEqual:    glDepthFunc(GL_LEQUAL);   break;
		case DepthMode::Equal:        glDepthFunc(GL_EQUAL);    break;
		case DepthMode::NotEqual:     glDepthFunc(GL_NOTEQUAL); break;
		case DepthMode::Greater:      glDepthFunc(GL_GREATER);  break;
		case DepthMode::GreaterEqual: glDepthFunc(GL_GEQUAL);   break;
	}
}

void akr::setCullFaceMode(CullMode cullMode) {
	switch(cullMode) {
		case CullMode::Front:        glCullFace(GL_FRONT);          break;
		case CullMode::Back:         glCullFace(GL_BACK);           break;
		case CullMode::FrontAndBack: glCullFace(GL_FRONT_AND_BACK); break;
	}
}

void akr::setUniform(uint32 bindingLocation, uint32 x) {
	glUniform1ui(bindingLocation, x);
}

void akr::setUniform(uint32 bindingLocation, uint32 x, uint32 y) {
	glUniform2ui(bindingLocation, x, y);
}

void akr::setUniform(uint32 bindingLocation, uint32 x, uint32 y, uint32 z) {
	glUniform3ui(bindingLocation, x, y, z);
}

void akr::setUniform(uint32 bindingLocation, uint32 x, uint32 y, uint32 z, uint32 w) {
	glUniform4ui(bindingLocation, x, y, z, w);
}


void akr::setUniform(uint32 bindingLocation, int32 x) {
	glUniform1i(bindingLocation, x);
}

void akr::setUniform(uint32 bindingLocation, int32 x, int32 y) {
	glUniform2i(bindingLocation, x, y);
}

void akr::setUniform(uint32 bindingLocation, int32 x, int32 y, int32 z) {
	glUniform3i(bindingLocation, x, y, z);
}

void akr::setUniform(uint32 bindingLocation, int32 x, int32 y, int32 z, int32 w) {
	glUniform4i(bindingLocation, x, y, z, w);
}


void akr::setUniform(uint32 bindingLocation, fpSingle x) {
	glUniform1f(bindingLocation, x);
}

void akr::setUniform(uint32 bindingLocation, fpSingle x, fpSingle y) {
	glUniform2f(bindingLocation, x, y);
}

void akr::setUniform(uint32 bindingLocation, fpSingle x, fpSingle y, fpSingle z) {
	glUniform3f(bindingLocation, x, y, z);
}

void akr::setUniform(uint32 bindingLocation, fpSingle x, fpSingle y, fpSingle z, fpSingle w) {
	glUniform4f(bindingLocation, x, y, z, w);
}


void akr::setUniform(uint32 bindingLocation, akm::Mat4 matrix) {
	glUniformMatrix4fv(bindingLocation, 1, GL_FALSE, &matrix[0][0]);
}

void akr::setUniform(uint32 bindingLocation, akm::Mat3 matrix) {
	glUniformMatrix3fv(bindingLocation, 1, GL_FALSE, &matrix[0][0]);
}

void akr::setUniform(uint32 bindingLocation, akm::Mat2 matrix) {
	glUniformMatrix2fv(bindingLocation, 1, GL_FALSE, &matrix[0][0]);
}
