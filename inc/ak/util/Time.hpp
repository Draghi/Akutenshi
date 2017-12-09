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

#ifndef AK_TIME_TIME_HPP_
#define AK_TIME_TIME_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ctime>

namespace ak {
	namespace util {
		struct Timestamp {
			std::tm ctime;
			uint32 milliseconds;
		};

		/**
		 * Returns the current UTC timestamp
		 * @return The current UTC timestamp
		 */
		Timestamp utcTimestamp();

		/**
		 * Returns the current local timestamp
		 * @return The current local timestamp
		 */
		Timestamp localTimestamp();
	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_UTIL_ALIAS_DISABLE))
namespace aku = ak::util;
#endif

#endif
