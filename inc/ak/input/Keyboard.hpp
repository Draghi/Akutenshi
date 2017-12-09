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

#include <ak/event/Dispatcher.hpp>
#include <ak/event/Event.hpp>
#include <ak/input/Keys.hpp>
#include <ak/input/Types.hpp>

namespace ak {
	namespace input {

		class Keyboard;

		struct KeyEventData {
			Key key;
			Action action;
			const Keyboard* sender;

			bool  wasPressed(Key keyVal) const { return (key == keyVal) && ((action == Action::Pressed) || (action == Action::Bumped));  }
			bool wasReleased(Key keyVal) const { return (key == keyVal) && ((action == Action::Released) || (action == Action::Bumped)); }
			bool   wasBumped(Key keyVal) const { return (key == keyVal) && (action == Action::Bumped); }
		};
		AK_DEFINE_EVENT(KeyEvent, const KeyEventData, true);

		class Keyboard {
			Keyboard(const Keyboard&) = delete;
			Keyboard& operator=(const Keyboard&) = delete;
			public:
				Keyboard() = default;
				virtual ~Keyboard() = default;

				virtual const ak::event::DispatcherProxy<KeyEvent>& keyEvent() = 0;

				virtual bool isUp(Key key) const { return !isDown(key); }
				virtual bool isDown(Key key) const = 0;

				virtual bool wasPressed(Key key) const = 0;
				virtual bool wasReleased(Key key) const = 0;
				virtual bool wasBumped(Key key) const = 0;

				virtual void update() = 0;
		};
	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_INPUT_ALIAS_DISABLE))
namespace akin = ak::input;
#endif

#endif
