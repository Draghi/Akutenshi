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

#ifndef AK_RENDER_INTERMEDIATEMODE_HPP_
#define AK_RENDER_INTERMEDIATEMODE_HPP_

namespace akri {

	// //////////// //
	// // Matrix // //
	// //////////// //

	enum class MatrixMode : uint8 {
		Projection = 0,
		View = 1,
		Model = 2
	};

	void matPush();
	void matPop();

	void matSetMode(MatrixMode mode);

	void matOpAdd(const akm::Mat4& val);
	void matOpSub(const akm::Mat4& val);
	void matOpPreMult(const akm::Mat4& val);
	void matOpPostMult(const akm::Mat4& val);

	void matSetIdentity();
	void matSet(const akm::Mat4& val);
	akm::Mat4& matGet();

	// //////////// //
	// // Render // //
	// //////////// //

	enum class Primitive : uint8 {
		Points,

		Lines,
		LinesAdj,
		LineStrip,
		LineStripAdj,
		LineLoop,

		Triangles,
		TrianglesAdj,
		TriangleStrip,
		TriangleStripAdj,
		TriangleFan,
	};

	void begin(Primitive primitive);
	void end();

	void vertex3(const akm::Vec3& position);
	void colour3(const akm::Vec3& colour);

	void setColour(const akm::Vec3& colour);
}

namespace akr { namespace intermediate = akri; }

#endif
