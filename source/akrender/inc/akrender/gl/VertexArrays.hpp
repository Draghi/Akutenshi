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

#ifndef AK_RENDER_GL_VERTEXARRAYS_HPP_
#define AK_RENDER_GL_VERTEXARRAYS_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akrender/gl/Buffers.hpp>
#include <akrender/gl/Types.hpp>
#include <vector>

namespace akr {
	namespace gl {

		class VertexArray final {
			VertexArray(const VertexArray&) = delete;
			VertexArray& operator=(const VertexArray&) = delete;
			private:
				uint32 m_id;
			public:
				VertexArray();
				~VertexArray();

				void enableVAttrib(uint32 index, bool state = true);
				void enableVAttribs(const std::vector<uint32>& indicies, bool state = true);

				void setVAttribFormat(uint32 index, uint32 sizePerVert, DataType dType, bool normalize = false, akSize offset = 0);
				void setVAttribFormats(const std::vector<uint32>& indicies, uint32 sizePerVert, DataType dType, bool normalize = false, akSize offset = 0);

				void setVAttribFormat(uint32 index, uint32 sizePerVert, IDataType dType, akSize offset = 0);
				void setVAttribFormats(const std::vector<uint32>& indicies, uint32 sizePerVert, IDataType dType, akSize offset = 0);

				void bindVertexBuffer(uint32 index, const Buffer& buffer, akSize stride, akSize offset = 0);
				void bindIndexBuffer(const Buffer& buffer);

				uint32 id() const;
		};

		void bindVertexArray(const VertexArray& va);
		void resetVertexArray();
	}
}

#endif
