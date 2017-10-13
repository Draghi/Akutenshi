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

#ifndef AK_INPUT_GAMEPAD_HPP_
#define AK_INPUT_GAMEPAD_HPP_

#include <ak/event/Event.hpp>
#include <ak/event/Util.hpp>
#include <ak/input/Analog.hpp>
#include <ak/input/GPButton.hpp>
#include <ak/input/Types.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <functional>

namespace ak {
	namespace input {

		class GamepadButtonEvent;
		class GamepadAnalogEvent;

		class Gamepad {
			public:
				using GamepadButton_f = std::function<void(GamepadButtonEvent&)>;
				using GamepadAnalog_f = std::function<void(GamepadAnalogEvent&)>;

				virtual bool isUp(GPButton buttonVal) { return !isDown(buttonVal); }
				virtual bool isDown(GPButton buttonVal) = 0;

				virtual bool wasPressed(GPButton buttonVal) = 0;
				virtual bool wasReleased(GPButton buttonVal) = 0;

				virtual ak::math::DVec2 axis(Analog axisVal) = 0;
				virtual fpDouble degree(Analog axisVal) { return ak::math::length(axis(axisVal)); }
				virtual fpDouble xAxis(Analog axisVal) { return axis(axisVal).x; }
				virtual fpDouble yAxis(Analog axisVal) { return axis(axisVal).y; }

				virtual ak::event::EventID subscribeButton(const GamepadButton_f& callback) = 0;
				virtual ak::event::EventID subscribeButton(ak::event::Subscription& subscriber, const GamepadButton_f& callback) = 0;
				virtual void unsubscribeButton(ak::event::EventID eventID) = 0;
				virtual void unsubscribeButton(ak::event::Subscription& subscriber) = 0;

				virtual ak::event::EventID subscribeAnalog(const GamepadAnalog_f& callback) = 0;
				virtual ak::event::EventID subscribeAnalog(ak::event::Subscription& subscriber, const GamepadAnalog_f& callback) = 0;
				virtual void unsubscribeAnalog(ak::event::EventID eventID) = 0;
				virtual void unsubscribeAnalog(ak::event::Subscription& subscriber) = 0;

				virtual void update() = 0;
		};

		class GamepadButtonEvent final : public ak::event::Event {
			AK_IMPLEMENT_EVENT("GamepadButtonEvent", true)
			private:
				GPButton m_button;
				Action m_action;
				const Gamepad& m_sender;
			public:
				GamepadButtonEvent(GPButton buttonVal, Action actionVal, Gamepad& senderVal) : m_button(buttonVal), m_action(actionVal), m_sender(senderVal) {}

				GPButton button() const { return m_button; }
				Action action() const { return m_action; }
				const Gamepad& sender() const { return m_sender; }

				bool  wasPressed(GPButton keyVal) const { return (m_button == keyVal) && (m_action == Action::Pressed);  }
				bool wasReleased(GPButton keyVal) const { return (m_button == keyVal) && (m_action == Action::Released); }
		};

		class GamepadAnalogEvent final : public ak::event::Event {
			AK_IMPLEMENT_EVENT("GamepadAnalogEvent", true)
			private:
				Analog m_axis;
				ak::math::DVec2 m_oldAxis;
				ak::math::DVec2 m_newAxis;
				const Gamepad& m_sender;
			public:
				GamepadAnalogEvent(Analog axis, ak::math::DVec2 oAxis, ak::math::DVec2 nAxis, const Gamepad& sender) : m_axis(axis), m_oldAxis(oAxis), m_newAxis(nAxis), m_sender(sender) {}

				Analog analog() const { return m_axis; }

				ak::math::DVec2 oAxis() const { return m_oldAxis; }
				fpDouble oDegree() const { return ak::math::length(m_oldAxis); }
				fpDouble oXAxis() const { return m_oldAxis.x; }
				fpDouble oYAxis() const { return m_oldAxis.y; }

				ak::math::DVec2 axis() const { return m_newAxis; }
				fpDouble degree() const { return ak::math::length(m_newAxis); }
				fpDouble xAxis() const { return m_newAxis.x; }
				fpDouble yAxis() const { return m_newAxis.y; }

				ak::math::DVec2 dAxis() const { return m_newAxis - m_oldAxis; }
				ak::math::DVec2 dDegree() const { return ak::math::length(dAxis()); }
				ak::math::DVec2 dXAxis() const { return dAxis().x; }
				ak::math::DVec2 dYAxis() const { return dAxis().y; }

				const Gamepad& sender() const { return m_sender; }
		};

	}
}

#endif
