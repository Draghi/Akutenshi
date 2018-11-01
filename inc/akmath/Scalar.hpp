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

#ifndef AK_MATH_SCALAR_HPP_
#define AK_MATH_SCALAR_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <glm/common.hpp>
#include <glm/gtc/vec1.hpp>
#include <cmath>

namespace akm {
	using scalar_t = fpSingle;

	constexpr scalar_t PI = static_cast<scalar_t>(3.141592653589793238462643);

	inline scalar_t degToRad(scalar_t deg) {
		constexpr scalar_t conversionFactor = PI/180;
		return deg * conversionFactor;
	}

	inline scalar_t radToDeg(scalar_t rad) {
		constexpr scalar_t conversionFactor = 180/PI;
		return rad * conversionFactor;
	}

	inline scalar_t abs(scalar_t v) { return std::abs(v); }

	inline scalar_t log(scalar_t v) { return std::log(v); }
	inline scalar_t log2(scalar_t v) { return std::log2(v); }

	inline scalar_t ceil(scalar_t v) { return std::ceil(v); }
	inline scalar_t floor(scalar_t v) { return std::floor(v); }
	inline scalar_t round(scalar_t v) { return std::round(v); }
	inline scalar_t trunc(scalar_t v) { return std::trunc(v); }

	inline scalar_t mod(scalar_t v, scalar_t mag) { return std::fmod(v, mag); }
	inline scalar_t pow(scalar_t v, scalar_t mag) { return std::pow(v, mag); }

	inline bool isinf(scalar_t val) { return std::isinf(val); }
	inline bool isnan(scalar_t val) { return std::isnan(val); }

	inline scalar_t mix(scalar_t x, scalar_t y, scalar_t a) { return glm::mix(x,y,a); }

	inline scalar_t sign(scalar_t val) { return glm::sign(val); }

	inline scalar_t sin(scalar_t val) { return std::sin(val); }
	inline scalar_t cos(scalar_t val) { return std::cos(val); }
	inline scalar_t tan(scalar_t val) { return std::tan(val); }

	inline scalar_t sinc(scalar_t val) {
		if (val == 0.f) return 1.f;
		return std::sin(val)/val;
	}

	inline scalar_t asin(scalar_t val) { return std::asin(val); }
	inline scalar_t acos(scalar_t val) { return std::acos(val); }
	inline scalar_t atan(scalar_t val) { return std::atan(val); }
	inline scalar_t atan2(scalar_t y, scalar_t x) { return std::atan2(y, x); }

	inline scalar_t sqrt(scalar_t val) { return std::sqrt(val); }

	inline scalar_t normalizeRange(scalar_t val, scalar_t min, scalar_t max) { return (val - min)/(max - min); }

	inline scalar_t min(scalar_t x, scalar_t y) { return std::fmin(x, y); }
	inline scalar_t max(scalar_t x, scalar_t y) { return std::fmax(x, y); }
	inline scalar_t clamp(scalar_t val, scalar_t minVal, scalar_t maxVal) { return akm::max(akm::min(val, maxVal), minVal); }

	inline scalar_t remainder(scalar_t x, scalar_t y) { return std::remainder(x, y); }

	inline scalar_t remap(scalar_t v, scalar_t oMin, scalar_t oMax, scalar_t nMin, scalar_t nMax) {
		return (v - oMin)/(oMax - oMin) * (nMax - nMin) + nMin;
	}
}


#endif
