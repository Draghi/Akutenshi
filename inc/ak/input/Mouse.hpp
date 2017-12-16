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

#ifndef AK_INPUT_MOUSE_HPP_
#define AK_INPUT_MOUSE_HPP_

#include <ak/event/Dispatcher.hpp>
#include <ak/event/Event.hpp>
#include <ak/input/Buttons.hpp>
#include <ak/input/Types.hpp>
#include <ak/math/Vector.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <functional>

namespace ak {
	namespace input {

		class Mouse;

		struct ButtonEventData {
			Button button;
			Action action;
			const Mouse* sender;

			bool  wasPressed(Button buttonVal) const { return (button == buttonVal) && (action == Action::Pressed);  }
			bool wasReleased(Button buttonVal) const { return (button == buttonVal) && (action == Action::Released); }
		};
		AK_DEFINE_EVENT(ButtonEvent, const ButtonEventData, true);

		struct ScrollEventData {
			int32 scrollUp;
			int32 scrollDown;
			int32 scrollLeft;
			int32 scrollRight;
			const Mouse* sender;

			int32 scrollX() { return scrollRight - scrollLeft; }
			int32 scrollY() { return scrollDown - scrollUp; }
		};
		AK_DEFINE_EVENT(ScrollEvent, const ScrollEventData, true);

		struct MoveEventData {
			ak::math::Vec2 position;
			ak::math::Vec2 delta;
			const Mouse* sender;
		};
		AK_DEFINE_EVENT(MoveEvent, const MoveEventData, true);

		class Mouse {
			Mouse(const Mouse&) = delete;
			Mouse& operator=(const Mouse&) = delete;
			public:
				Mouse() = default;
				virtual ~Mouse() = default;

				virtual const ak::event::DispatcherProxy<ButtonEvent>& buttonEvent() = 0;
				virtual const ak::event::DispatcherProxy<ScrollEvent>& scrollEvent() = 0;
				virtual const ak::event::DispatcherProxy<MoveEvent>& moveEvent() = 0;

				virtual bool isUp(Button button) const { return !isDown(button); }
				virtual bool isDown(Button button) const = 0;

				virtual bool wasPressed(Button button) const = 0;
				virtual bool wasReleased(Button button) const = 0;
				virtual bool wasBumped(Button button) const = 0;

				virtual int32 scrollLeft() const = 0;
				virtual int32 scrollRight() const = 0;
				virtual int32 scrollX() const { return scrollRight() - scrollLeft(); }

				virtual int32 scrollUp() const = 0;
				virtual int32 scrollDown() const = 0;
				virtual int32 scrollY() const { return scrollDown() - scrollUp(); }

				virtual ak::math::Vec2 position() const = 0;
				virtual ak::math::Vec2 deltaPosition() const = 0;

				virtual void update() = 0;
		};
	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_INPUT_ALIAS_DISABLE))
namespace akin = ak::input;
#endif

#endif
