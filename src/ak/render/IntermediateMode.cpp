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

#include <ak/math/Matrix.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/Buffers.hpp>
#include <ak/render/Draw.hpp>
#include <ak/render/IntermediateMode.hpp>
#include <ak/render/Shaders.hpp>
#include <ak/render/Types.hpp>
#include <ak/render/VertexArrays.hpp>
#include <ak/String.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec3.hpp>
#include <array>
#include <cstddef>
#include <deque>
#include <stdexcept>
#include <vector>

using namespace akri;

// //////////// //
// // Matrix // //
// //////////// //

static akri::MatrixMode matMode = akri::MatrixMode::Projection;
static std::array<std::deque<akm::Mat4>, 3> matricies = {{{akm::Mat4(1)}, {akm::Mat4(1)}, {akm::Mat4(1)}}};

static std::deque<akm::Mat4>& matrixStack() { return matricies[static_cast<uint8>(matMode)]; }
static akm::Mat4& matrix() { return matricies[static_cast<uint8>(matMode)].back(); }

void akri::matPush() { matrixStack().push_back(matrix()); }
void akri::matPop()  { matrixStack().pop_back(); if (matrixStack().size() == 0) matrixStack().push_back(akm::Mat4(1)); }

void akri::matSetMode(MatrixMode mode) { matMode = mode; }

void akri::matOpAdd(const akm::Mat4& val) { matrix() += val; }
void akri::matOpSub(const akm::Mat4& val) { matrix() -= val; }
void akri::matOpPreMult(const akm::Mat4& val) { matrix() = val * matrix(); }
void akri::matOpPostMult(const akm::Mat4& val) { matrix() *= val; }

void akri::matSetIdentity() { matrix() = akm::Mat4(1); }
void akri::matSet(const akm::Mat4& val) { matrix() = val; }

akm::Mat4& akri::matGet() { return matrix(); }

// //////////// //
// // Render // //
// //////////// //

static akr::ShaderProgram& shader() {
	static akr::ShaderProgram progShader = []() {
		akr::ShaderStage vertStage(akr::StageType::Vertex);
		vertStage.attach(
			"#version 450 \n"
			"layout(location = 0) in vec3 vPosition;"
			"layout(location = 1) in vec3 vColour;"
			"layout(location = 0) uniform mat4 uMatTransform;"
			"out vec3 fColour;"
			"void main() {"
			"	fColour = vColour;"
			"	gl_Position = uMatTransform * vec4(vPosition, 1);"
			"}"
		);
		if (!vertStage.compile()) throw std::runtime_error(ak::buildString("Failed to build vertex shader. Error Log:\n", vertStage.compileLog()));

		akr::ShaderStage fragStage(akr::StageType::Fragment);
		fragStage.attach(
			"#version 450 \n"
			"in vec3 fColour;"
			"layout(location = 1) uniform vec3 uColour;"
			"out vec4 fragColour;"
			"void main() {"
			"	fragColour = vec4(fColour*uColour, 1);"
			"}"
		);
		if (!fragStage.compile()) throw std::runtime_error(ak::buildString("Failed to build fragment shader. Error Log:\n", fragStage.compileLog()));

		akr::ShaderProgram prog;
		prog.attach(vertStage);
		prog.attach(fragStage);
		if (!prog.link()) throw std::runtime_error(ak::buildString("Failed to link shader program. Error Log:\n", prog.linkLog()));
		return prog;
	}();
	return progShader;
}

struct VertexData {
	akm::Vec3 position = akm::Vec3(0,0,0);
	akm::Vec3 colour = akm::Vec3(1,1,1);
};

static Primitive primitiveMode = Primitive::Lines;
static bool isRendering = false;

static std::vector<VertexData> vertexData;

static akm::Vec3 uniformColour = akm::Vec3(1,1,1);

void akri::begin(Primitive primitive) {
	isRendering = true;
	primitiveMode = primitive;
	vertexData.clear();
}

void akri::end() {
	if (!isRendering) return;
	isRendering = false;

	shader().setUniform(0, matricies[0].back() * matricies[1].back() * matricies[2].back() );
	shader().setUniform(1, uniformColour);

	akr::VertexArray vaMesh;
	vaMesh.enableVAttribs({0, 1});
	vaMesh.setVAttribFormats({0, 1}, 3, akr::DataType::Single);

	akr::Buffer vbufMeshVerts(vertexData.data(), vertexData.size()*sizeof(VertexData));
	vaMesh.bindVertexBuffer(0, vbufMeshVerts, sizeof(VertexData), offsetof(VertexData, position));
	vaMesh.bindVertexBuffer(1, vbufMeshVerts, sizeof(VertexData), offsetof(VertexData, colour));

	akr::bindShaderProgram(shader());
	akr::bindVertexArray(vaMesh);

	akr::DrawType drawType = [&](){
		switch(primitiveMode) {
			case Primitive::Points:           return akr::DrawType::Points;
			case Primitive::Lines:            return akr::DrawType::Lines;
			case Primitive::LinesAdj:         return akr::DrawType::LinesAdj;
			case Primitive::LineStrip:        return akr::DrawType::LineStrip;
			case Primitive::LineStripAdj:     return akr::DrawType::LineStripAdj;
			case Primitive::LineLoop:         return akr::DrawType::LineLoop;
			case Primitive::Triangles:        return akr::DrawType::Triangles;
			case Primitive::TrianglesAdj:     return akr::DrawType::TrianglesAdj;
			case Primitive::TriangleStrip:    return akr::DrawType::TriangleStrip;
			case Primitive::TriangleStripAdj: return akr::DrawType::TriangleStripAdj;
			case Primitive::TriangleFan:      return akr::DrawType::TriangleFan;
		}
	}();

	akr::draw(drawType, vertexData.size(), 0);
}

void akri::vertex3(const akm::Vec3& position) {
	if (!isRendering) return;
	vertexData.push_back(VertexData());
	vertexData.back().position = position;
}

void akri::colour3(const akm::Vec3& colour) {
	if (!isRendering) return;
	vertexData.back().colour = colour;
}

void akri::setColour(const akm::Vec3& colour) {
	uniformColour = colour;
}












