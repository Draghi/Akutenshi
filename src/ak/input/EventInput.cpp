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

#include <ak/input/EventInput.hpp>

using namespace ak::input;

Action EventKeyboard::getKeyAction(Key key) const { return m_keyStates[static_cast<size_t>(key)].first; }
State EventKeyboard::getKeyState(Key key) const { return m_keyStates[static_cast<size_t>(key)].second; }

EventKeyboard::EventKeyboard() = default;
EventKeyboard::~EventKeyboard() = default;

const ak::event::DispatcherProxy<KeyEvent>& EventKeyboard::keyEvent() { return m_keyEventProxy; }

bool EventKeyboard::isDown(Key key) const { return getKeyState(key) == State::Down; }

bool EventKeyboard::wasPressed(Key key) const { return (getKeyAction(key) == Action::Pressed) || (getKeyAction(key) == Action::Bumped); }
bool EventKeyboard::wasReleased(Key key) const { return (getKeyAction(key) == Action::Released) || (getKeyAction(key) == Action::Bumped); }
bool EventKeyboard::wasBumped(Key key) const { return getKeyAction(key) == Action::Bumped; }

void EventKeyboard::update() {
	m_keyEventBuffer.swap();

	for(auto iter = m_keyStates.begin(); iter != m_keyStates.end(); iter++) iter->first = Action::None;

	m_keyEventBuffer.iterate([this](size_t, KeyEventData& eventData) {
		KeyEvent event(eventData);
		m_keyEventDispatcher.send(event);
		if ((eventData.action == Action::Pressed) || (eventData.action == Action::Released) || (eventData.action == Action::Bumped)) {
			m_keyStates[static_cast<size_t>(eventData.key)] = std::make_pair(eventData.action, eventData.action == Action::Pressed ? State::Down : State::Up);
		}
	});
}

void EventKeyboard::onKeyEvent(const KeyEventData& data) { m_keyEventBuffer.push_back(data); }


Action EventMouse::getButtonAction(Button key) const { return m_buttonStates[static_cast<size_t>(key)].first; }
State EventMouse::getButtonState(Button key) const { return m_buttonStates[static_cast<size_t>(key)].second; }

EventMouse::EventMouse() = default;
EventMouse::~EventMouse() = default;

const ak::event::DispatcherProxy<ButtonEvent>& EventMouse::buttonEvent() { return m_buttonEventProxy; }
const ak::event::DispatcherProxy<ScrollEvent>& EventMouse::scrollEvent() { return m_scrollEventProxy; }
const ak::event::DispatcherProxy<MoveEvent>& EventMouse::moveEvent() { return m_moveEventProxy; }

bool EventMouse::isDown(Button button) const { return getButtonState(button) == State::Down; }

bool EventMouse::wasPressed(Button button) const { return getButtonAction(button) == Action::Pressed; }
bool EventMouse::wasReleased(Button button) const { return getButtonAction(button) == Action::Released; }
bool EventMouse::wasBumped(Button button) const { return getButtonAction(button) == Action::Bumped; }

int32 EventMouse::scrollLeft() const { return m_scrollLeft; }
int32 EventMouse::scrollRight() const { return m_scrollRight; }

int32 EventMouse::scrollUp() const { return m_scrollUp; }
int32 EventMouse::scrollDown() const { return m_scrollDown; }

ak::math::Vec2 EventMouse::position() const { return m_mousePosition; }
ak::math::Vec2 EventMouse::deltaPosition() const { return m_mousePosition - m_lastPosition; }

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

				ButtonEvent event(eData);
				m_buttonEventDispatcher.send(event);

				if ((eData.action == Action::Pressed) || (eData.action == Action::Released) || (eData.action == Action::Bumped)) {
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
