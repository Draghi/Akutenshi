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

#ifndef AK_RENDER_BUFFERS_HPP_
#define AK_RENDER_BUFFERS_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/render/gl/Types.hpp>

namespace akr {
	namespace gl {

		enum BufferHint : uint8 {
			BufferHint_None              = 0x00,
			BufferHint_Dynamic           = 0x01,
			BufferHint_Map_Read          = 0x02,
			BufferHint_Map_Write         = 0x04,
			BufferHint_Map_Persistant_IO = 0x08,
			BufferHint_Map_Coherent_IO   = 0x10,
			BufferHint_Store_Client      = 0x20,
		};

		class Buffer final {
			private:
				uint32 m_id;
				uint8 m_hint;

			public:
				Buffer();
				Buffer(const void* data, akSize size, uint8 hint = BufferHint_None);
				Buffer(Buffer&& other);
				~Buffer();

				bool writeData(const void*  data, akSize size, akSize offset = 0);
				bool writeData(const Buffer& src, akSize size, akSize readOffset = 0, akSize writeOffset = 0);

				bool readData(void* dst, akSize size, akSize offset = 0) const;

				uint32 id() const;
				bool canWrite() const;
				bool isValid() const;

				Buffer& operator=(Buffer&& other);
		};

		void bindBuffer(BufferTarget target, const Buffer& buffer, uint32 index, akSize offset, akSize size);
		void bindBuffer(BufferTarget target, const Buffer& buffer, uint32 index);
		void bindBuffer(BufferTarget target, const Buffer& buffer);
	}
}

#endif
