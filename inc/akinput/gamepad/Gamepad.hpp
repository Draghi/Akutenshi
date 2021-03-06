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

#ifndef AK_INPUT_GAMEPAD_GAMEPAD_HPP_
#define AK_INPUT_GAMEPAD_GAMEPAD_HPP_

#include <akengine/event/Event.hpp>
#include <akinput/gamepad/Analog.hpp>
#include <akinput/gamepad/GPButton.hpp>
#include <akinput/Types.hpp>

namespace akin {

	class Gamepad;

	struct GPButtonEventData {
		GPButton button;
		Action action;
		Gamepad& sender;

		bool  wasPressed(GPButton keyVal) const { return (button == keyVal) && (action == Action::Pressed);  }
		bool wasReleased(GPButton keyVal) const { return (button == keyVal) && (action == Action::Released); }
	};
	AK_DEFINE_EVENT(GPButtonEvent, const GPButtonEventData, true);

	struct AxisMoveEventData {
		Analog axis;
		akc::math::DVec2 degree;
		akc::math::DVec2 delta;
		Gamepad& sender;
	};
	AK_DEFINE_EVENT(AxisMoveEvent, const AxisMoveEventData, true);

	class Gamepad {
		Gamepad(const Gamepad&) = delete;
		Gamepad& operator=(const Gamepad&) = delete;
		public:
			Gamepad() = default;
			virtual ~Gamepad() = default;

			virtual const akc::event::DispatcherProxy<GPButtonEvent>& buttonEvent() = 0;
			virtual const akc::event::DispatcherProxy<AxisMoveEvent>& analogEvent() = 0;

			virtual bool isUp(GPButton buttonVal) const { return !isDown(buttonVal); }
			virtual bool isDown(GPButton buttonVal) const = 0;

			virtual bool wasPressed(GPButton buttonVal) const = 0;
			virtual bool wasReleased(GPButton buttonVal) const = 0;

			virtual akc::math::DVec2 degree(Analog axisVal) const = 0;
			virtual akc::math::DVec2 deltaDegree(Analog axisVal) const = 0;

			virtual void update() = 0;
	};
}

#endif
