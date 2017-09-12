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

#ifndef AK_MATH_QUATERNION_HPP_
#define AK_MATH_QUATERNION_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ak {
	namespace math {
		using  Quat = glm::tquat<fpSingle, glm::highp>;
		using DQuat = glm::tquat<fpDouble, glm::highp>;

		using glm::lerp;
		using glm::slerp;
		using glm::conjugate;

	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_MATH_ALIAS_DISABLE))
namespace akm = ak::math;
#endif

#endif