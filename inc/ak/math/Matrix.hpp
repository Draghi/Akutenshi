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

#ifndef AK_MATH_MATRIX_HPP_
#define AK_MATH_MATRIX_HPP_

#include <ak/math/Scalar.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <glm/detail/setup.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

namespace akm {
	// //////////////// //
	// // Projection // //
	// //////////////// //

	inline Mat4 frustum(scalar_t l, scalar_t r, scalar_t t, scalar_t b, scalar_t n, scalar_t f) { return glm::frustum<scalar_t>(l, r, t, b, n, f); }

	inline Mat4 orthographic(scalar_t width, scalar_t height, bool center) { scalar_t offX = center ? width/2 : 0, offY = center ? height/2 : 0; return glm::ortho<scalar_t>(-offX, -offY, width-offX, height-offY); }
	inline Mat4 orthographic(scalar_t left, scalar_t right, scalar_t top, scalar_t bottom) { return glm::ortho<scalar_t>(left, right, top, bottom); }
	inline Mat4 orthographic(scalar_t left, scalar_t right, scalar_t top, scalar_t bottom, scalar_t near, scalar_t far) { return glm::ortho<scalar_t>(left, right, top, bottom, near, far); }

	inline Mat4 perspectiveH(scalar_t fovX, scalar_t width, scalar_t height, scalar_t near, scalar_t far) { return glm::perspectiveFov(fovX, width, height, near, far); }
	inline Mat4 perspectiveV(scalar_t fovY, scalar_t width, scalar_t height, scalar_t near, scalar_t far) { return glm::perspective(fovY, width/height, near, far); }

	inline Mat4 perspectiveInfH(scalar_t fovX, scalar_t width, scalar_t height) { return glm::infinitePerspective(2.0f * akm::atan(akm::tan(fovX * .5f)/(width/height)), width, height); }
	inline Mat4 perspectiveInfV(scalar_t fovY, scalar_t width, scalar_t height) { return glm::infinitePerspective(fovY, width, height); }

	inline Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) { return glm::lookAt(eye, center, up); }

	// //////////// //
	// // Affine // //
	// //////////// //

	inline Vec4 row(const akm::Mat4& mat, akSize row) { return glm::row(mat, row); }
	inline Vec4 column(const akm::Mat4& mat, akSize col) { return glm::column(mat, col); }

	inline Mat4 scale(const akm::Vec3& scale) { return glm::scale(scale); }
	inline Mat4 translate(const akm::Vec3& offset) { return glm::translate(offset); }
	inline Mat4 rotate(scalar_t angle, const akm::Vec3& axis) { return glm::rotate(angle, axis); }

	template<glm::length_t l> scalar_t determinant(const Mat<l>& mat) { return glm::determinant(mat); }
	template<glm::length_t l> Mat<l> inverse(const Mat<l>& mat) { return glm::inverse(mat); }
	template<glm::length_t l> Mat<l> transpose(const Mat<l>& mat) { return glm::transpose(mat); }

	inline Mat3 eulerOrient3(const Vec3& eulerAngles) { return glm::orientate3(eulerAngles); }
	inline Mat4 eulerOrient4(const Vec3& eulerAngles) { return glm::orientate4(eulerAngles); }
}

#endif
