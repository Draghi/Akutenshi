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

#include <akengine/debug/Log.hpp>
#include <akengine/event/Dispatcher.hpp>
#include <akinput/keyboard/EventKeyboard.hpp>
#include <akinput/keyboard/Keyboard.hpp>
#include <akinput/keyboard/Keys.hpp>
#include <akinput/Types.hpp>
#include <crtdefs.h>

using namespace akin;

Action EventKeyboard::getKeyAction(Key key) const { return m_keyStates[static_cast<size_t>(key)].first; }
State EventKeyboard::getKeyState(Key key) const { return m_keyStates[static_cast<size_t>(key)].second; }

EventKeyboard::EventKeyboard() = default;
EventKeyboard::~EventKeyboard() = default;

const akev::DispatcherProxy<KeyEvent>& EventKeyboard::keyEvent() { return m_keyEventProxy; }

bool EventKeyboard::isDown(Key key) const { return getKeyState(key) == State::Down; }

bool EventKeyboard::wasPressed(Key key) const { return (getKeyAction(key) == Action::Pressed) || wasBumped(key); }
bool EventKeyboard::wasReleased(Key key) const { return (getKeyAction(key) == Action::Released) || wasBumped(key); }
bool EventKeyboard::wasBumped(Key key) const { return getKeyAction(key) == Action::Bumped; }

void EventKeyboard::update() {
	m_keyEventBuffer.swap();

	for(auto iter = m_keyStates.begin(); iter != m_keyStates.end(); iter++) iter->first = Action::None;

	m_keyEventBuffer.iterate([this](size_t, KeyEventData& eventData) {
		if (eventData.key >= Key::KEY_LAST_NORMAL) {
			akl::Logger("EventKeyboard").warn("Unknown key pressed - ", static_cast<size_t>(eventData.key));
		} else if ((eventData.action == Action::Pressed) || (eventData.action == Action::Released) || (eventData.action == Action::Bumped)) {
			KeyEvent event(eventData);
			m_keyEventDispatcher.send(event);
			auto keyID = static_cast<size_t>(eventData.key);
			if (m_keyStates[keyID].first != Action::None) {
				m_keyStates[keyID] = std::make_pair(Action::Bumped, eventData.action == Action::Pressed ? State::Down : State::Up);
			} else {
				m_keyStates[keyID] = std::make_pair(eventData.action, eventData.action == Action::Pressed ? State::Down : State::Up);
			}
		}
	});
}

void EventKeyboard::onKeyEvent(const KeyEventData& data) { m_keyEventBuffer.push_back(data); }


