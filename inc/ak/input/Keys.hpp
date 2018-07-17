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

#ifndef AK_INPUT_KEYS_HPP_
#define AK_INPUT_KEYS_HPP_

#include <ak/PrimitiveTypes.hpp>

namespace akin {
	enum class Key : uint16 {

			/* Ascii Keys */
			SPACE              = 32,

			APOSTROPHE         = 39,
			COMMA              = 44,
			MINUS              = 45,
			PERIOD             = 46,
			SLASH              = 47,

			N0                 = 48,
			N1                 = 49,
			N2                 = 50,
			N3                 = 51,
			N4                 = 52,
			N5                 = 53,
			N6                 = 54,
			N7                 = 55,
			N8                 = 56,
			N9                 = 57,

			SEMICOLON          = 59,
			EQUAL              = 61,

			A                  = 65,
			B                  = 66,
			C                  = 67,
			D                  = 68,
			E                  = 69,
			F                  = 70,
			G                  = 71,
			H                  = 72,
			I                  = 73,
			J                  = 74,
			K                  = 75,
			L                  = 76,
			M                  = 77,
			N                  = 78,
			O                  = 79,
			P                  = 80,
			Q                  = 81,
			R                  = 82,
			S                  = 83,
			T                  = 84,
			U                  = 85,
			V                  = 86,
			W                  = 87,
			X                  = 88,
			Y                  = 89,
			Z                  = 90,

			LBRACKET           = 91,
			BACKSLASH          = 92,
			RBRACKET           = 93,
			GRAVE_ACCENT       = 96,

			WORLD_1            = 161,
			WORLD_2            = 162,

			/* Non-ascii keys */
			ESCAPE             = 256,

			ENTER              = 257,
			TAB                = 258,
			BACKSPACE          = 259,
			INSERT             = 260,
			DELETE             = 261,

			RIGHT              = 262,
			LEFT               = 263,
			DOWN               = 264,
			UP                 = 265,

			PAGE_UP            = 266,
			PAGE_DOWN          = 267,
			HOME               = 268,
			END                = 269,

			CAPS_LOCK          = 280,
			SCROLL_LOCK        = 281,
			NUM_LOCK           = 282,
			PRINT_SCREEN       = 283,

			PAUSE              = 284,

			F1                 = 290,
			F2                 = 291,
			F3                 = 292,
			F4                 = 293,
			F5                 = 294,
			F6                 = 295,
			F7                 = 296,
			F8                 = 297,
			F9                 = 298,
			F10                = 299,
			F11                = 300,
			F12                = 301,
			F13                = 302,
			F14                = 303,
			F15                = 304,
			F16                = 305,
			F17                = 306,
			F18                = 307,
			F19                = 308,
			F20                = 309,
			F21                = 310,
			F22                = 311,
			F23                = 312,
			F24                = 313,
			F25                = 314,

			KP_0               = 320,
			KP_1               = 321,
			KP_2               = 322,
			KP_3               = 323,
			KP_4               = 324,
			KP_5               = 325,
			KP_6               = 326,
			KP_7               = 327,
			KP_8               = 328,
			KP_9               = 329,
			KP_DECIMAL         = 330,
			KP_DIVIDE          = 331,
			KP_MULTIPLY        = 332,
			KP_SUBTRACT        = 333,
			KP_ADD             = 334,
			KP_ENTER           = 335,
			KP_EQUAL           = 336,

			LSHIFT             = 340,
			LCONTROL           = 341,
			LALT               = 342,
			LSUPER             = 343,

			RSHIFT             = 344,
			RCONTROL           = 345,
			RALT               = 346,
			RSUPER             = 347,

			MENU               = 348,

			/* Special keys */
			KEY_LAST_NORMAL = MENU,
	};

}

#endif
