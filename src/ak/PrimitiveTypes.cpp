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

#include <ak/PrimitiveTypes.hpp>
#include <type_traits>

// //////////////////////////////////// //
// // Signed Integer Primitive Types // //
// //////////////////////////////////// //

static_assert(sizeof(int8)  == 1, "Size of int8 is not 1");
static_assert(sizeof(int16) == 2, "Size of int16 is not 2");
static_assert(sizeof(int32) == 4, "Size of int32 is not 4");
static_assert(sizeof(int64) == 8, "Size of int64 is not 8");

static_assert(std::is_signed<int>::value, "int isn't signed!"); // One would hope it is...

static_assert(std::is_signed<int8>::value,  "int8 isn't signed!");
static_assert(std::is_signed<int16>::value, "int16 isn't signed!");
static_assert(std::is_signed<int32>::value, "int32 isn't signed!");
static_assert(std::is_signed<int64>::value, "int64 isn't signed!");

static_assert(std::is_integral<int8>::value,  "int8 isn't integral!");
static_assert(std::is_integral<int16>::value, "int16 isn't integral!");
static_assert(std::is_integral<int32>::value, "int32 isn't integral!");
static_assert(std::is_integral<int64>::value, "int64 isn't integral!");

// ////////////////////////////////////// //
// // Unsigned Integer Primitive Types // //
// ////////////////////////////////////// //

static_assert(sizeof(uint8)  == 1, "Size of uint8 is not 1");
static_assert(sizeof(uint16) == 2, "Size of uint16 is not 2");
static_assert(sizeof(uint32) == 4, "Size of uint32 is not 4");
static_assert(sizeof(uint64) == 8, "Size of uint64 is not 8");

static_assert(std::is_unsigned<uint>::value, "uint isn't unsigned!");

static_assert(std::is_unsigned<uint8>::value,  "uint8 isn't unsigned!");
static_assert(std::is_unsigned<uint16>::value, "uint16 isn't unsigned!");
static_assert(std::is_unsigned<uint32>::value, "uint32 isn't unsigned!");
static_assert(std::is_unsigned<uint64>::value, "uint64 isn't unsigned!");

static_assert(std::is_integral<uint8>::value,  "uint8 isn't integral!");
static_assert(std::is_integral<uint16>::value, "uint16 isn't integral!");
static_assert(std::is_integral<uint32>::value, "uint32 isn't integral!");
static_assert(std::is_integral<uint64>::value, "uint64 isn't integral!");

// //////////////////////////////////// //
// // Floating Point Primitive Types // //
// //////////////////////////////////// //

static_assert(sizeof(fpSingle) == 4, "fpSingle isn't exactly 32-bits large!");
static_assert(sizeof(fpDouble) == 8, "fpDouble isn't exactly 64-bits large!");

static_assert(std::is_floating_point<fpSingle>::value, "fpSingle isn't a floating point!");
static_assert(std::is_floating_point<fpDouble>::value, "fpDouble isn't a floating point!");

static_assert(std::numeric_limits<fpSingle>::is_iec559, "fpSingle isn't a iec559 floating point!");
static_assert(std::numeric_limits<fpDouble>::is_iec559, "fpDouble isn't a iec559 floating point!");
