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

#ifndef AK_ASSETS_GLTF_SKIN_HPP_
#define AK_ASSETS_GLTF_SKIN_HPP_

#include <akasset/gltf/Types.hpp>
#include <akengine/data/PValue.hpp>
#include <deque>
#include <string>
#include <vector>

namespace aka {
	namespace gltf {

		struct Skin final {
			std::string name;
			gltfID inverseBindMatricies;
			gltfID rootNodeID;
			std::vector<gltfID> jointNodeIDs;
		};

		inline Skin extractSkin(const akd::PValue& val) {
			std::vector<gltfID> joints;
			auto& arr = val["joints"].getArr();
			joints.reserve(arr.size());
			for(auto& jointID : arr) joints.push_back(jointID.as<gltfID>());
			return Skin{
				val.atOrDef("name").getStrOrDef(""),
				val.atOrDef("inverseBindMatrices").asOrDef<gltfID>(-1),
				val.atOrDef("skeleton").asOrDef<gltfID>(-1),
				joints
			};
		}
	}
}



#endif /* AKRES_GLTF_SKIN_HPP_ */
