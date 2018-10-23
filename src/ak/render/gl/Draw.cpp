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

#include <ak/render/gl/Draw.hpp>

#include <GL/gl4.h>
#include <stdint.h>
#include <sstream>
#include <stdexcept>
#include <string>

#include <ak/Log.hpp>

namespace akr {
	namespace gl {
		class Buffer;
	}
}

#if defined(__linux)
#define BACKWARD_HAS_BFD 1
#include "backward.hpp"
#endif

using namespace akr::gl;

static void APIENTRY ogl_logErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* /*userParam​*/) {
	akl::Logger glLog("OGL");
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

		case GL_DEBUG_TYPE_OTHER:
			//sstream << "[Other]"; break;
			return;
	}

	switch(source) {
		case GL_DEBUG_SOURCE_API:             sstream << "[API]"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sstream << "[Window]"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: sstream << "[Shader]"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     sstream << "[3rd Party]"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     sstream << "[App]"; break;
		case GL_DEBUG_SOURCE_OTHER:           sstream << "[Other]"; break;
	}

	sstream << "[Error Code: " << id << "] " << std::string(message, length);

#if defined(__linux)
	if (severity == GL_DEBUG_SEVERITY_HIGH) {
		sstream << std::endl;
		backward::StackTrace st; st.load_here(32);
		backward::Printer p; p.print(st, sstream);
	}
#endif

	switch(severity) {
		case GL_DEBUG_SEVERITY_HIGH:         glLog.error(sstream.str()); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       glLog.warn( sstream.str()); break;
		case GL_DEBUG_SEVERITY_LOW:          glLog.info( sstream.str()); break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: glLog.debug(sstream.str()); break;
	}
}

void akr::gl::init() {
	constexpr akl::Logger log("Render::Init");

	static bool hasInit = false;
	if (hasInit) return;
	if (ogl_LoadFunctions() == ogl_LoadStatus::ogl_LOAD_FAILED) throw std::runtime_error("Failed to load OpenGL functions.");

	glDebugMessageCallback(ogl_logErrorCallback, nullptr);
	glEnable(GL_DEBUG_OUTPUT);

	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glClearDepth(1.0f);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);;
	//glEnable(GL_FRAMEBUFFER_SRGB);

	if (!ogl_ext_EXT_texture_filter_anisotropic) log.warn("Failed to load EXT_texture_filter_anisotropic");

	hasInit = true;
}

void akr::gl::setViewport(const akm::Vec2& offset, const akm::Vec2& size) {
	glViewport(offset.x, offset.y, size.x, size.y);
}

void akr::gl::draw(DrawType mode, uint32 vertexCount, uint32 offset) {
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

void akr::gl::drawIndexed(DrawType mode, IDataType indexType, uint32 vertexCount, uint32 offset) {

	uint32 dataType = 0;
	switch(indexType) {
		case IDataType::Int8: dataType = GL_BYTE; break;
		case IDataType::Int16: dataType = GL_SHORT; break;
		case IDataType::Int32: dataType = GL_INT; break;

		case IDataType::UInt8: dataType = GL_UNSIGNED_BYTE; break;
		case IDataType::UInt16: dataType = GL_UNSIGNED_SHORT; break;
		case IDataType::UInt32: dataType = GL_UNSIGNED_INT; break;
	}

	std::intptr_t offsetIntPtr = offset;
	switch(mode) {
		case DrawType::Points:           glDrawElements(GL_POINTS,                   vertexCount, dataType, reinterpret_cast<char*>(offsetIntPtr)); break;
		case DrawType::Lines:            glDrawElements(GL_LINES,                    vertexCount, dataType, reinterpret_cast<char*>(offsetIntPtr)); break;
		case DrawType::LinesAdj:         glDrawElements(GL_LINES_ADJACENCY,          vertexCount, dataType, reinterpret_cast<char*>(offsetIntPtr)); break;
		case DrawType::LineStrip:        glDrawElements(GL_LINE_STRIP,               vertexCount, dataType, reinterpret_cast<char*>(offsetIntPtr)); break;
		case DrawType::LineStripAdj:     glDrawElements(GL_LINE_STRIP_ADJACENCY,     vertexCount, dataType, reinterpret_cast<char*>(offsetIntPtr)); break;
		case DrawType::LineLoop:         glDrawElements(GL_LINE_LOOP,                vertexCount, dataType, reinterpret_cast<char*>(offsetIntPtr)); break;
		case DrawType::Triangles:        glDrawElements(GL_TRIANGLES,                vertexCount, dataType, reinterpret_cast<char*>(offsetIntPtr)); break;
		case DrawType::TrianglesAdj:     glDrawElements(GL_TRIANGLES_ADJACENCY,      vertexCount, dataType, reinterpret_cast<char*>(offsetIntPtr)); break;
		case DrawType::TriangleStrip:    glDrawElements(GL_TRIANGLE_STRIP,           vertexCount, dataType, reinterpret_cast<char*>(offsetIntPtr)); break;
		case DrawType::TriangleStripAdj: glDrawElements(GL_TRIANGLE_STRIP_ADJACENCY, vertexCount, dataType, reinterpret_cast<char*>(offsetIntPtr)); break;
		case DrawType::TriangleFan:      glDrawElements(GL_TRIANGLE_FAN,             vertexCount, dataType, reinterpret_cast<char*>(offsetIntPtr)); break;
	}
}

void akr::gl::clear(ClearMode clearMode) {
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

void akr::gl::setClearColour(fpSingle red, fpSingle green, fpSingle blue, fpSingle alpha) {
	glClearColor(red, green, blue, alpha);
}

void akr::gl::setClearDepth(fpSingle depth) {
	glClearDepth(depth);
}

void akr::gl::setClearStencil(int32 stencil) {
	glClearStencil(stencil);
}

void akr::gl::setFillMode(FillMode fillMode, Face face) {
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

void akr::gl::enableDepthTest(bool state) {
	if (state) glEnable(GL_DEPTH_TEST);
	else glDisable(GL_DEPTH_TEST);
}

void akr::gl::enableCullFace(bool state) {
	if (state) glEnable(GL_CULL_FACE);
	else glDisable(GL_CULL_FACE);
}

void akr::gl::setDepthTestMode(DepthMode depthMode) {
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

void akr::gl::setCullFaceMode(CullMode cullMode) {
	switch(cullMode) {
		case CullMode::Front:        glCullFace(GL_FRONT);          break;
		case CullMode::Back:         glCullFace(GL_BACK);           break;
		case CullMode::FrontAndBack: glCullFace(GL_FRONT_AND_BACK); break;
	}
}


