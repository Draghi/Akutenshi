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

#ifndef AK_ASSETS_GLTF_TEXTURE_HPP_
#define AK_ASSETS_GLTF_TEXTURE_HPP_

#include <string>

#include <ak/assets/gltf/Types.hpp>
#include <ak/data/PValue.hpp>

namespace akas {
	namespace gltf {

		struct Texture final {
			std::string name;
			gltfID samplerID;
			gltfID sourceID;
		};

		inline Texture extractTexture(const akd::PValue& textureData) {
			return Texture{
				textureData.atOrDef("name").asStrOrDef(""),
				textureData.atOrDef("sampler").asOrDef<gltfID>(-1),
				textureData["source"].as<gltfID>(),
			};
		}

	}
}



#endif /* AKRES_GLTF_TEXTURE_HPP_ */
