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

#ifndef AK_INPUT_KEYBOARD_EVENTKEYBOARD_HPP_
#define AK_INPUT_KEYBOARD_EVENTKEYBOARD_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/event/Dispatcher.hpp>
#include <akengine/thread/DoubleBuffer.hpp>
#include <akinput/keyboard/Keyboard.hpp>
#include <akinput/keyboard/Keys.hpp>
#include <akinput/Types.hpp>
#include <array>
#include <utility>

namespace akin {
	class EventKeyboard final : public Keyboard {
		private:
			akt::DoubleBuffer<KeyEventData> m_keyEventBuffer;

			akev::Dispatcher<KeyEvent> m_keyEventDispatcher;
			akev::DispatcherProxy<KeyEvent> m_keyEventProxy = m_keyEventDispatcher;

			std::array<std::pair<Action, State>, static_cast<akSize>(Key::KEY_LAST_NORMAL)> m_keyStates;

			Action getKeyAction(Key key) const;
			State getKeyState(Key key) const;

		public:
			EventKeyboard();
			~EventKeyboard() override;

			const akev::DispatcherProxy<KeyEvent>& keyEvent() override;

			bool isDown(Key key) const override;

			bool wasPressed(Key key) const override;
			bool wasReleased(Key key) const override;
			bool wasBumped(Key key) const override;

			void update() override;

			void onKeyEvent(const KeyEventData& data);
	};
}




#endif /* AKINPUT_KEYBOARD_EVENTKEYBOARD_HPP_ */
