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

#ifndef AKCOMMON_DYNAMICBITSET_HPP_
#define AKCOMMON_DYNAMICBITSET_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <climits>
#include <vector>

namespace akc {

	class DynamicBitset final {
		private:
			akSize m_length;
			std::vector<uint8> m_data;

		public:
			DynamicBitset() {}

			bool at(akSize index) const {
				if (index >= m_length) throw std::out_of_range("Attempt to index out of range.");
				akSize subOffset = (index - (index/CHAR_BIT)*CHAR_BIT);
				return (m_data[index/CHAR_BIT] >> subOffset) & 0x01;
			}

			void set(akSize index, bool val) {
				akSize subOffset = (index - (index/CHAR_BIT)*CHAR_BIT);
				m_data[index/CHAR_BIT] = (m_data[index/CHAR_BIT] & ~(0x01 << subOffset)) | ((val ? 0x01 : 0x00) << subOffset);
			}

			void flip(akSize index) {
				akSize subOffset = (index - (index/CHAR_BIT)*CHAR_BIT);
				m_data[index/CHAR_BIT] ^= 0x01 << subOffset;
			}

			void setLength(akSize len) {
				if (len == 0) m_data.clear();
				else m_data.resize(1 + (len - 1)/CHAR_BIT, 0);
				m_length = len;
			}

			akSize length() const {
				return m_length;
			}
	};

}



#endif /* AKCOMMON_DYNAMICBITSET_HPP_ */
