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

#ifndef AK_ASSETS_GLTF_MESH_HPP_
#define AK_ASSETS_GLTF_MESH_HPP_

#include <deque>
#include <string>
#include <vector>

#include <ak/assets/gltf/Types.hpp>
#include <ak/data/PValue.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/String.hpp>

namespace akas {
	namespace gltf {

		enum class PrimitiveMode : uint8 {
			Points        = 0,
			Lines         = 1,
			LineLoop      = 2,
			LineStrip     = 3,
			Triangles     = 4,
			TriangleStrip = 5,
			TriangleFan   = 6
		};

		struct Primitive final {
			gltfID materialID;
			PrimitiveMode mode;
			gltfID indiciesID;
			gltfID positionID;
			gltfID normalID;
			gltfID tangentID;
			std::vector<gltfID> texCoordIDs;
			std::vector<gltfID> colourIDs;
			std::vector<gltfID> jointIDs;
			std::vector<gltfID> weightIDs;
		};

		struct Mesh final {
			std::string name;
			std::vector<Primitive> primitives;
		};

		inline Primitive extractPrimitive(const akd::PValue& val) {
			auto collectMultiAttrib = [](const std::string& name, const akd::PValue& attribs) {
				std::vector<gltfID> result;
				for(uint i = 0; true; i++) {
					auto property = ak::buildString(name, i);
					if (!attribs.exists(property)) break;
					result.push_back(attribs.at(property).as<gltfID>());
				}
				return result;
			};

			std::vector<gltfID> texCoordIDs = collectMultiAttrib("TEXCOORD_", val["attributes"]);
			std::vector<gltfID>   colourIDs = collectMultiAttrib("COLOR_",    val["attributes"]);
			std::vector<gltfID>    jointIDs = collectMultiAttrib("JOINTS_",   val["attributes"]);
			std::vector<gltfID>   weightIDs = collectMultiAttrib("WEIGHTS_",  val["attributes"]);

			return Primitive{
				val.atOrDef("material").asOrDef<gltfID>(-1),
				static_cast<PrimitiveMode>(val.atOrDef("mode").asOrDef<uint32>(4)),
				val.atOrDef("indicies").asOrDef<gltfID>(-1),
				val.at("attributes").at("POSITION").as<gltfID>(),
				val.at("attributes").at("NORMAL"  ).as<gltfID>(),
				val.at("attributes").atOrDef("TANGENT").asOrDef<gltfID>(-1),
				texCoordIDs, colourIDs, jointIDs, weightIDs
			};
		}

		inline Mesh extractMesh(const akd::PValue& val) {
			std::vector<Primitive> primitives;
			auto& arr = val["primitives"].getArr();
			primitives.reserve(arr.size());
			for(auto& primitive : arr) primitives.push_back(extractPrimitive(primitive));
			return Mesh{val.atOrDef("name").getStrOrDef(""), primitives};
		}

	}
}



#endif /* AKRES_GLTF_MESH_HPP_ */
