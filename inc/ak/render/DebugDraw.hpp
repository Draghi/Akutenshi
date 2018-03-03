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

#ifndef AK_RENDER_DEBUGDRAW_HPP_
#define AK_RENDER_DEBUGDRAW_HPP_

#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/Buffers.hpp>
#include <ak/render/Draw.hpp>
#include <ak/render/VertexArrays.hpp>
#include <vector>

namespace akrd {
	namespace internal {
		struct VertexData {
			akm::Vec3 position;
			akm::Vec4 colour;
		};
	}

	enum class Primitive {
		Lines,
		Triangles
	};

	enum class Matrix {
		Projection,
		View,
		Model,
	};

	class DisplayList;

	void draw(const DisplayList& displayList);

	void pushMatrix(Matrix mode, const akm::Mat4& mat = akm::Mat4(1));
	void popMatrix(Matrix mode);
	void setMatrix(Matrix mode, const akm::Mat4& mat = akm::Mat4(1));
	const akm::Mat4& getMatrix(Matrix mode);

	void setColour(const akm::Vec3& colour);
	void setColour(const akm::Vec4& colour);
	const akm::Vec4& getColour();

	class DisplayList final {
		friend void akrd::draw(const DisplayList&);
		private:
			Primitive m_primitive = Primitive::Triangles;
			akr::VertexArray m_vao;
			akr::Buffer m_buffer;
			akSize m_vertexCount = 0;
			std::vector<internal::VertexData> m_vertexData;

		public:
			DisplayList() = default;

			DisplayList& begin(Primitive primitive);
			DisplayList& end();

			void draw() { akrd::draw(*this); }

			DisplayList& addVertex(const akm::Vec3& position,  const akm::Vec3& colour);
			DisplayList& addVertex(const akm::Vec3& position,  const akm::Vec4& colour = {1,1,1,1});

			DisplayList& addVertexList(akSize count, const akm::Vec3* positions, const akm::Vec3& colour);
			DisplayList& addVertexList(akSize count, const akm::Vec3* positions, const akm::Vec4& colour = {1,1,1,1});
			DisplayList& addVertexList(akSize count, const akm::Vec3* positions, const akm::Vec3* colours);
			DisplayList& addVertexList(akSize count, const akm::Vec3* positions, const akm::Vec4* colours);

			DisplayList& addVertexPoly(akSize count, const akm::Vec3* positions, const akm::Vec3& colours);
			DisplayList& addVertexPoly(akSize count, const akm::Vec3* positions, const akm::Vec4& colours = {1,1,1,1});
			DisplayList& addVertexPoly(akSize count, const akm::Vec3* positions, const akm::Vec3* colours);
			DisplayList& addVertexPoly(akSize count, const akm::Vec3* positions, const akm::Vec4* colours);
	};

}

#endif
