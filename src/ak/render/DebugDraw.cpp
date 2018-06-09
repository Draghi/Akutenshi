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

#include <ak/math/Types.hpp>
#include <ak/render/DebugDraw.hpp>
#include <ak/render/gl/Shaders.hpp>
#include <ak/render/gl/Types.hpp>
#include <ak/String.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>
#include <cstddef>
#include <deque>
#include <stdexcept>
#include <vector>

using namespace akr::gl;
using namespace akrd;

static ShaderProgram& shader() {
	static ShaderProgram progShader = []() {
		ShaderStage vertStage(StageType::Vertex);
		vertStage.attach(
			"#version 450 \n"
			"layout(location = 0) in vec3 vPosition;"
			"layout(location = 1) in vec4 vColour;"
			"layout(location = 0) uniform mat4 uMatTransform;"
			"out vec4 fColour;"
			"void main() {"
			"	fColour = vColour;"
			"	gl_Position = uMatTransform * vec4(vPosition, 1);"
			"}"
		);
		if (!vertStage.compile()) throw std::runtime_error(ak::buildString("Failed to build vertex shader. Error Log:\n", vertStage.compileLog()));

		ShaderStage fragStage(StageType::Fragment);
		fragStage.attach(
			"#version 450 \n"
			"in vec4 fColour;"
			"layout(location = 1) uniform vec4 uColour;"
			"out vec4 fragColour;"
			"void main() {"
			"	fragColour = fColour*uColour;"
			"}"
		);
		if (!fragStage.compile()) throw std::runtime_error(ak::buildString("Failed to build fragment shader. Error Log:\n", fragStage.compileLog()));

		ShaderProgram prog;
		prog.attach(vertStage);
		prog.attach(fragStage);
		if (!prog.link()) throw std::runtime_error(ak::buildString("Failed to link shader program. Error Log:\n", prog.linkLog()));
		return prog;
	}();
	return progShader;
}

static std::deque<akm::Mat4> uMatProj  = {{akm::Mat4(1)}};
static std::deque<akm::Mat4> uMatView  = {{akm::Mat4(1)}};
static std::deque<akm::Mat4> uMatModel = {{akm::Mat4(1)}};
static akm::Vec4 uColour = {{akm::Vec4(1,1,1,1)}};

static akm::Mat4 projViewCache = akm::Mat4(1);
static bool isProjViewDirty = false;

void akrd::draw(const DisplayList& displayList) {
	if (isProjViewDirty) {
		projViewCache = uMatProj.back() * uMatView.back();
		isProjViewDirty = false;
	}

	shader().setUniform(0, projViewCache * uMatModel.back());
	shader().setUniform(1, uColour);

	bindShaderProgram(shader());
	bindVertexArray(displayList.m_vao);
	switch(displayList.m_primitive) {
		case Primitive::Lines: {
			akr::gl::draw(DrawType::Lines, displayList.m_vertexCount, 0);
		} return;
		case Primitive::Triangles: {
			akr::gl::draw(DrawType::Triangles, displayList.m_vertexCount, 0);
		} return;
	}
}

void akrd::pushMatrix(Matrix mode, const akm::Mat4& mat) {
	switch(mode) {
		case Matrix::Projection: uMatProj.push_back(mat); isProjViewDirty = true; return;
		case Matrix::View:       uMatView.push_back(mat); isProjViewDirty = true; return;
		case Matrix::Model:     uMatModel.push_back(mat); return;
	}
}

void akrd::popMatrix(Matrix mode) {
	switch(mode) {
		case Matrix::Projection: uMatProj.pop_back(); isProjViewDirty = true; if (uMatProj.empty())  pushMatrix(mode, akm::Mat4(1)); return;
		case Matrix::View:       uMatView.pop_back(); isProjViewDirty = true; if (uMatView.empty())  pushMatrix(mode, akm::Mat4(1)); return;
		case Matrix::Model:     uMatModel.pop_back(); if (uMatModel.empty()) pushMatrix(mode, akm::Mat4(1)); return;
	}
}

void akrd::setMatrix(Matrix mode, const akm::Mat4& mat) {
	switch(mode) {
		case Matrix::Projection: uMatProj.back() = mat; isProjViewDirty = true; return;
		case Matrix::View:       uMatView.back() = mat; isProjViewDirty = true; return;
		case Matrix::Model:     uMatModel.back() = mat; return;
	}
}

const akm::Mat4& akrd::getMatrix(Matrix mode) {
	switch(mode) {
		case Matrix::Projection: return  uMatProj.back();
		case Matrix::View:       return  uMatView.back();
		case Matrix::Model:      return uMatModel.back();
	}
}

void akrd::setColour(const akm::Vec3& colour) { uColour = akm::Vec4(colour, 1); }
void akrd::setColour(const akm::Vec4& colour) { uColour = colour; }
const akm::Vec4& akrd::getColour() { return uColour; }

// ///////////////// //
// // DisplayList // //
// ///////////////// //

DisplayList& DisplayList::begin(Primitive primitive) {
	m_primitive = primitive;
	m_buffer = Buffer();
	m_vertexCount = 0;
	m_vertexData.clear();

	return *this;
}

DisplayList& DisplayList::end() {
	if (m_buffer.isValid()) throw std::logic_error("akrd::DisplayList::addVertex: Cannot re-finalize a finalized displaylist.");

	m_vertexCount = m_vertexData.size();
	m_buffer = Buffer(m_vertexData.data(), sizeof(akrd::internal::VertexData)*m_vertexCount);
	m_vertexData.clear();

	m_vao.enableVAttribs({0, 1});
	m_vao.setVAttribFormat(0, 3, DataType::Single);
	m_vao.setVAttribFormat(1, 4, DataType::Single);
	m_vao.bindVertexBuffer(0, m_buffer, sizeof(akrd::internal::VertexData), offsetof(akrd::internal::VertexData, position));
	m_vao.bindVertexBuffer(1, m_buffer, sizeof(akrd::internal::VertexData), offsetof(akrd::internal::VertexData, colour));

	return *this;
}

DisplayList& DisplayList::addVertex(const akm::Vec3& position, const akm::Vec3& colour) { return addVertex(position, akm::Vec4(colour, 1)); }
DisplayList& DisplayList::addVertex(const akm::Vec3& position, const akm::Vec4& colour) {
	if (m_buffer.isValid()) throw std::logic_error("akrd::DisplayList::addVertex: Cannot add vertex to finalized displaylist.");
	m_vertexData.push_back({position, colour});
	return *this;
}

DisplayList& DisplayList::addVertexList(akSize count, const akm::Vec3* positions, const akm::Vec3& colour) {
	if (m_buffer.isValid()) throw std::logic_error("akrd::DisplayList::addVertexList: Cannot add vertex to finalized displaylist.");
	for(auto i = 0u; i < count; i++) addVertex(positions[i], akm::Vec4(colour, 1));
	return *this;
}

DisplayList& DisplayList::addVertexList(akSize count, const akm::Vec3* positions, const akm::Vec4& colour) {
	if (m_buffer.isValid()) throw std::logic_error("akrd::DisplayList::addVertexList: Cannot add vertex to finalized displaylist.");
	for(auto i = 0u; i < count; i++) addVertex(positions[i], colour);
	return *this;
}

DisplayList& DisplayList::addVertexList(akSize count, const akm::Vec3* positions, const akm::Vec3* colours) {
	if (m_buffer.isValid()) throw std::logic_error("akrd::DisplayList::addVertexList: Cannot add vertex to finalized displaylist.");
	for(auto i = 0u; i < count; i++) addVertex(positions[i], colours ? colours[i] : akm::Vec3(1,1,1));
	return *this;
}

DisplayList& DisplayList::addVertexList(akSize count, const akm::Vec3* positions, const akm::Vec4* colours) {
	if (m_buffer.isValid()) throw std::logic_error("akrd::DisplayList::addVertexList: Cannot add vertex to finalized displaylist.");
	for(auto i = 0u; i < count; i++) addVertex(positions[i], colours ? colours[i] : akm::Vec4(1,1,1,1));
	return *this;
}


DisplayList& DisplayList::addVertexPoly(akSize count, const akm::Vec3* positions, const akm::Vec3& colour) {
	if (m_buffer.isValid()) throw std::logic_error("akrd::DisplayList::addVertexPoly: Cannot add vertex to finalized displaylist.");
	if (count <= 2) throw std::logic_error("akrd::DisplayList::addVertexPoly: Cannot create polygon with 2 or fewer vertices");

	switch(m_primitive) {
		case Primitive::Lines: {
			for(auto i = 1u; i < count; i++) {
				addVertex(positions[i - 1], akm::Vec4(colour, 1));
				addVertex(positions[i],     akm::Vec4(colour, 1));
			}
			addVertex(positions[count - 1], akm::Vec4(colour, 1));
			addVertex(positions[0],         akm::Vec4(colour, 1));
		} break;

		case Primitive::Triangles: {
			for(auto i = 2u; i < count; i++) {
				addVertex(positions[0],   akm::Vec4(colour, 1));
				addVertex(positions[i],   akm::Vec4(colour, 1));
				addVertex(positions[i-1], akm::Vec4(colour, 1));
			}
		} break;
	}

	return *this;
}

DisplayList& DisplayList::addVertexPoly(akSize count, const akm::Vec3* positions, const akm::Vec4& colour) {
	if (m_buffer.isValid()) throw std::logic_error("akrd::DisplayList::addVertexPoly: Cannot add vertex to finalized displaylist.");
	if (count <= 2) throw std::logic_error("akrd::DisplayList::addVertexPoly: Cannot create polygon with 2 or fewer vertices");

	switch(m_primitive) {
		case Primitive::Lines: {
			for(auto i = 1u; i < count; i++) {
				addVertex(positions[i - 1], colour);
				addVertex(positions[i],     colour);
			}
			addVertex(positions[count - 1], colour);
			addVertex(positions[0],         colour);
		} break;

		case Primitive::Triangles: {
			for(auto i = 2u; i < count; i++) {
				addVertex(positions[0],   colour);
				addVertex(positions[i],   colour);
				addVertex(positions[i-1], colour);
			}
		} break;
	}

	return *this;
}

DisplayList& DisplayList::addVertexPoly(akSize count, const akm::Vec3* positions, const akm::Vec3* colours) {
	if (m_buffer.isValid()) throw std::logic_error("akrd::DisplayList::addVertexPoly: Cannot add vertex to finalized displaylist.");
	if (count <= 2) throw std::logic_error("akrd::DisplayList::addVertexPoly: Cannot create polygon with 2 or fewer vertices");

	switch(m_primitive) {
		case Primitive::Lines: {
			for(auto i = 1u; i < count; i++) {
				addVertex(positions[i - 1], colours ? akm::Vec4(colours[i - 1], 1) : akm::Vec4(1,1,1,1));
				addVertex(positions[i],     colours ? akm::Vec4(colours[i]    , 1) : akm::Vec4(1,1,1,1));
			}
			addVertex(positions[count - 1], colours ? akm::Vec4(colours[count -1], 1) : akm::Vec4(1,1,1,1));
			addVertex(positions[0],         colours ? akm::Vec4(colours[0]       , 1) : akm::Vec4(1,1,1,1));
		} break;

		case Primitive::Triangles: {
			for(auto i = 2u; i < count; i++) {
				addVertex(positions[0],   colours ? akm::Vec4(colours[0]  , 1) : akm::Vec4(1,1,1,1));
				addVertex(positions[i],   colours ? akm::Vec4(colours[i]  , 1) : akm::Vec4(1,1,1,1));
				addVertex(positions[i-1], colours ? akm::Vec4(colours[i-1], 1) : akm::Vec4(1,1,1,1));
			}
		} break;
	}

	return *this;
}

DisplayList& DisplayList::addVertexPoly(akSize count, const akm::Vec3* positions, const akm::Vec4* colours) {
	if (m_buffer.isValid()) throw std::logic_error("akrd::DisplayList::addVertexPoly: Cannot add vertex to finalized displaylist.");
	if (count <= 2) throw std::logic_error("akrd::DisplayList::addVertexPoly: Cannot create polygon with 2 or fewer vertices");

	switch(m_primitive) {
		case Primitive::Lines: {
			for(auto i = 1u; i < count; i++) {
				addVertex(positions[i - 1], colours ? colours[i - 1] : akm::Vec4(1,1,1,1));
				addVertex(positions[i],     colours ? colours[i]     : akm::Vec4(1,1,1,1));
			}
			addVertex(positions[count - 1], colours ? colours[count -1] : akm::Vec4(1,1,1,1));
			addVertex(positions[0],         colours ? colours[0]        : akm::Vec4(1,1,1,1));
		} break;

		case Primitive::Triangles: {
			for(auto i = 2u; i < count; i++) {
				addVertex(positions[0],   colours ? colours[0]   : akm::Vec4(1,1,1,1));
				addVertex(positions[i],   colours ? colours[i]   : akm::Vec4(1,1,1,1));
				addVertex(positions[i-1], colours ? colours[i-1] : akm::Vec4(1,1,1,1));
			}
		} break;
	}

	return *this;
}



