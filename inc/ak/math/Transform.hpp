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

#ifndef AK_MATH_TRANSFORM_HPP_
#define AK_MATH_TRANSFORM_HPP_

#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <ak/math/Matrix.hpp>
#include <ak/math/Quaternion.hpp>
#include <ak/math/Types.hpp>

namespace akm {

	class Transform3D {
		private:
			akm::Vec3 m_position;
			akm::Quat m_rotation;

		public:
			Transform3D() : m_position(), m_rotation() {}
			Transform3D(const Transform3D& other) : m_position(other.m_position), m_rotation(other.m_rotation) {}
			Transform3D(const akm::Vec3& position, const akm::Quat& rotation) : m_position(position), m_rotation(rotation) {}

			akm::Mat4 composeModelMatrix() const { return akm::translate(m_position) * akm::mat4_cast(m_rotation); }
			akm::Mat4 composeViewMatrix()  const { return akm::transpose(akm::mat4_cast(m_rotation)) * akm::translate(m_position); }

			akm::Vec3& position() { return m_position; }
			akm::Quat& rotation() { return m_rotation; }

			akm::Vec3 position() const { return m_position; }
			akm::Quat rotation() const { return m_rotation; }

			Transform3D& operator=(const Transform3D& other) { m_position = other.m_position; m_rotation = other.m_rotation; return *this; }
	};

}

#endif
