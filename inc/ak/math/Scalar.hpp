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

#ifndef AK_MATH_SCALAR_HPP_
#define AK_MATH_SCALAR_HPP_

#include <ak/PrimitiveTypes.hpp>

#include <cmath>
#include <glm/glm.hpp>

namespace akm {
	constexpr fpDouble PI_d = 3.141592653589793238462643;
	template<typename scalar_t> constexpr scalar_t PI = static_cast<scalar_t>(PI_d);

	template<typename scalar_t> scalar_t degToRad(scalar_t deg) {
		constexpr scalar_t conversionFactor = 180/PI<scalar_t>;
		return deg * conversionFactor;
	}

	template<typename scalar_t> scalar_t radToDeg(scalar_t rad) {
		constexpr scalar_t conversionFactor = PI<scalar_t>/180;
		return rad * conversionFactor;
	}

	using glm::abs;

	using glm::ceil;
	using glm::floor;
	using glm::round;
	using glm::trunc;

	using glm::mod;

	using glm::isinf;
	using glm::isnan;

	using glm::max;
	using glm::min;
	using glm::clamp;

	using glm::mix;
	using glm::smoothstep;
	using glm::step;

	using glm::sign;

	using glm::sin;
	using glm::cos;
	using glm::tan;

	using glm::asin;
	using glm::acos;
	using glm::atan;

	using std::sqrt;

	template<typename vec_t> vec_t cMin(vec_t x, vec_t y) {
		vec_t result;
		for(int32 i = 0; i < x.length(); i++) result[i] = akm::min(x[i], y[i]);
		return result;
	}

	template<typename vec_t> vec_t cMax(vec_t x, vec_t y) {
		vec_t result;
		for(int32 i = 0; i < x.length(); i++) result[i] = akm::max(x[i], y[i]);
		return result;
	}

	template<typename scalar_t> scalar_t remainder(scalar_t x, scalar_t y) { return std::remainder(x, y); }
}


#endif
