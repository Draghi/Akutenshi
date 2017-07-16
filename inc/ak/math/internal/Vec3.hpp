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

#ifndef AK_MATH_VEC3_HPP_
#define AK_MATH_VEC3_HPP_

#include <cstddef>
#include <stdexcept>

#include "ak/String.hpp"
#include "ak/PrimitiveTypes.hpp"
#include "ak/math/Scalar.hpp"

namespace ak {
	namespace math {
		namespace internal {
			template<typename scalar> struct Vec3_t final {
				static constexpr size_t VECTOR_SIZE = 3;

				scalar x;
				scalar y;
				scalar z;

				// ///////////// //
				// // Methods // //
				// ///////////// //

				scalar* data() { return reinterpret_cast<scalar*>(this); }

				// /////////////////// //
				// // Const methods // //
				// /////////////////// //
				Vec3_t negate() const { return Vec3_t{-x, -y, -z}; }
				Vec3_t normalize() const { return Vec3_t{x, y, z} * (1/magnitude()); }
				Vec3_t normalize(scalar unitLength) const { return Vec3_t{x, y, z}.normalize() * unitLength; }

				scalar magnitude() const { return ak::math::sqrt(x*x + y*y + z*z); }
				size_t size() const { return VECTOR_SIZE; }

				const scalar* data() const { return reinterpret_cast<const scalar*>(this); }

				// /////////////// //
				// // Operators // //
				// /////////////// //

				Vec3_t& operator+=(const Vec3_t<scalar>& right) { x += right.x; y += right.y; z += right.z; return *this; }
				Vec3_t& operator-=(const Vec3_t<scalar>& right) { x -= right.x; y -= right.y; z -= right.z; return *this; }
				Vec3_t& operator*=(scalar right) { x *= right; y *= right; z *= right; return *this; }
				Vec3_t& operator/=(scalar right) { x /= right; y /= right; z /= right; return *this; }

				scalar& operator[](size_t index) {
					if (index >= size()) throw std::out_of_range(ak::buildString("Index(", index, ") out of range(", size(), ")"));
					return data()[index];
				}

				// ///////////////////// //
				// // Const Operators // //
				// ///////////////////// //

				Vec3_t operator+(const Vec3_t<scalar>& right) const { Vec3_t<scalar> result(*this); result += right; return result; }
				Vec3_t operator-(const Vec3_t<scalar>& right) const { Vec3_t<scalar> result(*this); result -= right; return result; }
				Vec3_t operator*(scalar right) const { Vec3_t<scalar> result(*this); result *= right; return result; }
				Vec3_t operator/(scalar right) const { Vec3_t<scalar> result(*this); result /= right; return result; }

				Vec3_t operator-() const { return negate(); }

				scalar operator[](size_t index) const {
					if (index >= size()) throw std::out_of_range(ak::buildString("Index(", index, ") out of range(", size(), ")"));
					return data()[index];
				}
			};

			template<typename scalar> Vec3_t<scalar> operator*(scalar left, const Vec3_t<scalar>& right) { return Vec3_t<scalar>(left*right.x, left*right.y, left*right.z); }
		}

		extern template struct internal::Vec3_t<fpSingle>;
		using Vec3 = internal::Vec3_t<fpSingle>;

		extern template struct internal::Vec3_t<fpDouble>;
		using Vec3d = internal::Vec3_t<fpDouble>;
	}
}

#endif
