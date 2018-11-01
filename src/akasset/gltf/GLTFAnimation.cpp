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

#include <akasset/gltf/GLTFAnimation.hpp>
#include <akasset/gltf/Types.hpp>
#include <akasset/gltf/Util.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/debug/Log.hpp>
#include <akmath/Quaternion.hpp>
#include <akmath/Vector.hpp>
#include <glm/gtc/quaternion.hpp>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace aka::gltf;

aka::Animation aka::gltf::proccessGLTFAnimation(const Asset& asset, const Animation& animation) {
	aka::Animation result;
	std::unordered_map<gltfID, uint64> boneRemap;
	for(auto& channel : animation.channels) {
		if ((channel.samplerID < 0) || (channel.target.nodeID < 0)) continue;
		auto& sampler = animation.samplers[channel.samplerID];
		if ((sampler.input < 0) || (sampler.output < 0)) continue;

		auto remapResult = boneRemap.emplace(channel.target.nodeID, result.boneNames.size());
		if (remapResult.second) result.boneNames.push_back(asset.nodes[channel.target.nodeID].name);
		auto nodeIndex = remapResult.first->second;

		auto timeData = extractAccessorData<fpSingle>(asset, sampler.input);
		switch(channel.target.animationType) {
			case AnimationType::Translation: {
				auto transformData = extractAccessorData<akm::Vec3>(asset, sampler.output);
				akl::Logger("GLTFAnim").test_warn(transformData.size() == timeData.size(), "Time/transform data size mismatch, animation may not play correctly.");
				for(akSize i = 0; i < timeData.size(); i++) result.positionChannel[timeData[i]].push_back({nodeIndex, sampler.interpolation, transformData[i]});
			} break;

			case AnimationType::Rotation: {
				auto transformData = extractAccessorData<akm::Vec4>(asset, sampler.output);
				akl::Logger("GLTFAnim").test_warn(transformData.size() == timeData.size(), "Time/transform data size mismatch, animation may not play correctly.");
				for(akSize i = 0; i < timeData.size(); i++) result.rotationChannel[timeData[i]].push_back({nodeIndex, sampler.interpolation, akm::Quat(transformData[i].w,transformData[i].x,transformData[i].y,transformData[i].z)});
			} break;

			case AnimationType::Scale: {
				auto transformData = extractAccessorData<akm::Vec3>(asset, sampler.output);
				akl::Logger("GLTFAnim").test_warn(transformData.size() == timeData.size(), "Time/transform data size mismatch, animation may not play correctly.");
				for(akSize i = 0; i < timeData.size(); i++) result.scaleChannel[timeData[i]].push_back({nodeIndex, sampler.interpolation, transformData[i]});
			} break;

			case AnimationType::Weights: throw std::logic_error("Animated weights not supported");
		}
	}

	return result;
}


