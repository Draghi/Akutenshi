/*
 * CameraControllerBehaviour.hpp
 *
 *  Created on: 8 Jun. 2018
 *      Author: Draghi
 */

#ifndef AKGAME_CAMERACONTROLLERBEHAVIOUR_HPP_
#define AKGAME_CAMERACONTROLLERBEHAVIOUR_HPP_

#include <ak/engine/components/Behaviours.hpp>
#include <ak/input/Keyboard.hpp>
#include <ak/input/Keys.hpp>
#include <ak/input/Mouse.hpp>
#include <ak/math/Quaternion.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/window/Types.hpp>
#include <ak/window/Window.hpp>

namespace akgame {

	class CameraControllerBehaviour final : public ake::Behaviour {
		protected:
			void onStart() override {}

			void onUpdate(fpSingle deltaTime) override {
				auto cameraTransform = transform();

				if (akw::keyboard().wasPressed(akin::Key::LALT)) akw::setCursorMode(akw::cursorMode() == akw::CursorMode::Captured ? akw::CursorMode::Normal : akw::CursorMode::Captured);

				if (akw::cursorMode() == akw::CursorMode::Captured) {
					auto mPos = akw::mouse().deltaPosition();

					auto rotY =  akm::degToRad(mPos.x)/20.0f;
					auto rotX = -akm::degToRad(mPos.y)/20.0f;

					auto eulerRot = cameraTransform.rotationEuler();
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
					cameraTransform.moveLocal(moveDir * moveSpeed);
				}
			}

		public:
			CameraControllerBehaviour() : ake::Behaviour("CameraControllerBehaviour") {}
	};

}



#endif /* AKGAME_CAMERACONTROLLERBEHAVIOUR_HPP_ */
