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

#ifndef AK_ASSETS_GLTF_SAMPLER_HPP_
#define AK_ASSETS_GLTF_SAMPLER_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/data/PValue.hpp>
#include <string>

namespace aka {
	namespace gltf {

		enum class MinFilter : uint32 {
			Nearest = 9728,
			Linear = 9729,
			Nearest_Mipmap_Nearest = 9984,
			Linear_Mipmap_Nearest = 9985,
			Nearest_Mipmap_Linear = 9986,
			Linear_Mipmap_Linear = 9987
		};

		enum class MagFilter : uint32 {
			Nearest = 9728,
			Linear = 9729
		};

		enum class Wrap : uint32 {
			ClampToEdge = 33071,
			MirroredRepeat = 33648,
			Repeat = 10497
		};

		struct Sampler final {
			std::string name;
			MinFilter minFilter;
			MagFilter magFilter;
			Wrap wrapS;
			Wrap wrapT;
		};

		inline Sampler extractSampler(const akd::PValue& samplerData) {
			return Sampler{
				samplerData.atOrDef("name").getStrOrDef(""),
				static_cast<MinFilter>(samplerData.atOrDef("minFilter").asOrDef<uint32>(static_cast<uint32>(MinFilter::Linear_Mipmap_Linear))),
				static_cast<MagFilter>(samplerData.atOrDef("magFilter").asOrDef<uint32>(static_cast<uint32>(MagFilter::Linear))),
				static_cast<Wrap>(samplerData.atOrDef("wrapS").asOrDef<uint32>(static_cast<uint32>(Wrap::Repeat))),
				static_cast<Wrap>(samplerData.atOrDef("wrapT").asOrDef<uint32>(static_cast<uint32>(Wrap::Repeat)))
			};
		}

	}
}

#endif /* AKRES_GLTF_SAMPLER_HPP_ */
