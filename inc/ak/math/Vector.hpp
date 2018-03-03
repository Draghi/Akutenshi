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

#include <ak/math/Types.hpp>
#include <ak/math/Scalar.hpp>
#include <glm/glm.hpp>
#include <glm/geometric.hpp>

namespace akm {
	template<glm::length_t l, typename func_t> Vec<l> forEachV(const Vec<l>& v0, const func_t& func) { Vec<l> result; for(akSSize i = 0; i < l; i++) result[i] = func(v0[i]); return result; }
	template<glm::length_t l, typename func_t> Vec<l> forEachV(const Vec<l>& v0, const Vec<l>& v1, const func_t& func) { Vec<l> result; for(akSSize i = 0; i < l; i++) result[i] = func(v0[i], v1[i]); return result; }

	template<glm::length_t l> Vec<l> abs(const Vec<l>& v) { Vec<l> result; for(akSSize i = 0; i < l; i++) result[i] = akm::abs(v[i]); return result; }

	template<glm::length_t l> Vec<l> ceil(const Vec<l>& v) { Vec<l> result; for(akSSize i = 0; i < l; i++) result[i] = akm::ceil(v[i]); return result; }
	template<glm::length_t l> Vec<l> floor(const Vec<l>& v) { Vec<l> result; for(akSSize i = 0; i < l; i++) result[i] = akm::floor(v[i]); return result; }
	template<glm::length_t l> Vec<l> round(const Vec<l>& v) { Vec<l> result; for(akSSize i = 0; i < l; i++) result[i] = akm::round(v[i]); return result; }
	template<glm::length_t l> Vec<l> trunc(const Vec<l>& v) { Vec<l> result; for(akSSize i = 0; i < l; i++) result[i] = akm::trunc(v[i]); return result; }

	template<glm::length_t l> scalar_t min(const Vec<l>& v) { scalar_t result = v[0]; for(akSSize i = 1; i < l; i++) result = akm::min(result, v[i]); return result; }
	template<glm::length_t l> scalar_t max(const Vec<l>& v) { scalar_t result = v[0]; for(akSSize i = 1; i < l; i++) result = akm::max(result, v[i]); return result; }

	template<glm::length_t l> scalar_t sum(const Vec<l>& v) { scalar_t result = v[0]; for(akSSize i = 1; i < l; i++) result += v[i]; return result; }

	template<glm::length_t l> Vec<l> min(const Vec<l>& x, const Vec<l>& y) { Vec<l> result; for(akSSize i = 0; i < l; i++) result[i] = akm::min(x[i], y[i]); return result; }
	template<glm::length_t l> Vec<l> max(const Vec<l>& x, const Vec<l>& y) { Vec<l> result; for(akSSize i = 0; i < l; i++) result[i] = akm::max(x[i], y[i]); return result; }
	template<glm::length_t l> Vec<l> clamp(const Vec<l>& v, const Vec<l>& minVal, const Vec<l>& maxVal) { Vec<l> result; for(akSSize i = 0; i < l; i++) result[i] = akm::clamp(v[i], minVal[i], maxVal[i]); return result; }

	template<glm::length_t l> Vec<l> mix(const Vec<l>& x, const Vec<l>& y, scalar_t a) { return glm::mix(x, y, a); }

	inline Vec2 perpendicular(const Vec2& vec) { return Vec2(-vec.y, vec.x); }
	inline Vec3 cross(const Vec3& x, const Vec3& y) { return glm::cross(x, y); }
	inline Vec4 cross(const Vec4& x, const Vec4& y) { return akm::Vec4(glm::cross(akm::Vec3(x), akm::Vec3(y)), 0); }

	template<glm::length_t l> Vec<l> normalize(const Vec<l>& x) { return glm::normalize<l, scalar_t, glm::highp>(x); }
	template<glm::length_t l> Vec<l> reflect(const Vec<l>& x, const Vec<l>& n) { return glm::reflect<l, scalar_t, glm::highp>(x, n); }
	template<glm::length_t l> Vec<l> refract(const Vec<l>& x, const Vec<l>& n, scalar_t eta) { return glm::refract<l, scalar_t, glm::highp>(x, n, eta); }

	template<glm::length_t l> scalar_t dot(const Vec<l>& x, const Vec<l>& y) { return glm::dot(x, y); }
	template<glm::length_t l> scalar_t distance(const Vec<l>& x, const Vec<l>& y) { return glm::distance(x, y); }
	template<glm::length_t l> scalar_t magnitude(const Vec<l>& x) { return glm::length<l, scalar_t, glm::highp>(x); }
	template<typename vec_t> typename vec_t::value_type sqrMagnitude(const vec_t& vec) { return glm::dot(vec, vec); }
}

#endif
