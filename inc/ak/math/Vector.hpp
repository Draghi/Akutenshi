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

#ifndef AK_MATH_VECTOR_HPP_
#define AK_MATH_VECTOR_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <glm/detail/func_common.hpp>
#include <glm/detail/func_geometric.hpp>
#include <glm/detail/precision.hpp>
#include <glm/detail/type_vec2.hpp>
#include <glm/gtc/quaternion.hpp>

namespace akm {

	template<int size, typename scalar_t> using Vec = glm::vec<size, scalar_t, glm::highp>;

	using Vec2 = Vec<2, fpSingle>;
	using Vec3 = Vec<3, fpSingle>;
	using Vec4 = Vec<4, fpSingle>;

	using glm::mix;
	using glm::cross;
	using glm::dot;
	using glm::distance;
	using glm::length;
	using glm::normalize;
	using glm::reflect;
	using glm::refract;

	inline Vec2 perpendicular(const Vec2& vec) { return Vec2(-vec.y, vec.x); }

	template<typename vec_t> typename vec_t::value_type sqrLength(const vec_t& vec) { return dot(vec, vec); }

}

#endif
