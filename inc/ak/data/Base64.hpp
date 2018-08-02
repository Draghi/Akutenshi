/*
   base64.cpp and base64.h

   base64 encoding and decoding with C++.

   Version: 1.01.00

   Copyright (C) 2004-2017 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

/**
 * This file has been altered from the original source.
 * The listed changes bellow are not exhaustive:
 * - Made header-only
 * - Placed in namespace
 * - Names and formatting changed to better suit engine
 * - Changed some data types to better suit engine
 * - Made use of some C++ features like function-local statics
 * - Changed std::string to std::vector<uint8> for decoding
 */

#ifndef AK_DATA_BASE64_HPP_
#define AK_DATA_BASE64_HPP_

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

#include <ak/PrimitiveTypes.hpp>
#include <ak/util/String.hpp>

namespace akd {
	namespace base64 {
		inline bool isValid(uint8 c) {
			return (std::isalnum(c) || (c == '+') || (c == '/'));
		}

		inline std::string encode(const uint8* data, akSize size) {
			static constexpr std::string_view base64_chars = AK_STRING_VIEW("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

			std::string result;
			akSize i = 0, j = 0;
			uint8 arrChar3[3], arrChar4[4];

			while (size--) {
				arrChar3[i++] = *(data++);
				if (i != 3)  continue;

				arrChar4[0] = ((arrChar3[0] & 0xfc) >> 2);
				arrChar4[1] = ((arrChar3[0] & 0x03) << 4) + ((arrChar3[1] & 0xf0) >> 4);
				arrChar4[2] = ((arrChar3[1] & 0x0f) << 2) + ((arrChar3[2] & 0xc0) >> 6);
				arrChar4[3] = ((arrChar3[2] & 0x3f));

				for(i = 0; i < 4; i++) result += base64_chars[arrChar4[i]];
				i = 0;
			}

			if (i) {
				for(j = i; j < 3; j++) arrChar3[j] = '\0';

				arrChar4[0] = ((arrChar3[0] & 0xfc) >> 2);
				arrChar4[1] = ((arrChar3[0] & 0x03) << 4) + ((arrChar3[1] & 0xf0) >> 4);
				arrChar4[2] = ((arrChar3[1] & 0x0f) << 2) + ((arrChar3[2] & 0xc0) >> 6);

				for (j = 0; j < i + 1; j++) result += base64_chars[arrChar4[j]];
				while(i++ < 3) result += '=';
			}

			return result;

		}

		inline std::vector<uint8> decode(const std::string& data) {
			static constexpr std::string_view base64_chars = AK_STRING_VIEW("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

			akSize size = static_cast<akSize>(data.size());
			akSize i = 0, j = 0, index = 0;
			uint8 arrChar4[4], arrChar3[3];
			std::vector<uint8> result;

			while (size-- && (data[index] != '=') && isValid(data[index])) {
				arrChar4[i++] = data[index]; index++;
				if (i != 4) continue;

				for (i = 0; i <4; i++) arrChar4[i] = base64_chars.find(arrChar4[i]);

				arrChar3[0] = ((arrChar4[0]      ) << 2) + ((arrChar4[1] & 0x30) >> 4);
				arrChar3[1] = ((arrChar4[1] & 0xf) << 4) + ((arrChar4[2] & 0x3c) >> 2);
				arrChar3[2] = ((arrChar4[2] & 0x3) << 6) + ((arrChar4[3]));

				for (i = 0; i < 3; i++) result.push_back(arrChar3[i]);
				i = 0;
			}

			if (i) {
				for (j = 0; j < i; j++) arrChar4[j] = base64_chars.find(arrChar4[j]);

				arrChar3[0] = ((arrChar4[0]      ) << 2) + ((arrChar4[1] & 0x30) >> 4);
				arrChar3[1] = ((arrChar4[1] & 0xf) << 4) + ((arrChar4[2] & 0x3c) >> 2);

				for (j = 0; j < i - 1; j++) result.push_back(arrChar3[j]);
			}

			return result;
		}
	}
}

#endif /* AK_DATA_BASE64_HPP_ */
