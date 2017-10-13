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

#include <ak/event/Event.hpp>
#include <ak/event/Util.hpp>
#include <ak/input/Buttons.hpp>
#include <ak/input/Types.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <glm/detail/type_vec2.hpp>
#include <functional>

namespace ak {
	namespace input {

		class ButtonActionEvent;
		class MouseMoveEvent;

		class Mouse {
			Mouse(const Mouse&) = delete;
			Mouse& operator=(const Mouse&) = delete;
			public:
				using MouseAction_f = std::function<void(ButtonActionEvent&)>;
				using MouseMove_f = std::function<void(MouseMoveEvent&)>;

				Mouse() = default;
				virtual ~Mouse() = default;

				virtual bool isUp(Button button) const { return !isDown(button); }
				virtual bool isDown(Button button) const = 0;

				virtual bool wasPressed(Button button) const = 0;
				virtual bool wasReleased(Button button) const = 0;

				virtual ak::math::DVec2 scroll() const = 0;
				virtual fpDouble scrollHorz() const { return pos().x; }
				virtual fpDouble ScrollVert() const { return pos().y; }

				virtual ak::math::DVec2 pos() const = 0;
				virtual fpDouble x() const { return pos().x; }
				virtual fpDouble y() const { return pos().y; }

				virtual ak::math::DVec2 delta() const = 0;
				virtual fpDouble dx() const { return delta().x; }
				virtual fpDouble dy() const { return delta().y; }

				virtual ak::event::EventID subscribeAction(const MouseAction_f& callback) = 0;
				virtual ak::event::EventID subscribeAction(ak::event::Subscription& subscriber, const MouseAction_f& callback) = 0;
				virtual void unsubscribeAction(ak::event::EventID eventID) = 0;
				virtual void unsubscribeAction(ak::event::Subscription& subscriber) = 0;

				virtual ak::event::EventID subscribeMovement(const MouseMove_f& callback) = 0;
				virtual ak::event::EventID subscribeMovement(ak::event::Subscription& subscriber, const MouseMove_f& callback) = 0;
				virtual void unsubscribeMovement(ak::event::EventID eventID) = 0;
				virtual void unsubscribeMovement(ak::event::Subscription& subscriber) = 0;

				virtual void update() = 0;

		};

		class MouseMoveEvent final : public ak::event::Event {
			AK_IMPLEMENT_EVENT("MouseMoveEvent", true)
			private:
				ak::math::DVec2 m_oldPos;
				ak::math::DVec2 m_newPos;
				const Mouse& m_sender;
			public:
				MouseMoveEvent(ak::math::DVec2 oPos, ak::math::DVec2 nPos, const Mouse& sender) : m_oldPos(oPos), m_newPos(nPos), m_sender(sender) {}

				ak::math::DVec2 oPos() const { return m_oldPos; }
				fpDouble oX() const { return m_oldPos.x; }
				fpDouble oY() const { return m_oldPos.y; }

				ak::math::DVec2 pos() const { return m_newPos; }
				fpDouble x() const { return m_newPos.x; }
				fpDouble y() const { return m_newPos.y; }

				ak::math::DVec2 dPos() const { return m_newPos - m_oldPos; }
				ak::math::DVec2 dx() const { return dPos().x; }
				ak::math::DVec2 dy() const { return dPos().y; }

				const Mouse& sender() const { return m_sender; }
		};

		class ButtonActionEvent final : public ak::event::Event {
			AK_IMPLEMENT_EVENT("ButtonActionEvent", true)
			private:
				Button m_button;
				Action m_action;
				uint8 m_actionCount;
				const Mouse& m_sender;
			public:
				ButtonActionEvent(Button buttonVal, Action actionVal, uint8 actionCountVal, Mouse& senderVal) : m_button(buttonVal), m_action(actionVal), m_actionCount(actionCountVal), m_sender(senderVal) {}

				Button button() const { return m_button; }
				Action action() const { return m_action; }
				uint8 actionCount() const { return m_actionCount; }
				const Mouse& sender() const { return m_sender; }

				bool  wasPressed(Button buttonVal) const { return (m_button == buttonVal) && (m_action == Action::Pressed);  }
				bool wasReleased(Button buttonVal) const { return (m_button == buttonVal) && (m_action == Action::Released); }
		};
	}
}

#endif
