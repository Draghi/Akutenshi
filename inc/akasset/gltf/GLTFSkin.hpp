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

#ifndef AK_ASSETS_GLTF_GLTFSKIN_HPP_
#define AK_ASSETS_GLTF_GLTFSKIN_HPP_

#include <akasset/gltf/Asset.hpp>
#include <akasset/gltf/Skin.hpp>
#include <akasset/gltf/Types.hpp>
#include <akasset/Skin.hpp>
#include <unordered_map>

namespace aka {
	namespace gltf {
		aka::Skin processGLTFSkin(const gltf::Asset& asset, const gltf::Skin& skin, gltf::gltfID nodeID, const std::unordered_map<gltf::gltfID, gltf::gltfID>& parentMap);
	}
}

#endif /* AKRES_GLTFSKIN_HPP_ */
