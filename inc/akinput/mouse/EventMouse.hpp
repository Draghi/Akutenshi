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

#ifndef AK_INPUT_MOUSE_EVENTINPUT_HPP_
#define AK_INPUT_MOUSE_EVENTINPUT_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/event/Dispatcher.hpp>
#include <akengine/thread/DoubleBuffer.hpp>
#include <akinput/mouse/Buttons.hpp>
#include <akinput/mouse/Mouse.hpp>
#include <akinput/Types.hpp>
#include <akmath/Types.hpp>
#include <array>
#include <utility>


namespace akin {

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

			akt::DoubleBuffer<EventRecord> m_eventBuffer;

			akev::Dispatcher<ButtonEvent> m_buttonEventDispatcher;
			akev::DispatcherProxy<ButtonEvent> m_buttonEventProxy = m_buttonEventDispatcher;

			akev::Dispatcher<ScrollEvent> m_scrollEventDispatcher;
			akev::DispatcherProxy<ScrollEvent> m_scrollEventProxy = m_scrollEventDispatcher;

			akev::Dispatcher<MoveEvent> m_moveEventDispatcher;
			akev::DispatcherProxy<MoveEvent> m_moveEventProxy = m_moveEventDispatcher;


			akm::Vec2 m_mousePosition;
			akm::Vec2 m_lastPosition;

			std::array<std::pair<Action, State>, static_cast<akSize>(Button::BUTTON_LAST)> m_buttonStates;

			int32 m_scrollUp;
			int32 m_scrollDown;
			int32 m_scrollLeft;
			int32 m_scrollRight;

			Action getButtonAction(Button key) const;
			State getButtonState(Button key) const;

		public:
			EventMouse();
			~EventMouse() override;

			const akev::DispatcherProxy<ButtonEvent>& buttonEvent() override;
			const akev::DispatcherProxy<ScrollEvent>& scrollEvent() override;
			const akev::DispatcherProxy<MoveEvent>& moveEvent() override;

			bool isDown(Button button) const override;

			bool wasPressed(Button button) const override;
			bool wasReleased(Button button) const override;
			bool wasBumped(Button button) const override;

			int32 scrollLeft() const override;
			int32 scrollRight() const override;

			int32 scrollUp() const override;
			int32 scrollDown() const override;

			akm::Vec2 position() const override;
			akm::Vec2 deltaPosition() const override;

			void update() override;

			void onButtonEvent(const ButtonEventData& data);
			void onScrollEvent(const ScrollEventData& data);
			void onMoveEvent(const MoveEventData& data);
	};
}


#endif
