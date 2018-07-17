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

#ifndef AK_ANIMATION_ANIMATION_HPP_
#define AK_ANIMATION_ANIMATION_HPP_

#include <map>
#include <stdexcept>
#include <vector>

#include <ak/animation/Fwd.hpp>
#include <ak/animation/Type.hpp>
#include <ak/math/Matrix.hpp>
#include <ak/math/Quaternion.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/Iterator.hpp>

namespace aka {

	inline Animation::Animation() = default;
	inline Animation::Animation(fpSingle ticksPerSecond, fpSingle duration, std::vector<AnimNode>& animNodes) : m_ticksPerSecond(ticksPerSecond), m_duration(duration), m_animNodes(animNodes) {}

	inline std::vector<PoseNode> Animation::calculatePose(fpSingle time) const {
		std::vector<PoseNode> result;
		result.reserve(m_animNodes.size());
		for(auto& animNode : m_animNodes) {
			result.push_back({
				animNode.nodeName,
				calculateFrame(animNode, time)
			});
		}
		return result;
	}

	inline std::vector<akm::Mat4> Animation::calculateIndexedPose(fpSingle time) const {
		std::vector<akm::Mat4> result;
		result.reserve(m_animNodes.size());
		for(auto& animNode : m_animNodes) result.push_back(calculateFrame(animNode, time));
		return result;
	}

	inline const std::vector<AnimNode>& Animation::nodes() const { return m_animNodes; }

	inline fpSingle Animation::duration() const {
		return (m_ticksPerSecond > 0) ? m_duration/m_ticksPerSecond : m_duration/25.0f;
	}

	inline fpSingle Animation::ticksPerSecond() const {
		return m_ticksPerSecond;
	}

	// ///////////// //
	// // Private // //
	// ///////////// //

	template<typename container_t> struct AnimHelper {
		using iter_t = typename container_t::const_iterator;

		iter_t startFrame;
		iter_t endFrame;
		fpSingle startTime;
		fpSingle endTime;

		AnimHelper(fpSingle animTime, fpSingle animDuration, const container_t& frames, TweenMode preAnim, TweenMode postAnim) {
			endFrame = frames.lower_bound(animTime);

			if (endFrame == frames.begin()) {

				// @hack Maybe? Handle starting at 0 animTime
				if (animTime >= endFrame->first) {
					startFrame = frames.begin(); startTime = startFrame->first;
					endFrame = frames.size() <= 1 ? frames.begin() : iter_t(frames.begin())++; endTime = endFrame->first;
					return;
				}

				// @todo are these correct?
				switch(preAnim) {
					case TweenMode::None: {
						startFrame = frames.end(); startTime = 0;
						endFrame = frames.end(); endTime = 0;
					} break;

					case TweenMode::Linear: throw std::runtime_error("aka::Animation: Linear extrapolation is not supported."); //@todo Implement

					case TweenMode::Nearest: {
						startFrame = endFrame;
						startTime = animTime; endTime = animTime;
					} break;

					case TweenMode::Repeat: throw std::logic_error("aka::Animation: Repeating with time less than 0, should be >0.");
				}

			} else if (endFrame == frames.end()) {

				switch(postAnim) {
					case TweenMode::None: {
						startFrame = frames.end(); endFrame = frames.end();
						startTime = 0; endTime = 0;
					} break;

					case TweenMode::Linear: throw std::runtime_error("aka::Animation: Linear extrapolation is not supported."); //@todo Implement

					case TweenMode::Nearest: {
						startFrame = frames.rend().base(); endFrame = startFrame;
						startTime = animDuration; endTime = animDuration;
					} break;

					case TweenMode::Repeat: {
						startFrame = frames.rend().base(); endFrame = frames.begin();
						startTime = startFrame->first; endTime = animDuration + endFrame->first;
					} break;
				}

			} else {
				startFrame = ak::prevIter(endFrame);
				startTime = startFrame->first;
				endTime = endFrame->first;
			}
		}
	};

	inline akm::Vec3 Animation::calculateFramePos(fpSingle animTime, const AnimNode& node) const {
		AnimHelper<std::map<fpSingle, akm::Vec3>> frameInfo(animTime, m_duration, node.posFrames, node.preAnim, node.postAnim);
		if (frameInfo.endFrame == node.posFrames.end()) throw std::runtime_error("aka::Animation: Default node extrapolation not supported"); //@todo Implement

		return (frameInfo.startFrame == frameInfo.endFrame)
			? frameInfo.startFrame->second
			: akm::mix(frameInfo.startFrame->second, frameInfo.endFrame->second, akm::normalizeRange(animTime, frameInfo.startTime, frameInfo.endTime));
	}

	inline akm::Vec3 Animation::calculateFrameScl(fpSingle animTime, const AnimNode& node) const {
		AnimHelper<std::map<fpSingle, akm::Vec3>> frameInfo(animTime, m_duration, node.sclFrames, node.preAnim, node.postAnim);
		if (frameInfo.endFrame == node.sclFrames.end()) throw std::runtime_error("aka::Animation: Default node extrapolation not supported"); //@todo Implement

		return (frameInfo.startFrame == frameInfo.endFrame)
			? frameInfo.startFrame->second
			: akm::mix(frameInfo.startFrame->second, frameInfo.endFrame->second, akm::normalizeRange(animTime, frameInfo.startTime, frameInfo.endTime));
	}

	inline akm::Quat Animation::calculateFrameRot(fpSingle animTime, const AnimNode& node) const {
		AnimHelper<std::map<fpSingle, akm::Quat>> frameInfo(animTime, m_duration, node.rotFrames, node.preAnim, node.postAnim);
		if (frameInfo.endFrame == node.rotFrames.end()) throw std::runtime_error("aka::Animation: Default node extrapolation not supported"); //@todo Implement

		return (frameInfo.startFrame == frameInfo.endFrame)
			? frameInfo.startFrame->second
			: akm::slerp(frameInfo.startFrame->second, frameInfo.endFrame->second, akm::normalizeRange(animTime, frameInfo.startTime, frameInfo.endTime));
	}

	inline akm::Mat4 Animation::calculateFrame(const AnimNode& node, fpSingle time) const {
		auto animTime = akm::mod((m_ticksPerSecond > 0) ? (time * m_ticksPerSecond) : (time * 25.0f), m_duration);
		if (animTime < 0) animTime += m_duration;
		akm::Vec3 framePos = calculateFramePos(animTime, node);
		akm::Vec3 frameScl = calculateFrameScl(animTime, node);
		akm::Quat frameRot = calculateFrameRot(animTime, node);
		return akm::translate(framePos) * akm::mat4_cast(frameRot) * akm::scale(frameScl);
	}

}

#endif
