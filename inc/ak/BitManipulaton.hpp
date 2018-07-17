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

#ifndef AK_BITMANIPULATON_HPP_
#define AK_BITMANIPULATON_HPP_

#include <climits>

#include <ak/PrimitiveTypes.hpp>
#include <ak/Traits.hpp>

namespace ak {
	template<typename type_t> inline constexpr type_t bitmask(akSize count) {
		return static_cast<type_t>(-(count != 0)) & (static_cast<type_t>(-1) >> ((sizeof(type_t) * CHAR_BIT) - count));
	}

	template<typename type_t> inline constexpr bool hasBitFlag(const type_t& val, const typename ak::traits::Identity<type_t>::type& flag) {
		return (val & flag) == flag;
	}
}

#endif
