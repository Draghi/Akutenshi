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

#ifndef AK_ASSETS_GLTF_ASSET_HPP_
#define AK_ASSETS_GLTF_ASSET_HPP_

#include <akasset/gltf/Accessor.hpp>
#include <akasset/gltf/Animation.hpp>
#include <akasset/gltf/Buffer.hpp>
#include <akasset/gltf/BufferView.hpp>
#include <akasset/gltf/Image.hpp>
#include <akasset/gltf/Material.hpp>
#include <akasset/gltf/Mesh.hpp>
#include <akasset/gltf/Node.hpp>
#include <akasset/gltf/Sampler.hpp>
#include <akasset/gltf/Scene.hpp>
#include <akasset/gltf/Skin.hpp>
#include <akasset/gltf/Texture.hpp>
#include <akasset/gltf/Types.hpp>
#include <akengine/data/PValue.hpp>
#include <vector>

namespace aka {
	namespace gltf {

		struct Asset final {
			std::vector<Node> nodes;
			std::vector<Scene> scenes;
			std::vector<Mesh> meshes;
			std::vector<Skin> skins;
			std::vector<Material> materials;
			std::vector<Texture> textures;
			std::vector<Image> images;
			std::vector<Sampler> samplers;
			std::vector<Animation> animations;
			std::vector<Accessor> accessors;
			std::vector<BufferView> bufferViews;
			std::vector<Buffer> buffers;
			gltfID activeScene;
		};

		inline Asset extractAsset(const akd::PValue& val) {
			std::vector<Node> nodes; {
				auto& arrData = val["nodes"].getArr();
				for(auto& data : arrData) nodes.push_back(extractNode(data));
			}

			std::vector<Scene> scenes; {
				auto& arrData = val["scenes"].getArr();
				for(auto& data : arrData) scenes.push_back(extractScene(data));
			}

			std::vector<Mesh> meshes; {
				auto& arrData = val["meshes"].getArr();
				for(auto& data : arrData) meshes.push_back(extractMesh(data));
			}

			std::vector<Skin> skins; {
				auto& arrData = val["skins"].getArr();
				for(auto& data : arrData) skins.push_back(extractSkin(data));
			}

			std::vector<Material> materials; {
				auto& arrData = val["materials"].getArr();
				for(auto& data : arrData) materials.push_back(extractMaterial(data));
			}

			std::vector<Texture> textures; {
				auto& arrData = val["textures"].getArr();
				for(auto& data : arrData) textures.push_back(extractTexture(data));
			}

			std::vector<Image> images; {
				auto& arrData = val["images"].getArr();
				for(auto& data : arrData) images.push_back(extractImage(data));
			}

			std::vector<Sampler> samplers; {
				auto& arrData = val["samplers"].getArr();
				for(auto& data : arrData) samplers.push_back(extractSampler(data));
			}

			std::vector<Animation> animations; {
				auto& arrData = val["animations"].getArr();
				for(auto& data : arrData) animations.push_back(extractAnimation(data));
			}

			std::vector<Accessor> accessors; {
				auto& arrData = val["accessors"].getArr();
				for(auto& data : arrData) accessors.push_back(extractAccessor(data));
			}

			std::vector<BufferView> bufferViews; {
				auto& arrData = val["bufferViews"].getArr();
				for(auto& data : arrData) bufferViews.push_back(extractBufferView(data));
			}

			std::vector<Buffer> buffers; {
				auto& arrData = val["buffers"].getArr();
				for(auto& data : arrData) buffers.push_back(extractBuffer(data));
			}

			return Asset{nodes, scenes, meshes, skins, materials, textures, images, samplers, animations, accessors, bufferViews, buffers, val.atOrDef("scene").asOrDef(-1)};
		}
	}
}




#endif /* AKRES_GLTF_ASSET_HPP_ */
