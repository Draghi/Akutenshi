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

#ifndef AK_ASSETS_GLTF_NODE_HPP_
#define AK_ASSETS_GLTF_NODE_HPP_

#include <AkAsset/gltf/Types.hpp>
#include <AkEngine/data/PValue.hpp>
#include <AkMath/Matrix.hpp>
#include <AkMath/Serialize.hpp>
#include <AkMath/Types.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

namespace aka {
	namespace gltf {

		struct Node final {
			std::string name;

			std::vector<gltfID> childrenIDs;

			akm::Vec3 position;
			akm::Quat rotation;
			akm::Vec3 scale;

			gltfID meshID;
			gltfID skinID;
		};

		inline Node extractNode(const akd::PValue& val) {
			std::vector<gltfID> nodes;
			if (val.exists("children")) {
				auto& arr = val["children"].getArr();
				nodes.reserve(arr.size());
				for(auto& node : arr) nodes.push_back(node.as<gltfID>());
			}
			akm::Vec3 position; akm::Quat rotation; akm::Vec3 scale;
			if (val.exists("matrix")) {
				akm::Mat4 matrix;
				if (!akd::deserialize(matrix, val["matrix"])) std::cout << "Hello" << std::endl;
				akm::decompose(matrix, position, rotation, scale);
			} else {
				if (!akd::deserialize(position, val.atOrDef("translation"))) position = akm::Vec3(0,0,0);
				if (!akd::deserialize(rotation, val.atOrDef("rotation")))    rotation = akm::Quat(1,0,0,0);
				if (!akd::deserialize(scale,    val.atOrDef("scale")))          scale = akm::Vec3(0,0,0);
			}
			return Node{val.atOrDef("name").getStrOrDef(""), nodes, position, rotation, scale, val.atOrDef("mesh").asOrDef<gltfID>(-1), val.atOrDef("skin").asOrDef<gltfID>(-1)};
		}
	}
}

#endif /* AKRES_GLTF_NODE_HPP_ */
