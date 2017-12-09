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

#ifndef AK_RENDER_BUFFER_HPP_
#define AK_RENDER_BUFFER_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/render/Types.hpp>

namespace ak {
	namespace render {

		class Buffer final {
			private:
				uint32 m_id;
			public:
				Buffer();
				Buffer(Buffer&& other) : m_id(other.m_id) { other.m_id = 0; }
				~Buffer();

				uint32 id() const { return m_id; }
		};

		void bind(BufferTarget target, const Buffer& buffer);

		void setDataRaw(BufferTarget target, const void* data, uint32 size);
		template<typename type_t> void setData(BufferTarget target, const type_t* data, uint32 count) { setDataRaw(target, data, sizeof(type_t)*count); }

		void replaceDataRaw(BufferTarget target, uint32 offset, const void* data, uint32 size);
		template<typename type_t> void replaceData(BufferTarget target, uint32 offset, const type_t* data, uint32 count) { replaceDataRaw(target, sizeof(type_t)*offset, data, sizeof(type_t)*count); }
		template<typename type_t> void replaceDataB(BufferTarget target, uint32 offset, const type_t* data, uint32 count) { replaceDataRaw(target, offset, data, sizeof(type_t)*count); }

	}
}

namespace akr = ak::render;

#endif
