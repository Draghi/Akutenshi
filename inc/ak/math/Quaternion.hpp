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

#include <ak/math/Types.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

namespace akm {
	inline Quat lerp(const Quat& l, const Quat& r, scalar_t a) { return glm::lerp(l, r, a); }
	inline Quat slerp(const Quat& l, const Quat& r, scalar_t a) { return glm::slerp(l, r, a); }
	inline Quat conjugate(const Quat& x) { return glm::conjugate(x); }

	inline Quat quat_cast(const Mat3& x) { return glm::quat_cast(x); }
	inline Quat quat_cast(const Mat4& x) { return glm::quat_cast(x); }
	inline Mat3 mat3_cast(const Quat& x) { return glm::mat3_cast(x); }
	inline Mat4 mat4_cast(const Quat& x) { return glm::mat4_cast(x); }

	inline Quat rotateQ(scalar_t angle, const akm::Vec3& axis) { return glm::rotate(akm::Quat(1,0,0,0), angle, axis); }
}

#endif
