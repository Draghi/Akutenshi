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

#ifndef AK_DATA_IBASESTREAM_HPP_
#define AK_DATA_IBASESTREAM_HPP_

#include <ak/PrimitiveTypes.hpp>

namespace ak {
	namespace data {

		/**
		 * The anchoring point for seek operation
		 */
		enum class StreamAnchor {
			START, //!< START Anchor to the beginning of the stream
			END,   //!< END Anchor to the end of the stream
			CURRENT//!< CURRENT Anchor to the current position in the stream
		};

		class IBaseStream {
			private:
				IBaseStream(const IBaseStream&) = delete;
				IBaseStream& operator=(const IBaseStream&) = delete;

			protected:
				IBaseStream() = default;

			public:
				virtual ~IBaseStream() = default;

				virtual ssize_t seek(ssize_t offset) = 0;
				virtual ssize_t tell() = 0;

				virtual ssize_t size() = 0;
				virtual ssize_t remaining() = 0;
		};

	}
}

#endif
