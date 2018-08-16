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

#ifndef AK_SOUND_SOURCE_HPP_
#define AK_SOUND_SOURCE_HPP_

#include <ak/math/Scalar.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/Backend.hpp>
#include <ak/sound/Buffer.hpp>
#include <cstring>
#include <memory>
#include <vector>

namespace aks {
	class Buffer;
}

namespace aks {

	class Source final {
		private:
			std::shared_ptr<Buffer> m_audioBuffer;

			bool m_isLooping;
			fpSingle m_volume;
			fpSingle m_playbackSpeed;

			fpDouble m_currentFrame;

			akSize copyRange(void* audioFrames, akSize startFrame, akSize frameCount) {
				akSize maxFrameCount = m_audioBuffer->frameCount(), frameSize = m_audioBuffer->frameSize();

				uint8* uAudioFrames = static_cast<uint8*>(audioFrames);
				const uint8* uBufferFrames = static_cast<const uint8*>(m_audioBuffer->data());

				akSize copiedFrames = 0;
				while(copiedFrames < frameCount) {
					akSize curFrameStart = (startFrame + copiedFrames) % maxFrameCount;
					akSize framesToCopy = (curFrameStart + (frameCount - copiedFrames)) >= maxFrameCount ? (maxFrameCount - curFrameStart) : frameCount - copiedFrames;
					std::memcpy(uAudioFrames + copiedFrames*frameSize, uBufferFrames + curFrameStart*frameSize, framesToCopy*frameSize);
					copiedFrames += framesToCopy;
				}

				return copiedFrames;
			}

		public: //protected:
			Source() : m_audioBuffer(), m_isLooping(false), m_volume(1.f), m_playbackSpeed(1.f) {}

			akSize fillBuffer(void* audioFrames, akSize frameCount, fpSingle sampleRate, Format format, ChannelMap channelMap) {
				bool formatMatches = m_audioBuffer->format() == format;
				bool channelMapMatches = m_audioBuffer->channelMap() == channelMap;
				bool skipResampling = m_audioBuffer->sampleRate() == sampleRate && (m_currentFrame == static_cast<akSize>(m_currentFrame)) && (m_playbackSpeed == 1.f);

				if (formatMatches && channelMapMatches && skipResampling) { // Perfect match, no conversion or interpolation required.
					akSize startFrame = static_cast<akSize>(m_currentFrame);
					auto copiedFrames = copyRange(audioFrames, startFrame, frameCount);
					akSize maxFrameCount = m_audioBuffer->frameCount();
					if (copiedFrames != frameCount) akl::Logger("").info("NEQ - ", copiedFrames, " - ", frameCount);

					m_currentFrame = (startFrame + copiedFrames) % maxFrameCount;
					return copiedFrames;
				} else {
					return 0;
				}
			}

		public:
			void playSound(const std::shared_ptr<Buffer>& audioBuffer, bool isLooping = false, fpSingle volume = 1.f, fpSingle playbackSpeed = 1.f) {
				m_audioBuffer = audioBuffer;
				m_isLooping = isLooping;
				m_volume = volume;
				m_playbackSpeed = playbackSpeed;
				m_currentFrame = 0.f;
			}

	};
}

#endif /* AK_SOUND_SOURCE_HPP_ */
