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

#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/debug/Log.hpp>
#include <akengine/event/Dispatcher.hpp>
#include <akinput/mouse/Buttons.hpp>
#include <akinput/mouse/EventMouse.hpp>
#include <akinput/mouse/Mouse.hpp>
#include <akinput/Types.hpp>
#include <akmath/Types.hpp>
#include <crtdefs.h>

using namespace akin;

Action EventMouse::getButtonAction(Button key) const { return m_buttonStates[static_cast<size_t>(key)].first; }
State EventMouse::getButtonState(Button key) const { return m_buttonStates[static_cast<size_t>(key)].second; }

EventMouse::EventMouse() = default;
EventMouse::~EventMouse() = default;

const akev::DispatcherProxy<ButtonEvent>& EventMouse::buttonEvent() { return m_buttonEventProxy; }
const akev::DispatcherProxy<ScrollEvent>& EventMouse::scrollEvent() { return m_scrollEventProxy; }
const akev::DispatcherProxy<MoveEvent>& EventMouse::moveEvent() { return m_moveEventProxy; }

bool EventMouse::isDown(Button button) const { return getButtonState(button) == State::Down; }

bool EventMouse::wasPressed(Button button) const { return getButtonAction(button) == Action::Pressed; }
bool EventMouse::wasReleased(Button button) const { return getButtonAction(button) == Action::Released; }
bool EventMouse::wasBumped(Button button) const { return getButtonAction(button) == Action::Bumped; }

int32 EventMouse::scrollLeft() const { return m_scrollLeft; }
int32 EventMouse::scrollRight() const { return m_scrollRight; }

int32 EventMouse::scrollUp() const { return m_scrollUp; }
int32 EventMouse::scrollDown() const { return m_scrollDown; }

akm::Vec2 EventMouse::position() const { return m_mousePosition; }
akm::Vec2 EventMouse::deltaPosition() const { return m_mousePosition - m_lastPosition; }

void EventMouse::update() {
	m_eventBuffer.swap();
	for(auto iter = m_buttonStates.begin(); iter != m_buttonStates.end(); iter++) iter->first = Action::None;

	m_scrollUp = 0;
	m_scrollDown = 0;
	m_scrollLeft = 0;
	m_scrollRight = 0;

	m_lastPosition = m_mousePosition;

	m_eventBuffer.iterate([this](size_t, EventRecord& eventData) {
		switch(eventData.eventType) {
			case ButtonType: {
				auto& eData = eventData.eventData.button;

				if (eData.button >= Button::BUTTON_LAST) {
					akl::Logger("EventMouse").warn("Unknown button pressed - ", static_cast<size_t>(eData.button));
				} else if ((eData.action == Action::Pressed) || (eData.action == Action::Released) || (eData.action == Action::Bumped)) {
					ButtonEvent event(eData);
					m_buttonEventDispatcher.send(event);
					m_buttonStates[static_cast<size_t>(eData.button)] = std::make_pair(eData.action, eData.action == Action::Pressed ? State::Down : State::Up);
				}

				return;
			}

			case ScrollType: {
				ScrollEvent event(eventData.eventData.scroll);
				m_scrollEventDispatcher.send(event);
				m_scrollUp    += eventData.eventData.scroll.scrollUp;
				m_scrollDown  += eventData.eventData.scroll.scrollDown;
				m_scrollLeft  += eventData.eventData.scroll.scrollLeft;
				m_scrollRight += eventData.eventData.scroll.scrollRight;
				return;
			}

			case MoveType: {
				MoveEvent event(eventData.eventData.move);
				m_moveEventDispatcher.send(event);
				m_mousePosition = eventData.eventData.move.position;
				return;
			}
		}
	});
}

void EventMouse::onButtonEvent(const ButtonEventData& data) { m_eventBuffer.push_back(EventRecord(data)); }
void EventMouse::onScrollEvent(const ScrollEventData& data) { m_eventBuffer.push_back(EventRecord(data)); }
void EventMouse::onMoveEvent(const MoveEventData& data) { m_eventBuffer.push_back(EventRecord(data)); }
