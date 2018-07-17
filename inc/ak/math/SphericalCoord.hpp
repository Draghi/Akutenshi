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

#ifndef AK_MATH_SPHERICALCOORD_HPP_
#define AK_MATH_SPHERICALCOORD_HPP_

#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>

#include <ak/math/Quaternion.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/math/Types.hpp>
#include <ak/math/Vector.hpp>

namespace akm {

	inline akm::Mat4 orientToward(akm::Vec3 direction, akm::Vec3 up) {
		auto lookVec = akm::normalize(direction);

		auto zVec = lookVec;
		auto xVec = akm::normalize(akm::cross(  up, zVec));
		auto yVec = akm::normalize(akm::cross(zVec, xVec));

		akm::Mat4 result(1);
		result[0] = akm::Vec4(xVec, 0);
		result[1] = akm::Vec4(yVec, 0);
		result[2] = akm::Vec4(zVec, 0);

		return result;
	}

	class SphericalCoord {
		private:
			using CartesianCoord = akm::Vec<3>;

			scalar_t m_radius;
			scalar_t m_polar;
			scalar_t m_azimuth;

		public:
			using value_type = scalar_t;

			SphericalCoord() : m_radius(1), m_polar(0), m_azimuth(akm::PI/2) {}

			void setRadius(scalar_t radius) {
				m_radius = radius;
			}

			void rotateLR(scalar_t hAngle) {
				m_polar += hAngle;
			}

			void rotateUD(scalar_t vAngle) {
				m_azimuth = akm::remainder(m_azimuth + vAngle, 2*akm::PI);
			}

			CartesianCoord toCartesianCoord() const {
				return akm::Vec3(
					m_radius*akm::sin(m_polar)* akm::sin(m_azimuth),
					m_radius*akm::cos(m_azimuth),
					m_radius*akm::cos(m_polar)* akm::sin(m_azimuth));
			}

			#pragma clang diagnostic push
			#pragma clang diagnostic ignored "-Wfloat-equal"
			akm::Mat4 toOrientation() const {
				if (m_azimuth == 0) {
					return orientToward(akm::Vec3(0, 1, 0), akm::Vec3(-akm::sin(m_polar), 0, -akm::cos(m_polar)));
				} else if (akm::mod(m_azimuth, akm::PI) == 0) {
					return orientToward(akm::Vec3(0, -1, 0), akm::Vec3(akm::sin(m_polar), 0, akm::cos(m_polar)));
				} else {
					auto up = akm::Vec3(0, m_azimuth >= 0 ? 1 : -1, 0);
					auto coord = toCartesianCoord();
					return orientToward(coord, up);
				}
			}
			#pragma clang diagnostic pop

			akm::Vec3 getOrientationUp() const {
				return toOrientation()[1];
			}

			scalar_t radius() const { return m_radius; }
			scalar_t polar() const { return m_polar; }
			scalar_t azimuth() const { return m_azimuth; }
	};

}

#endif
