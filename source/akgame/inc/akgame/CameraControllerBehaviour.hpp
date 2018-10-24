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

#ifndef AKGAME_CAMERACONTROLLERBEHAVIOUR_HPP_
#define AKGAME_CAMERACONTROLLERBEHAVIOUR_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/component/Behaviours.hpp>
#include <akengine/entity/Entity.hpp>
#include <akinput/keyboard/Keyboard.hpp>
#include <akinput/keyboard/Keys.hpp>
#include <akinput/mouse/Mouse.hpp>
#include <akmath/Quaternion.hpp>
#include <akmath/Scalar.hpp>
#include <akmath/Types.hpp>
#include <akmath/Vector.hpp>
#include <akrender/window/Types.hpp>
#include <akrender/window/Window.hpp>
#include <glm/detail/type_vec3.hpp>

namespace akgame {

	class CameraControllerBehaviour final : public ake::Behaviour {
		protected:
			void onStart() override {}

			void onUpdate(fpSingle deltaTime) override {
				auto cameraTransform = entity();

				if (akw::keyboard().wasPressed(akin::Key::LALT)) akw::setCursorMode(akw::cursorMode() == akw::CursorMode::Captured ? akw::CursorMode::Normal : akw::CursorMode::Captured);

				if (akw::cursorMode() == akw::CursorMode::Captured) {
					auto mPos = akw::mouse().deltaPosition();

					auto rotY =  akm::degToRad(mPos.x)/20.0f;
					auto rotX = -akm::degToRad(mPos.y)/20.0f;

					auto eulerRot = akm::toEuler(cameraTransform.rotation());
					eulerRot.x += rotX;
					eulerRot.y += rotY * (akm::abs(eulerRot.x) > akm::degToRad(90) ? -1 : 1);
					cameraTransform.setRotation(akm::fromEuler(eulerRot));
				}

				akm::Vec3 moveDir(0,0,0);
				if (akw::keyboard().isDown(akin::Key::W)) moveDir += akm::Vec3( 0,  0,  1);
				if (akw::keyboard().isDown(akin::Key::S)) moveDir += akm::Vec3( 0,  0, -1);
				if (akw::keyboard().isDown(akin::Key::D)) moveDir += akm::Vec3( 1,  0,  0);
				if (akw::keyboard().isDown(akin::Key::A)) moveDir += akm::Vec3(-1,  0,  0);
				if (akw::keyboard().isDown(akin::Key::R)) moveDir += akm::Vec3( 0,  1,  0);
				if (akw::keyboard().isDown(akin::Key::F)) moveDir += akm::Vec3( 0, -1,  0);

				if (akm::sqrMagnitude(moveDir) > 0) {
					moveDir = akm::normalize(moveDir);
					fpSingle moveSpeed = (akw::keyboard().isDown(akin::Key::LSHIFT) ? 40 : 5) * deltaTime;
					cameraTransform.localMove(moveDir * moveSpeed);
				}
			}

		public:
			CameraControllerBehaviour() : ake::Behaviour("CameraControllerBehaviour") {}
	};

}



#endif /* AKGAME_CAMERACONTROLLERBEHAVIOUR_HPP_ */
