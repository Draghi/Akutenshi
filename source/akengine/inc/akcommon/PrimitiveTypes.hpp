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

#ifndef AK_COMMON_PRIMITIVETYPES_HPP_
#define AK_COMMON_PRIMITIVETYPES_HPP_

#include <string>

/**
 * This header provides sized primitive data-types such as:
 *     Signed   integers: 8-bit, 16-bit, 32-bit and 64-bit
 *     Unsigned integers: 8-bit, 16-bit, 32-bit and 64-bit
 *     Floating   points: 32-bit and 64-bit
 *
 *	Additionally some helper/shortcut types are defined:
 *	   uint - Unsigned integer
 *	   fpNumber - redefinable floating point type
 *
 *	These types are backed by static assertions in a translation unit,
 *	To get max/min values use std::numeric_limits<type>
 */

// //////////////////////////////////// //
// // Signed Integer Primitive Types // //
// //////////////////////////////////// //

// using int = int; // No-duh

using int8  = char;      /// Signed  8-bit integer
using int16 = short;     /// Signed 16-bit integer
using int32 = int;       /// Signed 32-bit integer
using int64 = long long; /// Signed 64-bit integer

// ////////////////////////////////////// //
// // Unsigned Integer Primitive Types // //
// ////////////////////////////////////// //

using uint = unsigned; /// Unsigned integer

using uint8  = unsigned char;      /// Unsigned  8-bit integer
using uint16 = unsigned short;     /// Unsigned 16-bit integer
using uint32 = unsigned int;       /// Unsigned 32-bit integer
using uint64 = unsigned long long; /// Unsigned 64-bit integer

// //////////////////////////////////// //
// // Floating Point Primitive Types // //
// //////////////////////////////////// //

using fpSingle = float;  /// 32-bit floating point number
using fpDouble = double; /// 64-bit floating point number

// ///////////////// //
// // Other Types // //
// ///////////////// //

using akSize  = uint32;
using akSSize =  int32;

//
// Util
//

namespace ak {
	template<typename type_t> std::string getPrimitiveName();

	template<> inline std::string getPrimitiveName<uint8>() { return "uint8"; }
	template<> inline std::string getPrimitiveName<uint16>() { return "uint16"; }
	template<> inline std::string getPrimitiveName<uint32>() { return "uint32"; }
	template<> inline std::string getPrimitiveName<uint64>() { return "uint64"; }

	template<> inline std::string getPrimitiveName<int8>() { return "int8"; }
	template<> inline std::string getPrimitiveName<int16>() { return "int16"; }
	template<> inline std::string getPrimitiveName<int32>() { return "int32"; }
	template<> inline std::string getPrimitiveName<int64>() { return "int64"; }

	template<> inline std::string getPrimitiveName<fpSingle>() { return "fpSingle"; }
	template<> inline std::string getPrimitiveName<fpDouble>() { return "fpDouble"; }
}

#if defined(_MSC_VER)
#include <type_traits>
using ssize_t = typename std::make_signed<size_t>::type;
#endif

#endif
