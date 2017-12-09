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

#ifndef AK_RENDER_TYPES_HPP_
#define AK_RENDER_TYPES_HPP_

namespace ak {
	namespace render {

		enum class BufferTarget {
			VARRYING,
			UNIFORM,
			INDEX,
		};

		enum class StageType : uint8 {
			Vertex = 0,
			Geometry = 1,
			Fragment = 2,
		};

		enum class DrawType {
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

		enum class Face {
			Front,
			Back,
			FrontAndBack
		};

		enum class FillMode {
			Point,
			Line,
			Fill
		};

		enum class ClearMode : uint8 {
			Colour  = 0x01,
			Depth   = 0x02,
			Stencil = 0x04,

			ColourAndDepth = Colour | Depth,
			ColourAndStencil = Colour | Stencil,

			DepthAndStencil = Colour | Stencil,

			All = Colour | Depth | Stencil,
		};

		enum class CullMode {
			Front,
			Back,
			FrontAndBack
		};

		enum class DepthMode {
			Never,
			Always,
			Less,
			LessEqual,
			Equal,
			NotEqual,
			Greater,
			GreaterEqual
		};

		enum class IDataType {
			Int8,
			Int16,
			Int32,

			UInt8,
			UInt16,
			UInt32,
		};

		enum class DataType {
			Int8,
			Int16,
			Int32,

			UInt8,
			UInt16,
			UInt32,

			FPDouble,
			FPSingle
		};
	}
}

#endif
