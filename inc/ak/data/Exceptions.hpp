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

#ifndef AK_DATA_EXCEPTIONS_HPP_
#define AK_DATA_EXCEPTIONS_HPP_

#include <stdexcept>

namespace ak {
	namespace data {

		/**
		 * Base exception for all stream related errors
		 */
		class StreamException : public std::runtime_error {
			public:
				using std::runtime_error::runtime_error;
		};

		/**
		 * Base exception for all input stream related errors
		 */
		class InputStreamException : public StreamException {
			public:
				using StreamException::StreamException;
		};

		/**
		 * Exception thrown when a read error occurs on an input stream
		 */
		class InputStreamReadException : public InputStreamException {
			public:
				using InputStreamException::InputStreamException;
		};

		/**
		 * Base exception for all output stream related errors
		 */
		class OutputStreamException : public StreamException {
			public:
				using StreamException::StreamException;
		};

		/**
		 * Exception thrown when a read error occurs on an output stream
		 */
		class OutputStreamWriteException : public OutputStreamException {
			public:
				using OutputStreamException::OutputStreamException;
		};
	}
}



#endif
