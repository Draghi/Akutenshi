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
#include <ak/render/gl/Shaders.hpp>
#include "GL/gl4.h"

using namespace akr::gl;

static bool getOGLShaderLog(GLuint id, std::string& dst);
static bool getOGLProgramLog(GLuint id, std::string& dst);

// ////////////////// //
// // SHADER STAGE // //
// ////////////////// //

ShaderStage::ShaderStage() : m_id(0), m_type(StageType::Vertex), m_compileLog() {}

ShaderStage::ShaderStage(StageType stageType) : m_id(0), m_type(stageType), m_compileLog() {
	switch(stageType) {
		case StageType::Fragment: m_id = glCreateShader(GL_FRAGMENT_SHADER); break;
		case StageType::Geometry: m_id = glCreateShader(GL_GEOMETRY_SHADER); break;
		case StageType::Vertex:   m_id = glCreateShader(GL_VERTEX_SHADER);   break;
	}
}

ShaderStage::ShaderStage(ShaderStage&& other) : m_id(other.m_id), m_type(other.m_type), m_compileLog(std::move(other.m_compileLog)) {
	other.m_id = 0;
}

ShaderStage::~ShaderStage() {
	if (isValid()) glDeleteShader(m_id);
}


bool ShaderStage::attach(const std::string& src) {
	if (!isValid() || isCompiled()) return false;

	auto* srcData = src.data();
	GLint srcSize = src.size();
	glShaderSource(m_id, 1, &srcData, &srcSize);

	return true;
}

bool ShaderStage::compile() {
	if (!isValid() || isCompiled()) return false;

	glCompileShader(m_id);
	if (isCompiled()) return true;

	getOGLShaderLog(m_id, m_compileLog);
	return false;
}


bool ShaderStage::isCompiled() const {
	GLint success = 0;
	glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
	return success == GL_TRUE;
}

const std::string& ShaderStage::compileLog() const {
	return m_compileLog;
}

void ShaderStage::clearCompileLog() {
	m_compileLog.clear();
}


bool ShaderStage::isValid() const {
	return m_id != 0;
}


uint32 ShaderStage::id() const {
	return m_id;
}

StageType ShaderStage::type() const {
	return m_type;
}

ShaderStage& ShaderStage::operator=(ShaderStage&& other) {
	if (m_id == other.m_id) return *this;
	if (isValid()) glDeleteShader(m_id);
	m_id = other.m_id;
	m_type = other.m_type;
	m_compileLog = std::move(other.m_compileLog);
	other.m_id = 0;
	return *this;
}

// //////////////////// //
// // SHADER PROGRAM // //
// //////////////////// //

ShaderProgram::ShaderProgram() : m_id(0), m_attachedShaders() {
	m_id = glCreateProgram();
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) : m_id(other.m_id), m_attachedShaders(std::move(other.m_attachedShaders)), m_linkLog(std::move(other.m_linkLog)) {
	other.m_id = 0;
}

ShaderProgram::~ShaderProgram() {
	glDeleteProgram(m_id);
}

bool ShaderProgram::attach(const ShaderStage& stage) {
	if (!stage.isValid() || !stage.isCompiled()) return false;

	detach(stage.type());

	uint32 typeAsId = static_cast<uint32>(stage.type());
	glAttachShader(m_id, stage.id());
	m_attachedShaders[typeAsId] = stage.id();

	return true;
}

bool ShaderProgram::detach(StageType stageType) {
	uint32 typeAsId = static_cast<uint32>(stageType);
	if (!m_attachedShaders[typeAsId]) return false;
	glDetachShader(m_id, m_attachedShaders[typeAsId]);
	m_attachedShaders[typeAsId] = 0;
	return true;
}

void ShaderProgram::detachAll() {
	for(auto& val : m_attachedShaders) {
		if (val) glDetachShader(m_id, val);
		val = 0;
	}
}

bool ShaderProgram::link() {
	if (isLinked()) return false;
	if (std::none_of(m_attachedShaders.begin(), m_attachedShaders.end(), [](uint32 val){return val != 0;})) return false;

	glLinkProgram(m_id);

	if (!isLinked()) {
		getOGLProgramLog(m_id, m_linkLog);
		return false;
	}

	detachAll();
	return true;
}

bool ShaderProgram::isLinked() const {
	GLint isLinked = 0;
	glGetProgramiv(m_id, GL_LINK_STATUS, &isLinked);
	return isLinked == GL_TRUE;
}

const std::string& ShaderProgram::linkLog() const {
	return m_linkLog;
}

void ShaderProgram::clearLinkLog() {
	m_linkLog.clear();
}


uint32 ShaderProgram::id() const {
	return m_id;
}

bool ShaderProgram::isValid() const {
	return m_id != 0;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) {
	if ((other.m_id == m_id) || (!m_id)) return *this;
	m_id = other.m_id; other.m_id = 0;
	m_attachedShaders = std::move(other.m_attachedShaders);
	m_linkLog = std::move(other.m_linkLog);
	return *this;
}

void ShaderProgram::setUniform(uint32 index, uint32   x) { glProgramUniform1ui(m_id, index, x); }
void ShaderProgram::setUniform(uint32 index, int32    x) { glProgramUniform1i( m_id, index, x); }
void ShaderProgram::setUniform(uint32 index, fpSingle x) { glProgramUniform1f( m_id, index, x); }

void ShaderProgram::setUniform(uint32 index, uint32   x, uint32   y) { glProgramUniform2ui(m_id, index, x, y); }
void ShaderProgram::setUniform(uint32 index, int32    x, int32    y) { glProgramUniform2i( m_id, index, x, y); }
void ShaderProgram::setUniform(uint32 index, fpSingle x, fpSingle y) { glProgramUniform2f( m_id, index, x, y); }

void ShaderProgram::setUniform(uint32 index, uint32   x, uint32   y, uint32   z) { glProgramUniform3ui(m_id, index, x, y, z); }
void ShaderProgram::setUniform(uint32 index, int32    x, int32    y, int32    z) { glProgramUniform3i( m_id, index, x, y, z); }
void ShaderProgram::setUniform(uint32 index, fpSingle x, fpSingle y, fpSingle z) { glProgramUniform3f( m_id, index, x, y, z); }

void ShaderProgram::setUniform(uint32 index,   uint32 x,   uint32 y,   uint32 z,   uint32 w) { glProgramUniform4ui(m_id, index, x, y, z, w); }
void ShaderProgram::setUniform(uint32 index,    int32 x,    int32 y,    int32 z,    int32 w) { glProgramUniform4i( m_id, index, x, y, z, w); }
void ShaderProgram::setUniform(uint32 index, fpSingle x, fpSingle y, fpSingle z, fpSingle w) { glProgramUniform4f( m_id, index, x, y, z, w); }

void ShaderProgram::setUniform(uint32 index, akm::Vec2 vec) { this->setUniform(index, vec.x, vec.y); }
void ShaderProgram::setUniform(uint32 index, akm::Vec3 vec) { this->setUniform(index, vec.x, vec.y, vec.z); }
void ShaderProgram::setUniform(uint32 index, akm::Vec4 vec) { this->setUniform(index, vec.x, vec.y, vec.z, vec.w); }

void ShaderProgram::setUniform(uint32 index, akm::Mat4 matrix) { glProgramUniformMatrix4fv(m_id, index, 1, GL_FALSE, &matrix[0][0]); }
void ShaderProgram::setUniform(uint32 index, akm::Mat3 matrix) { glProgramUniformMatrix3fv(m_id, index, 1, GL_FALSE, &matrix[0][0]); }
void ShaderProgram::setUniform(uint32 index, akm::Mat2 matrix) { glProgramUniformMatrix2fv(m_id, index, 1, GL_FALSE, &matrix[0][0]); }

// ///////////////////// //
// // NAMESPACE FUNCS // //
// ///////////////////// //

void akr::gl::bindShaderProgram(const ShaderProgram& pipeline) {
	glUseProgram(pipeline.id());
}

void akr::gl::unbindShaderProgram() {
	glUseProgram(0);
}

// ////////////////// //
// // STATIC FUNCS // //
// ////////////////// //

static bool getOGLShaderLog(GLuint id, std::string& dst) {
	std::string logString;
	GLint logLength = 0;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 0) {
		GLchar* log = new GLchar[logLength+1];
		GLsizei logSize = 0;
		glGetShaderInfoLog(id, logLength, &logSize, log);
		dst = std::string(log, logSize);
		return true;
	} else {
		dst.clear();
		return false;
	}
}

static bool getOGLProgramLog(GLuint id, std::string& dst) {
	std::string logString;
	GLint logLength = 0;
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 0) {
		GLchar* log = new GLchar[logLength+1];
		GLsizei logSize = 0;
		glGetProgramInfoLog(id, logLength, &logSize, log);
		dst = std::string(log, logSize);
		return true;
	} else {
		dst.clear();
		return false;
	}
}



