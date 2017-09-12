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

#include <glm/glm.hpp>

namespace ak {
	namespace math {

		constexpr fpDouble PI = 3.141592653589793238462643;

		template<typename scalar_t> scalar_t degToRad(scalar_t deg) {
			constexpr scalar_t conversionFactor = static_cast<scalar_t>(180/PI);
			return deg * conversionFactor;
		}

		template<typename scalar_t> scalar_t radToDeg(scalar_t rad) {
			constexpr scalar_t conversionFactor = static_cast<scalar_t>(PI/180);
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


	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_MATH_ALIAS_DISABLE))
namespace akm = ak::math;
#endif

#endif
