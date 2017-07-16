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

#ifndef AK_MATH_INTERNAL_VECTORCONVERSIONS_HPP_
#define AK_MATH_INTERNAL_VECTORCONVERSIONS_HPP_

#include "ak/math/internal/Vec2.hpp"
#include "ak/math/internal/Vec3.hpp"
#include "ak/math/internal/Vec4.hpp"

namespace ak {
	namespace math {

		// //////////// //
		// // Single // //
		// //////////// //

		template<typename scalar> Vec2 toVec2(scalar x, scalar y) {
			return Vec2{static_cast<fpSingle>(x), static_cast<fpSingle>(y)};
		}

		template<typename scalar> Vec3 toVec3(scalar x, scalar y, scalar z) {
			return Vec3{static_cast<fpSingle>(x), static_cast<fpSingle>(y), static_cast<fpSingle>(z)};
		}

		template<typename scalar> Vec4 toVec4(scalar x, scalar y, scalar z, scalar w) {
			return Vec4{static_cast<fpSingle>(x), static_cast<fpSingle>(y), static_cast<fpSingle>(z), static_cast<fpSingle>(w)};
		}

		template<typename scalar> Vec2 toVec2() { return toVec2(0,0); }
		template<typename scalar> Vec3 toVec3() { return toVec3(0, 0, 0); }
		template<typename scalar> Vec4 toVec4() { return toVec4(0, 0, 0, 0); }

		template<typename scalar> Vec2 toVec2(internal::Vec2_t<scalar> vec) { return toVec2(vec.x, vec.y); }
		template<typename scalar> Vec3 toVec3(internal::Vec2_t<scalar> vec, scalar z) { return toVec3(vec.x, vec.y, z); }
		template<typename scalar> Vec4 toVec4(internal::Vec2_t<scalar> vec, scalar z, scalar w) { return toVec4(vec.x, vec.y, z, w); }

		template<typename scalar> Vec2 toVec2(internal::Vec3_t<scalar> vec) { return toVec2(vec.x, vec.y); }
		template<typename scalar> Vec3 toVec3(internal::Vec3_t<scalar> vec) { return toVec3(vec.x, vec.y, vec.z); }
		template<typename scalar> Vec4 toVec4(internal::Vec3_t<scalar> vec, scalar w) { return toVec4(vec.x, vec.y, vec.z, w); }

		template<typename scalar> Vec2 toVec2(internal::Vec4_t<scalar> vec) { return toVec2(vec.x, vec.y); }
		template<typename scalar> Vec3 toVec3(internal::Vec4_t<scalar> vec) { return toVec3(vec.x, vec.y, vec.z); }
		template<typename scalar> Vec4 toVec4(internal::Vec4_t<scalar> vec) { return toVec4(vec.x, vec.y, vec.z, vec.w); }

		// //////////// //
		// // Double // //
		// //////////// //

		template<typename scalar> Vec2d toVec2d(scalar x, scalar y) {
			return Vec2d{static_cast<fpDouble>(x), static_cast<fpDouble>(y)};
		}

		template<typename scalar> Vec3d toVec3d(scalar x, scalar y, scalar z) {
			return Vec3d{static_cast<fpDouble>(x), static_cast<fpDouble>(y), static_cast<fpDouble>(z)};
		}

		template<typename scalar> Vec4d toVec4d(scalar x, scalar y, scalar z, scalar w) {
			return Vec4d{static_cast<fpDouble>(x), static_cast<fpDouble>(y), static_cast<fpDouble>(z), static_cast<fpDouble>(w)};
		}

		template<typename scalar> Vec2d toVec2d() { return toVec2d(0, 0); }
		template<typename scalar> Vec3d toVec3d() { return toVec3d(0, 0, 0); }
		template<typename scalar> Vec4d toVec4d() { return toVec4d(0, 0, 0, 0); }

		template<typename scalar> Vec2d toVec2d(internal::Vec2_t<scalar> vec) { return toVec2d(vec.x, vec.y); }
		template<typename scalar> Vec3d toVec3d(internal::Vec2_t<scalar> vec, scalar z) { return toVec3d(vec.x, vec.y, z); }
		template<typename scalar> Vec4d toVec4d(internal::Vec2_t<scalar> vec, scalar z, scalar w) { return toVec4d(vec.x, vec.y, z, w); }

		template<typename scalar> Vec2d toVec2d(internal::Vec3_t<scalar> vec) { return toVec2d(vec.x, vec.y); }
		template<typename scalar> Vec3d toVec3d(internal::Vec3_t<scalar> vec) { return toVec3d(vec.x, vec.y, vec.z); }
		template<typename scalar> Vec4d toVec4d(internal::Vec3_t<scalar> vec, scalar w) { return toVec4d(vec.x, vec.y, vec.z, w); }

		template<typename scalar> Vec2d toVec2d(internal::Vec4_t<scalar> vec) { return toVec2d(vec.x, vec.y); }
		template<typename scalar> Vec3d toVec3d(internal::Vec4_t<scalar> vec) { return toVec3d(vec.x, vec.y, vec.z); }
		template<typename scalar> Vec4d toVec4d(internal::Vec4_t<scalar> vec) { return toVec4d(vec.x, vec.y, vec.z, vec.w); }
	}
}

#endif
