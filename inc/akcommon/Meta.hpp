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

#ifndef AKCOMMON_META_HPP_
#define AKCOMMON_META_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <initializer_list>
#include <type_traits>
#include <tuple>
#include <utility>

namespace akc {
	template<size_t... Is, typename func_t> auto compileSwitch(size_t index, std::integer_sequence<size_t, Is...>, func_t func) {
		using return_type = typename std::common_type<decltype(func(std::integral_constant<size_t, Is>{}))...>::type;
		std::optional<return_type> ret;
		(void)std::initializer_list<int> ({(index == Is ? (ret = func(std::integral_constant<size_t, Is>{})), 0 : 0)...});
		return ret;
	}

	namespace internal {
		template<typename tuple_t, typename func_t, size_t... seq_v> auto vardicSwitch(size_t index, func_t func, tuple_t funcData, std::index_sequence<seq_v...> seq) {
			return compileSwitch(index, seq, [&](auto i){ return func(std::get<i>(std::move(funcData))); });
		}
	}

	template<typename func_t, typename... varg_t> auto vardicSwitch(size_t index, func_t func, varg_t&&... vargs) {
		  return internal::vardicSwitch(index, func, std::forward_as_tuple(std::forward<varg_t>(vargs)...), std::index_sequence_for<varg_t...>{});
	}
}

#endif /* AKCOMMON_META_HPP_ */
