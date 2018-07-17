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

#ifndef AK_MATH_TYPES_HPP_
#define AK_MATH_TYPES_HPP_

#include <glm/detail/qualifier.hpp>
#include <glm/detail/setup.hpp>
#include <glm/gtc/quaternion.hpp>

#include <ak/PrimitiveTypes.hpp>

namespace akm {

	using scalar_t = fpSingle;

	template<glm::length_t l> using Vec = glm::vec<l, scalar_t, glm::highp>;
	using Vec2 = Vec<2>;
	using Vec3 = Vec<3>;
	using Vec4 = Vec<4>;

	template<glm::length_t l> using Mat = glm::mat<l, l, scalar_t, glm::highp>;
	using Mat2 = Mat<2>;
	using Mat3 = Mat<3>;
	using Mat4 = Mat<4>;

	using Quat = glm::tquat<fpSingle, glm::highp>;
}

#endif
