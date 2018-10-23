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

#ifndef AK_ASSETS_GLTF_SCENE_HPP_
#define AK_ASSETS_GLTF_SCENE_HPP_

#include <AkAsset/gltf/Types.hpp>
#include <AkEngine/data/PValue.hpp>
#include <deque>
#include <string>
#include <vector>

namespace aka {
	namespace gltf {

		struct Scene final {
			std::string name;
			std::vector<gltfID> nodeIDs;
		};

		inline Scene extractScene(const akd::PValue& val) {
			std::vector<gltfID> nodeIDs;
			if (val.exists("nodes")) {
				auto& arr = val["nodes"].getArr();
				nodeIDs.reserve(arr.size());
				for(auto& node : arr) nodeIDs.push_back(node.as<gltfID>());
			}
			return Scene{val.atOrDef("name").getStrOrDef(""), nodeIDs};
		}

	}
}





#endif /* AKRES_GLTF_SCENE_HPP_ */
