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

#ifndef AK_SOUND_BUFFER_HPP_
#define AK_SOUND_BUFFER_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/Backend.hpp>
#include <cstring>
#include <vector>

namespace aks {

	class Buffer final {
		private:
			std::vector<uint8> m_data;
			akSize m_sampleRate;
			Format m_format;
			std::vector<Channel> m_channelMap;

		public:
			Buffer(const void* source, akSize frameCount, akSize sampleRate, Format format, const std::vector<Channel>& channelMap) {
				m_sampleRate = sampleRate; m_format = format; m_channelMap = channelMap;
				m_data.resize(frameCount*frameSize());
				std::memcpy(m_data.data(), source, m_data.size());
			}

			const void* data() const { return m_data.data(); }
			akSize size() const { return m_data.size(); }

			akSize sampleRate() const { return m_sampleRate; }
			Format format()     const { return m_format; }
			const std::vector<Channel>& channelMap() const { return m_channelMap; }

			akSize frameSize() const { return m_channelMap.size()*getFormatElementSize(m_format); }
			akSize frameCount() const { return m_data.size()/frameSize(); }
			fpSingle duration() const { return m_data.size()/static_cast<fpSingle>(frameSize()*m_sampleRate); }
	};

}

#endif /* AK_SOUND_BUFFER_HPP_ */
