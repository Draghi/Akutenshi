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

#ifndef AK_ANIMATION_ANIMATION_HPP_
#define AK_ANIMATION_ANIMATION_HPP_

#include <ak/math/Matrix.hpp>
#include <ak/math/Quaternion.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <stddef.h>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

namespace aka {

	enum class LoopType {
		Stop,
		Reset,
		Repeat,
		Reverse
	};

	using BonePose = std::unordered_map<std::string, akm::Mat4>;

	class Animation {
		private:
			fpDouble m_frameRate;
			LoopType m_loopType;
			std::map<akSize, BonePose> m_keyFrames;

		public:
			Animation() : m_frameRate(60), m_loopType(LoopType::Repeat), m_keyFrames() {}

			Animation(fpDouble frameRate, LoopType loopType, const std::map<akSize, BonePose>& keyFrames) : m_frameRate(frameRate), m_loopType(loopType), m_keyFrames(keyFrames) {}
			Animation(fpDouble frameRate, LoopType loopType, std::map<akSize, BonePose>&& keyFrames) : m_frameRate(frameRate), m_loopType(loopType), m_keyFrames(keyFrames) {}

			Animation(const Animation& other) : m_frameRate(other.m_frameRate), m_loopType(other.m_loopType), m_keyFrames(other.m_keyFrames) {}
			Animation(Animation&& other) : m_frameRate(other.m_frameRate), m_loopType(other.m_loopType), m_keyFrames(std::move(other.m_keyFrames)) {}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
			BonePose resolveFrame(fpDouble frameTime) {
				fpSingle frame = frameTime*m_frameRate;

				std::map<akSize, BonePose>::iterator previousFrame = m_keyFrames.end();
				std::map<akSize, BonePose>::iterator currentFrame  = m_keyFrames.end();
				std::map<akSize, BonePose>::iterator nextFrame     = m_keyFrames.end();

				for(auto iter = m_keyFrames.begin(); iter != m_keyFrames.end(); iter++) {
					if (frame == iter->first) {
						currentFrame = iter;
						nextFrame = iter++;
						break;
					} else if (frame < iter->first) {
						currentFrame = previousFrame;
						nextFrame = iter;
						break;
					}
					previousFrame = iter;
				}

				if (currentFrame == m_keyFrames.end()) return m_keyFrames.begin()->second;
				if (nextFrame == m_keyFrames.end()) return currentFrame->second;

				fpSingle frameDelta = static_cast<fpSingle>((frame - currentFrame->first)/(nextFrame->first - currentFrame->first));

				std::set<std::string> bones;
				for(auto iter = currentFrame->second.begin(); iter != currentFrame->second.end(); iter++) bones.insert(iter->first);
				for(auto iter = nextFrame->second.begin();    iter != nextFrame->second.end();    iter++) bones.insert(iter->first);

				BonePose result;
				for(auto iter = bones.begin(); iter != bones.end(); iter++) {
					auto currentPose = currentFrame->second.find(*iter);
					auto nextPose = nextFrame->second.find(*iter);

					if (currentPose == currentFrame->second.end()) result.insert(std::make_pair(*iter, nextPose->second));
					if (nextPose == nextFrame->second.end()) result.insert(std::make_pair(*iter, currentPose->second));

					akm::Vec3 cPos = currentPose->second[3];
					akm::Quat cRot = currentPose->second;

					akm::Vec3 nPos = nextPose->second[3];
					akm::Quat nRot = nextPose->second;

					akm::Mat4 fMat = akm::mat4_cast(akm::slerp(cRot, nRot, frameDelta));
					fMat[3] = akm::Vec4(akm::mix(cPos, nPos, frameDelta), fMat[3][3]);

					result.insert(std::make_pair(*iter, fMat));
				}

				return result;
			}
#pragma clang diagnostic pop

			LoopType loopType() const { return m_loopType; }
			fpDouble frameRate() const { return m_frameRate; }
			const std::map<akSize, BonePose>& keyFrames() const { return m_keyFrames; }

			Animation& operator=(const Animation&  other) { m_frameRate = other.m_frameRate; m_loopType = other.m_loopType; m_keyFrames = other.m_keyFrames; return *this; }
			Animation& operator=(      Animation&& other) { m_frameRate = other.m_frameRate; m_loopType = other.m_loopType; m_keyFrames = std::move(other.m_keyFrames); return *this; }
	};

}

#endif
