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

#include <ak/math/internal/Vec2.hpp>
#include <ak/math/internal/Vec3.hpp>
#include <ak/math/internal/Vec4.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <type_traits>

// ////////// //
// // Vec2 // //
// ////////// //

template struct ak::math::internal::Vec2_t<fpSingle>;
static_assert(std::is_pod<ak::math::Vec2>(), "Vec2 is not POD");
static_assert(sizeof(ak::math::Vec2) == sizeof(fpSingle)*2, "Vec2 is an invalid size");

// /////////// //
// // Vec2d // //
// /////////// //

template struct ak::math::internal::Vec2_t<fpDouble>;
static_assert(std::is_pod<ak::math::Vec2d>(), "Vec2d is not POD");
static_assert(sizeof(ak::math::Vec2d) == sizeof(fpDouble)*2, "Vec2d is an invalid size");

// ////////// //
// // Vec3 // //
// ////////// //

template struct ak::math::internal::Vec3_t<fpSingle>;
static_assert(std::is_pod<ak::math::Vec3>(), "Vec3 is not POD");
static_assert(sizeof(ak::math::Vec3) == sizeof(fpSingle)*3, "Vec3 is an invalid size");

// /////////// //
// // Vec3d // //
// /////////// //

template struct ak::math::internal::Vec3_t<fpDouble>;
static_assert(std::is_pod<ak::math::Vec3d>(), "Vec3d is not POD");
static_assert(sizeof(ak::math::Vec3d) == sizeof(fpDouble)*3, "Vec3d is an invalid size");

// ////////// //
// // Vec4 // //
// ////////// //

template struct ak::math::internal::Vec4_t<fpSingle>;
static_assert(std::is_pod<ak::math::Vec4>(), "Vec4 is not POD");
static_assert(sizeof(ak::math::Vec4) == sizeof(fpSingle)*4, "Vec4 is an invalid size");

// /////////// //
// // Vec4d // //
// /////////// //

template struct ak::math::internal::Vec4_t<fpDouble>;
static_assert(std::is_pod<ak::math::Vec4d>(), "Vec4d is not POD");
static_assert(sizeof(ak::math::Vec4d) == sizeof(fpDouble)*4, "Vec4d is an invalid size");

