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

#ifndef INC_AK_MATH_VEC2_HPP_
#define INC_AK_MATH_VEC2_HPP_

#include <cstddef>
#include <stdexcept>

#include "ak/String.hpp"
#include "ak/PrimitiveTypes.hpp"
#include "ak/math/Scalar.hpp"

namespace ak {
	namespace math {

		namespace internal {

			template<typename scalar> struct Vec2_t final {
				static constexpr size_t VECTOR_SIZE = 2;

				scalar x;
				scalar y;

				// ///////////// //
				// // Methods // //
				// ///////////// //

				scalar* data() { return reinterpret_cast<scalar*>(this); }

				// /////////////////// //
				// // Const methods // //
				// /////////////////// //

				Vec2_t negate() const { return Vec2_t{-x, -y}; }
				Vec2_t normalize() const { return Vec2_t{x, y} * (1/magnitude()); }
				Vec2_t normalize(scalar unitLength) const { return Vec2_t{x, y}.normalize() * unitLength; }

				Vec2_t perpendicular() const { return Vec2_t{-y, x}; }
				scalar magnitude() const { return ak::math::sqrt(x*x + y*y); }
				size_t size() const { return VECTOR_SIZE; }

				const scalar* data() const { return reinterpret_cast<const scalar*>(this); }

				// /////////////// //
				// // Operators // //
				// /////////////// //

				Vec2_t& operator+=(const Vec2_t<scalar>& right) { x += right.x; y += right.y; return *this; }
				Vec2_t& operator-=(const Vec2_t<scalar>& right) { x -= right.x; y -= right.y; return *this; }
				Vec2_t& operator*=(scalar right) { x *= right; y *= right; return *this; }
				Vec2_t& operator/=(scalar right) { x /= right; y /= right; return *this; }

				scalar& operator[](size_t index) {
					if (index >= size()) throw std::out_of_range(ak::buildString("Index(", index, ") out of range(", size(), ")"));
					return data()[index];
				}

				// ///////////////////// //
				// // Const Operators // //
				// ///////////////////// //

				Vec2_t operator+(const Vec2_t<scalar>& right) const { Vec2_t<scalar> result(*this); result += right; return result; }
				Vec2_t operator-(const Vec2_t<scalar>& right) const { Vec2_t<scalar> result(*this); result -= right; return result; }
				Vec2_t operator*(scalar right) const { Vec2_t<scalar> result(*this); result *= right; return result; }
				Vec2_t operator/(scalar right) const { Vec2_t<scalar> result(*this); result /= right; return result; }

				Vec2_t operator-() const { return negate(); }

				scalar operator[](size_t index) const {
					if (index >= size()) throw std::out_of_range(ak::buildString("Index(", index, ") out of range(", size(), ")"));
					return data()[index];
				}
			};

			template<typename scalar> Vec2_t<scalar> operator*(scalar left, const Vec2_t<scalar>& right) { return Vec2_t<scalar>(left*right.x, left*right.y); }
		}

		extern template struct internal::Vec2_t<fpSingle>;
		using Vec2 = internal::Vec2_t<fpSingle>;

		extern template struct internal::Vec2_t<fpDouble>;
		using Vec2d = internal::Vec2_t<fpDouble>;
	}
}

#endif
