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
#include <ak/render/Pipeline.hpp>

#include "GL/gl4.h"

using namespace ak::render;


PipelineStage::PipelineStage(StageType stageType) : m_id(0), m_type(stageType), m_src() {
	switch(stageType) {
		case StageType::Fragment: m_id = glCreateShader(GL_FRAGMENT_SHADER); break;
		case StageType::Geometry: m_id = glCreateShader(GL_GEOMETRY_SHADER); break;
		case StageType::Vertex:   m_id = glCreateShader(GL_VERTEX_SHADER);   break;
	}
}

PipelineStage::~PipelineStage() {
	if (m_id != 0) glDeleteShader(m_id);
}

bool PipelineStage::attach(const std::string& src) {
	if (isCompiled()) return false;

	m_src = src;

	const char* srcData = src.data();
	GLint srcSize = static_cast<GLint>(src.size());
	glShaderSource(m_id, 1, &srcData, &srcSize);

	return true;
}

bool PipelineStage::compile() {
	if (m_src.empty()) return false;

	glCompileShader(m_id);

	if (!isCompiled()) {
		std::string logString;
		GLint logLength = 0;
		glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) {
			GLchar* log = new GLchar[logLength+1];
			GLsizei logSize = 0;
			glGetShaderInfoLog(m_id, logLength+1, &logSize, log);
			logString = std::string(log);
		}

		ak::log::Logger("PipelineStage").warn("Shader compilation failed.\n", logString);
	}

	return true;
}

bool PipelineStage::isCompiled() const {
	GLint success = 0;
	glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
	return success == GL_TRUE;
}


Pipeline::Pipeline() : m_id(0), m_attachedShaders(), m_isLinked(false) {
	m_id = glCreateProgram();
}

Pipeline::~Pipeline() {
	glDeleteProgram(m_id);
}

bool Pipeline::attach(const PipelineStage& stage) {
	uint32 typeAsId = static_cast<uint32>(stage.type());
	if (m_attachedShaders[typeAsId]) glDetachShader(m_id, m_attachedShaders[typeAsId]);

	glAttachShader(m_id, stage.id());
	m_attachedShaders[typeAsId] = stage.id();

	return true;
}

bool Pipeline::link() {
	if (m_isLinked) return false;
	glLinkProgram(m_id);
	return true;
}

bool ak::render::resetActivePipeline() { glUseProgram(0); return true; }
bool ak::render::setActivePipeline(const Pipeline& pipeline) { glUseProgram(pipeline.id()); return true; }
