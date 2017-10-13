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

#ifndef AK_INPUT_KEYBOARD_HPP_
#define AK_INPUT_KEYBOARD_HPP_

#include <ak/event/Event.hpp>
#include <ak/event/Util.hpp>
#include <ak/input/Keys.hpp>
#include <ak/input/Types.hpp>
#include <functional>

namespace ak {
	namespace input {

		class KeyboardActionEvent;

		class Keyboard {
			Keyboard(const Keyboard&) = delete;
			Keyboard& operator=(const Keyboard&) = delete;
			public:
				using KeyboardAction_f = std::function<void(KeyboardActionEvent&)>;

				Keyboard() = default;
				virtual ~Keyboard() = default;

				virtual bool isUp(Key key) const { return !isDown(key); }
				virtual bool isDown(Key key) const = 0;

				virtual bool wasPressed(Key key) const = 0;
				virtual bool wasReleased(Key key) const = 0;

				virtual ak::event::EventID subscribe(const KeyboardAction_f& callback) = 0;
				virtual ak::event::EventID subscribe(ak::event::Subscription& subscriber, const KeyboardAction_f& callback) = 0;
				virtual void unsubscribe(ak::event::EventID eventID) = 0;
				virtual void unsubscribe(ak::event::Subscription& subscriber) = 0;

				virtual void update() = 0;


		};

		class KeyboardActionEvent final : public ak::event::Event {
			AK_IMPLEMENT_EVENT("KeyboardActionEvent", true)
			private:
				Key m_key;
				Action m_action;
				const Keyboard& m_sender;
			public:
				KeyboardActionEvent(Key keyVal, Action actionVal, Keyboard& senderVal) : m_key(keyVal), m_action(actionVal), m_sender(senderVal) {}

				Key key() const { return m_key; }
				Action action() const { return m_action; }
				const Keyboard& sender() const { return m_sender; }

				bool  wasPressed(Key keyVal) const { return (m_key == keyVal) && (m_action == Action::Pressed);  }
				bool wasReleased(Key keyVal) const { return (m_key == keyVal) && (m_action == Action::Released); }
		};


	}
}

#endif
