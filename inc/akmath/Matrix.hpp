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

#ifndef AK_MATH_MATRIX_HPP_
#define AK_MATH_MATRIX_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/data/PValue.hpp>
#include <akengine/data/SmartClass.hpp>
#include <akmath/Scalar.hpp>
#include <akmath/Vector.hpp>
#include <glm/common.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/vec4.hpp>

namespace akm {
	template<glm::length_t l, glm::length_t m> using Mat = glm::mat<l, m, scalar_t, glm::highp>;
	using Mat2 = Mat<2, 2>;
	using Mat3 = Mat<3, 3>;
	using Mat4 = Mat<4, 4>;
	using Mat4x3 = Mat<4, 3>;

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

	inline Vec3 row(const akm::Mat3& mat, akSize row) { return glm::row(mat, row); }
	inline Vec4 row(const akm::Mat4& mat, akSize row) { return glm::row(mat, row); }
	inline Vec4 row(const akm::Mat4x3& mat, akSize row) { return glm::row(mat, row); }

	inline Vec3 column(const akm::Mat3& mat, akSize col) { return glm::column(mat, col); }
	inline Vec4 column(const akm::Mat4& mat, akSize col) { return glm::column(mat, col); }
	inline Vec3 column(const akm::Mat4x3& mat, akSize col) { return glm::column(mat, col); }

	inline akm::Mat4 toMat4(akm::Mat4x3 mat) {
		return akm::Mat4{
			akm::Vec4(akm::column(mat, 0), 0),
			akm::Vec4(akm::column(mat, 1), 0),
			akm::Vec4(akm::column(mat, 2), 0),
			akm::Vec4(akm::column(mat, 3), 1)
		};
	}

	inline Mat4 translate(const akm::Vec3& offset) { return glm::translate(offset); }
	inline Mat4 scale (const akm::Vec3& scale) { return glm::scale(scale); }
	inline Mat3 scale3(const akm::Vec3& scale)  { return Mat3(akm::scale(scale)); }
	inline Mat4 rotate (scalar_t angle, const akm::Vec3& axis) { return glm::rotate(angle, axis); }
	inline Mat3 rotate3(scalar_t angle, const akm::Vec3& axis) { return Mat3(akm::rotate(angle, axis)); }

	template<glm::length_t l, glm::length_t m> scalar_t determinant(const Mat<l, m>& mat) { return glm::determinant(mat); }
	template<glm::length_t l, glm::length_t m> Mat<l,m>     inverse(const Mat<l, m>& mat) { return glm::inverse(mat);     }

	// Assumes that the 4x3 matrix provided is *actually* a 4x4 matrix with a last row of (0,0,0,1)
	template<> inline Mat<4,3> inverse<4, 3>(const Mat<4, 3>& mat) {
		return glm::inverse(toMat4(mat));
	}
	template<glm::length_t l, glm::length_t m> Mat<l,m>   transpose(const Mat<l, m>& mat) { return glm::transpose(mat);   }

	inline Mat3 eulerOrient3(const Vec3& eulerAngles) { return glm::orientate3(eulerAngles); }
	inline Mat4 eulerOrient4(const Vec3& eulerAngles) { return glm::orientate4(eulerAngles); }
}

AK_SMART_CLASS_ARRAY(akm::Mat2,   akm::Mat2::length());
AK_SMART_CLASS_ARRAY(akm::Mat3,   akm::Mat3::length());
AK_SMART_CLASS_ARRAY(akm::Mat4,   akm::Mat4::length());
AK_SMART_CLASS_ARRAY(akm::Mat4x3, akm::Mat4x3::length());

#endif
