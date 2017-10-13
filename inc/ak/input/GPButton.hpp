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

#ifndef AK_INPUT_GPBUTTON_HPP_
#define AK_INPUT_GPBUTTON_HPP_

#include <ak/PrimitiveTypes.hpp>

namespace ak {
	namespace input {

		enum class GPButton : uint8 {
			A,
			B,
			X,
			Y,

			Cross = A,
			Circle = B,
			Square = X,
			Triangle = Y,

			Start,
			Select,
			Menu,

			Up,
			Down,
			Left,
			Right,

			LBumper,
			LTrigger,
			LStick,

			L1 = LBumper,
			L2 = LTrigger,
			L3 = LStick,

			RBumper,
			RTrigger,
			RStick,

			R1 = RBumper,
			R2 = RTrigger,
			R3 = RStick,
		};

	}
}

#endif
