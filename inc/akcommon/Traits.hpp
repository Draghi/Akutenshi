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

#ifndef AK_COMMON_TRAITS_HPP_
#define AK_COMMON_TRAITS_HPP_

#include <akcommon/PrimitiveTypes.hpp>

namespace akc {
	namespace traits {

		/**
		 * Aliases the typename type_t as Identity<type_t>::type
		 * Useful for preventing deduction
		 */
		template<typename type_t> struct Identity {
			using type = type_t;
		};

		/**
		 * Finds the index in a parameter pack of the first type-match.
		 * @note find_t The type to find \
		 *      vargs_t The types to index
		 */
		template<typename...> struct VargIndexer;
		template<typename find_t, typename... vargs_t> struct VargIndexer<find_t, find_t, vargs_t...> : std::integral_constant<akSize, 0> {};
		template<typename find_t, typename mismatch_t, typename... vargs_t> struct VargIndexer<find_t, mismatch_t, vargs_t...> : std::integral_constant<akSize, 1 + VargIndexer<find_t, vargs_t...>::value> {};

		/**
		 * Calculates the number of bytes, in an unsigned type, required to store a value.
		 * @note max_val The value to fit (max 2^64-1)
		 */
		template<uint64 max_val> struct RequiredBytes {
			static constexpr uint8 value =
				max_val < 0xffull             ?  1 :
				max_val < 0xffffull           ?  2 :
				max_val < 0xffffffull         ?  3 :
				max_val < 0xffffffffull       ?  4 :
				max_val < 0xffffffffffull     ?  5 :
				max_val < 0xffffffffffffull   ?  6 :
				max_val < 0xffffffffffffffull ?  7 : 8;
		};

		/**
		 * Returns the closest unsigned integer type that can hold the maximium number the bytes can represent.
		 * @note byte_n The number of bytes of number data
		 */
		namespace internal {
			template<uint64 byte_n> struct SmallestIntegerForBytes;
			template<> struct SmallestIntegerForBytes<1> { using type = uint8; };
			template<> struct SmallestIntegerForBytes<2> { using type = uint16; };
			template<> struct SmallestIntegerForBytes<3> { using type = uint32; };
			template<> struct SmallestIntegerForBytes<4> { using type = uint32; };
			template<> struct SmallestIntegerForBytes<5> { using type = uint64; };
			template<> struct SmallestIntegerForBytes<6> { using type = uint64; };
			template<> struct SmallestIntegerForBytes<7> { using type = uint64; };
			template<> struct SmallestIntegerForBytes<8> { using type = uint64; };
		}
		template<uint64 byte_n> struct IntegerForBytes : internal::SmallestIntegerForBytes<RequiredBytes<byte_n>::value> {};

		template<typename type_t, type_t...> struct IsUniqueValue;
		template<typename type_t, type_t v1, type_t v2> struct IsUniqueValue<type_t, v1, v2> : std::integral_constant<bool, v1 != v2> {};
		template<typename type_t, type_t v1, type_t v2, type_t... vargs_v> struct IsUniqueValue<type_t, v1, v2, vargs_v...> : std::integral_constant<bool, IsUniqueValue<type_t, v1, v2>::value && IsUniqueValue<type_t, v2, vargs_v...>::value> {};

		template<typename type_t, type_t...> struct IsUniqueList;
		template<typename type_t, type_t v1, type_t v2> struct IsUniqueList<type_t, v1, v2> : IsUniqueValue<type_t, v1, v2> {};
		template<typename type_t, type_t v1, type_t v2, type_t... vargs_v> struct IsUniqueList<type_t, v1, v2, vargs_v...> : std::integral_constant<bool, IsUniqueValue<type_t, v1, v2, vargs_v...>::value && IsUniqueList<type_t, v2, vargs_v...>::value> {};

	}
}

#endif
