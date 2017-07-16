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

#ifndef AK_DATA_STREAM_HPP_
#define AK_DATA_STREAM_HPP_

#include <ak/data/InputStream.hpp>
#include <ak/data/OutputStream.hpp>

namespace ak {
	namespace data {

		class Stream : public virtual InputStream, public virtual OutputStream {
			private:
				Stream(const OutputStream&) = delete;
				Stream& operator=(const OutputStream&) = delete;

			protected:
				Stream() = default;

			public:
				virtual ~Stream() = default;

				/**
				 * Input stream operator wrapper around read(dest)
				 * @param dest The destination to store the read value into
				 * @return This object, for chaining
				 * @throws InputStreamReadException Thrown when a read fails
				 */
				template<typename type_t> Stream& operator>>(type_t& dest) {
					this->InputStream::operator >>(dest);
					return *this;
				}

				/**
				 * Output stream operator wrapper around write(src)
				 * @param src The value to write
				 * @return This stream for chaining
				 * @throws InputStreamReadException Thrown when a write fails
				 */
				template<typename type_t> Stream& operator<<(const type_t& src) {
					this->OutputStream::operator <<(src);
					return *this;
				}
		};

	}
}

#endif
