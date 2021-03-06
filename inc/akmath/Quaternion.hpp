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

#ifndef AK_MATH_QUATERNION_HPP_
#define AK_MATH_QUATERNION_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/data/PValue.hpp>
#include <akengine/data/SmartClass.hpp>
#include <akmath/Matrix.hpp>
#include <akmath/Scalar.hpp>
#include <akmath/Vector.hpp>
#include <glm/common.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x3.hpp>

namespace akm {
	using Quat = glm::tquat<fpSingle, glm::highp>;

	inline Quat lerp(const Quat& l, const Quat& r, scalar_t a) { return glm::lerp(l, r, a); }
	inline Quat slerp(const Quat& l, const Quat& r, scalar_t a) { return glm::slerp(l, r, a); }
	inline Quat conjugate(const Quat& x) { return glm::conjugate(x); }

	inline Quat quat_cast(const Mat3& x) { return glm::quat_cast(x); }
	inline Quat quat_cast(const Mat4& x) { return glm::quat_cast(x); }
	inline Quat quat_cast(const Mat4x3& x) { return quat_cast(akm::Mat3(x)); }

	inline Mat3 mat3_cast(const Quat& x) { return glm::mat3_cast(x); }
	inline Mat4 mat4_cast(const Quat& x) { return glm::mat4_cast(x); }
	inline Mat4x3 mat4x3_cast(const Quat& x) { return Mat4x3(glm::mat4_cast(x)); }

	inline Quat rotateQ(scalar_t angle, const akm::Vec3& axis) { return glm::rotate(akm::Quat(1,0,0,0), angle, axis); }

	inline Quat inverse(const Quat& q) { return glm::inverse(q); }
	inline Quat normalize(const Quat& q) { return glm::normalize(q); }

	inline Quat fromEuler(const akm::Vec3& euler) {
		return rotateQ(euler.y, {0, 1, 0}) * rotateQ(euler.z, {0, 0, 1}) * rotateQ(euler.x, {1, 0, 0});
	}

	inline Vec3 toEuler(const Quat& q) {
		double test = q.x*q.y + q.z*q.w;

		if (test >  0.499) return {0,  2*akm::atan2(q.x,q.w),  akm::PI/2}; // singularity at north pole
		if (test < -0.499) return {0, -2*akm::atan2(q.x,q.w), -akm::PI/2}; // singularity at south pole

	    scalar_t sqx = q.x*q.x;
	    scalar_t sqy = q.y*q.y;
	    scalar_t sqz = q.z*q.z;

	    return {
	    	akm::atan2(2*q.x*q.w - 2*q.y*q.z, 1 - 2*sqx - 2*sqz),
	    	akm::atan2(2*q.y*q.w - 2*q.x*q.z, 1 - 2*sqy - 2*sqz),
			akm::asin(2*test)
	    };
	}

	inline Vec3 extractAxisX(const Quat& q) {
		scalar_t qyy(q.y * q.y);
		scalar_t qzz(q.z * q.z);
		scalar_t qxz(q.x * q.z);
		scalar_t qxy(q.x * q.y);
		scalar_t qwy(q.w * q.y);
		scalar_t qwz(q.w * q.z);

		return {1 - 2 * (qyy +  qzz), 2 * (qxy + qwz), 2 * (qxz - qwy)};
	}

	inline Vec3 extractAxisY(const Quat& q) {
		scalar_t qxx(q.x * q.x);
		scalar_t qzz(q.z * q.z);
		scalar_t qxy(q.x * q.y);
		scalar_t qyz(q.y * q.z);
		scalar_t qwx(q.w * q.x);
		scalar_t qwz(q.w * q.z);

		return {2 * (qxy - qwz), 1 - 2 * (qxx +  qzz), 2 * (qyz + qwx)};
	}

	inline Vec3 extractAxisZ(const Quat& q) {
		scalar_t qxx(q.x * q.x);
		scalar_t qyy(q.y * q.y);
		scalar_t qxz(q.x * q.z);
		scalar_t qyz(q.y * q.z);
		scalar_t qwx(q.w * q.x);
		scalar_t qwy(q.w * q.y);

		return {2 * (qxz + qwy), 2 * (qyz - qwx), 1 - 2 * (qxx +  qyy)};
	}

	inline void decompose(const Mat4& mat, Vec3& translation, Quat& rotation, Vec3& scale) {
		akm::Vec3 skew; akm::Vec4 persp;
		glm::decompose(mat, scale, rotation, translation, skew, persp);
	}
}

AK_SMART_CLASS_ARRAY(akm::Quat, akm::Quat::length());

#endif
