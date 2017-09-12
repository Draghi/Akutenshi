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

#ifndef AK_TRAITS_HPP_
#define AK_TRAITS_HPP_

namespace ak {
	namespace traits {

		/**
		 * Aliases the typename type_t as Identity<type_t>::type
		 * Useful for preventing deduction
		 */
		template<typename type_t> struct Identity {
			using type = type_t;
		};

	}
}

#endif
