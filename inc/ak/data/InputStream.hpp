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

#ifndef AK_DATA_INPUTSTREAM_HPP_
#define AK_DATA_INPUTSTREAM_HPP_

#include <ak/data/Exceptions.hpp>
#include <ak/data/IBaseStream.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <Optional>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

namespace ak {
	namespace data {

		class InputStream : public virtual IBaseStream {
			private:
				InputStream(const InputStream&) = delete;
				InputStream& operator=(const InputStream&) = delete;

			protected:
				InputStream() = default;

				/**
				 * Used internally to perform reading actions
				 * @param dest The destination array
				 * @param size The size of each element to read
				 * @param count The number of each element
				 * @param typeInfo The type information of the type being read
				 * @return The number of complete elements read
				 * @remarks Stream should seek to the last complete element read (IE partial reads are discarded)
				 */
				virtual size_t read_internal(void* dest, size_t size, size_t count, std::type_index typeInfo) = 0;

			public:
				virtual ~InputStream() = default;

				/**
				 * Attempts to read a single value from a binary stream
				 * @tparam type_t The type of value to read (Must be either a POD or primitive type)
				 * @return A single value of the given type, or nothing on a failed read
				 */
				template<typename type_t> std::optional<type_t> read() {
					static_assert(std::is_pod<type_t>::value, "The type being read must be either a POD or primitive type.");
					type_t dest;
					return read_internal(&dest, sizeof(type_t), 1, typeid(type_t)) ? std::optional<type_t>(std::move(dest)) : stx::optional<type_t>();
				}

				/**
				 * Attempts to read a single value from a binary stream, into the given reference
				 * @tparam type_t The type of value to read (Must be either a POD or primitive type)
				 * @param dest The destination to store the read value into
				 * @return If the value was read correctly
				 * @post Using dest after a failed read is undefined behavior
				 */
				template<typename type_t> bool read(type_t& dest) {
					static_assert(std::is_pod<type_t>::value, "The type being read must be either a POD or primitive type.");
					return read_internal(&dest, sizeof(type_t), 1, typeid(type_t)) > 0;
				}

				/**
				 * Attempts to read an array of values from a binary stream
				 * @tparam type_t The type of value to read (Must be either a POD or primitive type)
				 * @param dest The destination to store the read values into
				 * @param count The number of elements to read
				 * @return If the value was read correctly
				 * @pre Dest contains a preallocated array that can store at least <count> instances of type_t
				 * @post Using dest after a failed read is undefined behavior
				 */
				template<typename type_t> size_t read(type_t* dest, size_t count) {
					static_assert(std::is_pod<type_t>::value, "The type being read must be either a POD or primitive type.");
					return ((count == 0) ||(dest == nullptr)) ? 0 : read_internal(dest, sizeof(type_t), count, typeid(type_t));
				}

				/**
				 * Input stream operator wrapper around read(dest)
				 * @param dest The destination to store the read value into
				 * @return This object, for chaining
				 * @throws InputStreamReadException Thrown when a read fails
				 */
				template<typename type_t> InputStream& operator>>(type_t& dest) {
					if (!read(dest)) throw InputStreamReadException("Failed to read from stream during InputStream::operator>>");
					return *this;
				}
		};

	}
}

#endif
