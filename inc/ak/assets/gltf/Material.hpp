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

#ifndef AK_ASSETS_GLTF_MATERIAL_HPP_
#define AK_ASSETS_GLTF_MATERIAL_HPP_

#include <stdexcept>
#include <string>

#include <ak/assets/gltf/Types.hpp>
#include <ak/data/PValue.hpp>
#include <ak/math/Serialize.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>

namespace akas {
	namespace gltf {

		struct TextureInfo {
			gltfID index;
			int32 texCoordSet;
		};

		enum class AlphaMode : uint8 {
			Opaque = 0,
			Mask   = 1,
			Blend  = 2
		};

		struct Material final {
			std::string name;

			akm::Vec4 baseFactor;
			fpSingle metallicFactor;
			fpSingle roughnessFactor;
			akm::Vec3 emissiveFactor;

			TextureInfo baseTexture;
			TextureInfo metallicRoughnessTexture;
			TextureInfo normalTexture;
			TextureInfo occlusionTexture;
			TextureInfo emissiveTexture;

			AlphaMode alphaMode;
			fpSingle alphaCutoff;
			bool doubleSided;
		};

		inline TextureInfo extractTextureInfo(const akd::PValue& val) {
			return TextureInfo{
				val.atOrDef("index").asOrDef<gltfID>(-1),
				val.atOrDef("texCoord").asOrDef<int32>(0)
			};
		}

		inline Material extractMaterial(const akd::PValue& val) {
			AlphaMode alphaMode;
			std::string modeStr = val.atOrDef("alphaMode").getStrOrDef("OPAQUE");
			     if (modeStr == "OPAQUE") alphaMode = AlphaMode::Opaque;
			else if (modeStr == "MASK")   alphaMode = AlphaMode::Mask;
			else if (modeStr == "BLEND")  alphaMode = AlphaMode::Blend;
			else throw std::runtime_error("Unsupported mode string for material");

			akm::Vec4 baseFactor{1,1,1,1}; akd::deserialize(baseFactor, val["pbrMetallicRoughness"].atOrDef("baseColorFactor"));
			akm::Vec3 emissiveFactor{0,0,0}; akd::deserialize(emissiveFactor, val.atOrDef("emissiveFactor"));

			return Material{
				val.atOrDef("name").getStrOrDef(""),

				baseFactor,
				val["pbrMetallicRoughness"].atOrDef("metallicFactor").asOrDef<fpSingle>(1.0f),
				val["pbrMetallicRoughness"].atOrDef("roughnessFactor").asOrDef<fpSingle>(1.0f),
				emissiveFactor,

				extractTextureInfo(val["pbrMetallicRoughness"].atOrDef("baseColorTexture")),
				extractTextureInfo(val["pbrMetallicRoughness"].atOrDef("metallicRoughnessTexture")),
				extractTextureInfo(val.atOrDef("normalTexture")),
				extractTextureInfo(val.atOrDef("occlusionTexture")),
				extractTextureInfo(val.atOrDef("emissiveTexture")),

				alphaMode,
				val.atOrDef("alphaCutoff").asOrDef<fpSingle>(0.5f),

				val.atOrDef("doubleSided").asOrDef<bool>(false)
			};
		}
	}
}



#endif /* AKRES_GLTF_MATERIAL_HPP_ */
