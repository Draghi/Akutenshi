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

#ifndef AK_ASSETS_GLTF_ANIMATION_HPP_
#define AK_ASSETS_GLTF_ANIMATION_HPP_

#include <AkAsset/Animation.hpp>
#include <AkAsset/gltf/Types.hpp>
#include <deque>
#include <stdexcept>

namespace aka {
	namespace gltf {

		struct AnimationSampler final {
			gltfID input; // seconds
			Interpolation interpolation;
			gltfID output; // transformation data
		};

		enum class AnimationType {
			Translation,
			Rotation,
			Scale,
			Weights
		};

		struct AnimationTarget final {
			gltfID nodeID;
			AnimationType animationType;
		};

		struct AnimationChannel final {
			gltfID samplerID;
			AnimationTarget target;
		};

		struct Animation final {
			std::string name;
			std::vector<AnimationChannel> channels;
			std::vector<AnimationSampler> samplers;
		};

		inline AnimationSampler extractAnimationSampler(const akd::PValue& animationSamplerData) {
			Interpolation interpolationType;
			std::string interpolationTypeStr = animationSamplerData.atOrDef("interpolation").getStrOrDef("LINEAR");


			if (interpolationTypeStr == "LINEAR") interpolationType = Interpolation::Linear;
			else if (interpolationTypeStr == "STEP") interpolationType = Interpolation::Step;
			else if (interpolationTypeStr == "CUBICSPLINE") interpolationType = Interpolation::Cubic;
			else throw std::runtime_error("Unsupported interpolation type.");

			return AnimationSampler{
				animationSamplerData["input"].as<gltfID>(),
				interpolationType,
				animationSamplerData["output"].as<gltfID>(),
			};
		}

		inline AnimationTarget extractAnimationTarget(const akd::PValue& animationTargetData) {
			AnimationType animationType;
			std::string animationTypeStr = animationTargetData["path"].getStr();

			if (animationTypeStr == "translation")   animationType = AnimationType::Translation;
			else if (animationTypeStr == "rotation") animationType = AnimationType::Rotation;
			else if (animationTypeStr == "scale")    animationType = AnimationType::Scale;
			else if (animationTypeStr == "weights")  animationType = AnimationType::Weights;
			else throw std::runtime_error("Unsupported animation path.");

			return AnimationTarget {
				animationTargetData["node"].as<gltfID>(),
				animationType
			};
		}

		inline AnimationChannel extractAnimationChannel(const akd::PValue& animationChannelData) {
			return AnimationChannel{
				animationChannelData["sampler"].as<gltfID>(),
				extractAnimationTarget(animationChannelData["target"])
			};
		}

		inline Animation extractAnimation(const akd::PValue& animationData) {
			std::vector<AnimationChannel> channels;
			auto& channelArr = animationData["channels"].getArr();
			channels.reserve(channelArr.size());
			for(auto& channel : channelArr) channels.push_back(extractAnimationChannel(channel));

			std::vector<AnimationSampler> samplers;
			auto& samplerArr = animationData["samplers"].getArr();
			samplers.reserve(samplerArr.size());
			for(auto& sampler : samplerArr) samplers.push_back(extractAnimationSampler(sampler));

			return Animation {
				animationData.atOrDef("name").getStrOrDef(""),
				channels,
				samplers
			};
		}

	}
}



#endif /* AKRES_GLTF_ANIMATION_HPP_ */
