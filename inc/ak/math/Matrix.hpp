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


#include <ak/PrimitiveTypes.hpp>
#include <ak/math/Vector.hpp>

#include <glm/glm.hpp>

#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/matrix_major_storage.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

namespace akm {

	template<int size, typename scalar_t> using Mat = glm::mat<size, size, scalar_t, glm::highp>;

	using Mat2 = Mat<2, fpSingle>;
	using Mat3 = Mat<3, fpSingle>;
	using Mat4 = Mat<4, fpSingle>;

	using glm::colMajor2;
	using glm::colMajor3;
	using glm::colMajor4;

	using glm::rowMajor2;
	using glm::rowMajor3;
	using glm::rowMajor4;

	using glm::frustum;
	using glm::ortho;
	using glm::perspective;
	using glm::perspectiveFov;
	using glm::infinitePerspective;

	using glm::lookAt;
	using glm::rotate;
	using glm::scale;
	using glm::translate;

	using glm::column;
	using glm::row;

	using glm::determinant;
	using glm::inverse;
	using glm::transpose;

	//template<typename scalar_t> Mat<2, scalar_t> translate(const Vec<2, scalar_t>& offset) { return translate(Mat<2, scalar_t>(static_cast<scalar_t>(1)), offset); }
	template<typename scalar_t> Mat<2, scalar_t> rotate(scalar_t angle) { return rotate(Mat<2, scalar_t>(static_cast<scalar_t>(1)), angle); }
	template<typename scalar_t> Mat<2, scalar_t> scale(const Vec<2, scalar_t>& factor) { return scale(Mat<2, scalar_t>(static_cast<scalar_t>(1)), factor); }

	template<typename scalar_t> Mat<3, scalar_t> eulerOrient3(const Vec<3, scalar_t>& eulerAngles) { return glm::orientate3(eulerAngles); }
	template<typename scalar_t> Mat<3, scalar_t> eulerOrient3(scalar_t x, scalar_t y, scalar_t z) { return eulerOrient3(Vec3(x, y, z)); }

	template<typename scalar_t> Mat<4, scalar_t> eulerOrient4(const Vec<3, scalar_t>& eulerAngles) { return glm::orientate4(eulerAngles); }
	template<typename scalar_t> Mat<4, scalar_t> eulerOrient4(scalar_t x, scalar_t y, scalar_t z) { return eulerOrient3(Vec3(x, y, z)); }

}

#endif
