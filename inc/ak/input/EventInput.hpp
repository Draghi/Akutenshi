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

#ifndef AK_INPUT_EVENTINPUT_HPP_
#define AK_INPUT_EVENTINPUT_HPP_

#include <ak/container/DoubleBuffer.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/input/Keyboard.hpp>
#include <ak/input/Keys.hpp>
#include <ak/input/Mouse.hpp>
#include <ak/input/Buttons.hpp>
#include <ak/input/Types.hpp>
#include <stddef.h>
#include <array>
#include <utility>

namespace ak {
	namespace input {

		class EventKeyboard final : public Keyboard {
			private:
				ak::container::DoubleBuffer<KeyEventData> m_keyEventBuffer;

				ak::event::Dispatcher<KeyEvent> m_keyEventDispatcher;
				ak::event::DispatcherProxy<KeyEvent> m_keyEventProxy = m_keyEventDispatcher;

				std::array<std::pair<Action, State>, static_cast<size_t>(Key::KEY_LAST_NORMAL)> m_keyStates;

				Action getKeyAction(Key key) const;
				State getKeyState(Key key) const;

			public:
				EventKeyboard();
				~EventKeyboard() override;

				const ak::event::DispatcherProxy<KeyEvent>& keyEvent() override;

				bool isDown(Key key) const override;

				bool wasPressed(Key key) const override;
				bool wasReleased(Key key) const override;
				bool wasBumped(Key key) const override;

				void update() override;

				void onKeyEvent(const KeyEventData& data);
		};

		class EventMouse final : public Mouse {
			private:
				enum EventType : uint8 {
					ButtonType,
					ScrollType,
					MoveType
				};
				struct EventRecord {
					EventType eventType;
					union {
						ButtonEventData button;
						ScrollEventData scroll;
						MoveEventData move;
					} eventData;

					EventRecord(ButtonEventData buttonEvent) : eventType(ButtonType) { eventData.button = buttonEvent; }
					EventRecord(ScrollEventData scrollEvent) : eventType(ScrollType) { eventData.scroll = scrollEvent; }
					EventRecord(MoveEventData moveEvent) : eventType(MoveType) { eventData.move = moveEvent; }

					EventRecord(const EventRecord& other) { *this = other; }

					EventRecord& operator=(const EventRecord& other) {
						eventType = other.eventType;
						switch(eventType) {
							case ButtonType: eventData.button = other.eventData.button; return *this;
							case ScrollType: eventData.scroll = other.eventData.scroll; return *this;
							case MoveType:   eventData.move   = other.eventData.move;   return *this;
						}
					}
				};

				ak::container::DoubleBuffer<EventRecord> m_eventBuffer;

				ak::event::Dispatcher<ButtonEvent> m_buttonEventDispatcher;
				ak::event::DispatcherProxy<ButtonEvent> m_buttonEventProxy = m_buttonEventDispatcher;

				ak::event::Dispatcher<ScrollEvent> m_scrollEventDispatcher;
				ak::event::DispatcherProxy<ScrollEvent> m_scrollEventProxy = m_scrollEventDispatcher;

				ak::event::Dispatcher<MoveEvent> m_moveEventDispatcher;
				ak::event::DispatcherProxy<MoveEvent> m_moveEventProxy = m_moveEventDispatcher;


				ak::math::DVec2 m_mousePosition;
				ak::math::DVec2 m_lastPosition;

				std::array<std::pair<Action, State>, static_cast<size_t>(Button::BUTTON_LAST)> m_buttonStates;

				int32 m_scrollUp;
				int32 m_scrollDown;
				int32 m_scrollLeft;
				int32 m_scrollRight;

				Action getButtonAction(Button key) const;
				State getButtonState(Button key) const;

			public:
				EventMouse();
				~EventMouse() override;

				const ak::event::DispatcherProxy<ButtonEvent>& buttonEvent() override;
				const ak::event::DispatcherProxy<ScrollEvent>& scrollEvent() override;
				const ak::event::DispatcherProxy<MoveEvent>& moveEvent() override;

				bool isDown(Button button) const override;

				bool wasPressed(Button button) const override;
				bool wasReleased(Button button) const override;
				bool wasBumped(Button button) const override;

				int32 scrollLeft() const override;
				int32 scrollRight() const override;

				int32 scrollUp() const override;
				int32 scrollDown() const override;

				ak::math::DVec2 position() const override;
				ak::math::DVec2 deltaPosition() const override;

				void update() override;

				void onButtonEvent(const ButtonEventData& data);
				void onScrollEvent(const ScrollEventData& data);
				void onMoveEvent(const MoveEventData& data);
		};


	}
}

#endif
