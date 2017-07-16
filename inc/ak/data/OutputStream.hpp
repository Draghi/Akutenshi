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

#ifndef AK_DATA_OUTPUTSTREAM_HPP_
#define AK_DATA_OUTPUTSTREAM_HPP_

#include <ak/data/Exceptions.hpp>
#include <ak/data/IBaseStream.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

namespace ak {
	namespace data {

		class OutputStream : public virtual IBaseStream {
			private:
				OutputStream(const OutputStream&) = delete;
				OutputStream& operator=(const OutputStream&) = delete;

			protected:
				OutputStream() = default;

				/**
				 * Used internally to perform writing operations
				 * @param src The array of data to write
				 * @param size The size of each element of data
				 * @param count The number of elements to write
				 * @param typeInfo The typeinfo of the object being written
				 * @return The number of complete elements written
				 * @remarks Stream should seek and truncate to the last complete element written (IE partial writes are discarded)
				 */
				virtual size_t write_internal(const void* src, size_t size, size_t count, std::type_index typeInfo) = 0;

			public:
				virtual ~OutputStream() = default;

				/**
				 * Attempts to write a single value to a stream
				 * @param src The value to write to the stream
				 * @return If the value was written to the stream
				 */
				template<typename type_t> bool write(const type_t& src) {
					static_assert(std::is_pod<type_t>::value, "The type being written must be either a POD or primitive type.");
					return write_internal(&src, sizeof(src), 1, typeid(src));
				}

				/**
				 * Attempts to write an array of values to a stream
				 * @param src The values to write to the stream
				 * @param count The number of values to write to the stream
				 * @return The number of values written to the stream
				 */
				template<typename type_t> size_t write(const type_t* src, size_t count) {
					static_assert(std::is_pod<type_t>::value, "The type being written must be either a POD or primitive type.");
					return write_internal(&src, sizeof(src), count, typeid(src));
				}

				/**
				 * Output stream operator wrapper around write(src)
				 * @param src The value to write
				 * @return This stream for chaining
				 * @throws InputStreamReadException Thrown when a write fails
				 */
				template<typename type_t> OutputStream& operator<<(const type_t& src) {
					if (!write(src)) throw OutputStreamWriteException("Failed to read from stream during WriteStream::operator<<");
					return *this;
				}

				/**
				 * Attempts to flush the stream, if it's buffered
				 * @return If the stream was flushed, or is unbuffered
				 */
				virtual bool flush() = 0;
		};

	}
}

#endif
