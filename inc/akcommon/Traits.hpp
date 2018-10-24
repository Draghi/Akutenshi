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

namespace aku {
	namespace traits {

		/**
		 * Aliases the typename type_t as Identity<type_t>::type
		 * Useful for preventing deduction
		 */
		template<typename type_t> struct Identity {
			using type = type_t;
		};

		template<uint64 max_val> struct RequiredBits {
			static constexpr uint8 value =
				max_val < 0xffull       ?  8 :
				max_val < 0xffffull     ? 16 :
				max_val < 0xffffffffull ? 32 : 64;

		};

		namespace internal {
			template<uint64 max_val> struct SmallestIntegerFor;
			template<> struct SmallestIntegerFor< 8> { using type = uint8; };
			template<> struct SmallestIntegerFor<16> { using type = uint16; };
			template<> struct SmallestIntegerFor<32> { using type = uint32; };
			template<> struct SmallestIntegerFor<64> { using type = uint64; };
		}
		template<uint64 max_val> struct SmallestIntergerFor : internal::SmallestIntegerFor<RequiredBits<max_val>::value> {};

		namespace internal {
			template<uint64 bit_val> struct SizedInteger;
			template<> struct SizedInteger<1> { using type = uint8;  };

			template<> struct SizedInteger<2> { using type = uint16; };

			template<> struct SizedInteger<3> { using type = uint32; };
			template<> struct SizedInteger<4> { using type = uint32; };

			template<> struct SizedInteger<5> { using type = uint64; };
			template<> struct SizedInteger<6> { using type = uint64; };
			template<> struct SizedInteger<7> { using type = uint64; };
			template<> struct SizedInteger<8> { using type = uint64; };
		}
		template<uint64 bit_val> struct IntegerMinBytes : internal::SizedInteger<bit_val> {};

	}
}

#endif
