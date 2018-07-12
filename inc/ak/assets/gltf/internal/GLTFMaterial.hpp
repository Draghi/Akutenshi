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

#ifndef AKRES_GLTFMATERIAL_HPP_
#define AKRES_GLTFMATERIAL_HPP_

#include <ak/assets/Convert.hpp>
#include <vector>

namespace akas {
	namespace gltf {
		struct Asset;
		struct Material;
	} /* namespace gltf */
} /* namespace akas */

namespace akas {
	namespace gltf {
		akas::Material proccessGLTFMaterial(const Asset& asset, const Material& material, const std::vector<akas::ConversionInfo>& textureAssetIDs);
	}
}



#endif /* AKRES_GLTFMATERIAL_HPP_ */
