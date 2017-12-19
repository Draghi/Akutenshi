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

#ifndef AK_ENGINE_CAMERA_HPP_
#define AK_ENGINE_CAMERA_HPP_

#include <ak/math/Quaternion.hpp>
#include <ak/math/SphericalCoord.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <glm/gtc/quaternion.hpp>

namespace ake {

	class Camera {
		public:
			Camera() = default;
			virtual ~Camera() = default;

			virtual akm::Vec3 up()      const = 0;
			virtual akm::Vec3 forward() const = 0;
			virtual akm::Vec3 right()   const = 0;

			virtual akm::Vec3 position() const = 0;
			virtual akm::Quat oritentation() const = 0;
	};

	class FPSCamera : public Camera {
		private:
			mutable bool m_isDirty;
			mutable akm::Mat4 m_cache;

			akm::SphericalCoord_t<fpSingle> m_look;
			akm::Vec3 m_pos;

			akm::Mat4& cacheMat() const {
				if (m_isDirty) {
					m_isDirty = false;
					m_cache = m_look.toOrientation();
				}
				return m_cache;
			}

		public:
			FPSCamera() : m_isDirty(true), m_cache(1), m_look(), m_pos(0,0,0) {}

			~FPSCamera() override {}

			void lookLR(fpSingle angle) {
				m_isDirty = true;
				m_look.rotateLR(angle);
			}

			void lookUD(fpSingle angle) {
				m_isDirty = true;
				m_look.rotateUD(angle);
			}

			void moveUp(  fpSingle dist) { m_pos +=  dist*up(); }
			void moveDown(fpSingle dist) { m_pos += -dist*up(); }

			void moveForward( fpSingle dist) { m_pos +=  dist*forward(); }
			void moveBackward(fpSingle dist) { m_pos += -dist*forward(); }

			void moveRight(fpSingle dist) { m_pos +=  dist*right(); }
			void moveLeft( fpSingle dist) { m_pos += -dist*right(); }

			void moveX(fpSingle dist) { m_pos += akm::Vec3(dist,    0,    0); }
			void moveY(fpSingle dist) { m_pos += akm::Vec3(   0, dist,    0); }
			void moveZ(fpSingle dist) { m_pos += akm::Vec3(   0,    0, dist); }

			akm::Vec3 position() { return m_pos; }

			akm::Vec3 up() const override {
				return cacheMat()[1];
			}

			akm::Vec3 forward() const override {
				return cacheMat()[2];
			}

			akm::Vec3 right() const override {
				return cacheMat()[0];
			}

			akm::Vec3 position() const override {
				return m_pos;
			}

			akm::Quat oritentation() const override {
				return akm::quat_cast(cacheMat());
			}
	};
}

#endif
