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

#ifndef AK_ASSETS_ASSET_HPP_
#define AK_ASSETS_ASSET_HPP_

#include <ak/data/Json.hpp>
#include <ak/data/PValue.hpp>
#include <ak/data/SUID.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace akas { struct AssetInfo; }

namespace akd {
	void serialize(akd::PValue& dst, const akas::AssetInfo& src);
	bool deserialize(akas::AssetInfo& dst, const akd::PValue& src);
}

namespace akas {

	enum class AssetType : uint32 {
		Animation,
		Material,
		Mesh,
		Prefab,
		Scene,
		Sound,
		Texture,
		Image,
	};

	// @todo Add source field
	struct AssetInfo final {
		akd::SUID identifier;
		AssetType type;
		std::string displayName;
		akfs::Path source;
	};
}

namespace akd {

	inline void serialize(akd::PValue& dst, akas::AssetType src) {
		switch(src) {
			case akas::AssetType::Animation: dst.setStr("ANIMATION");      break;
			case akas::AssetType::Material:  dst.setStr("MATERIAL");       break;
			case akas::AssetType::Mesh:      dst.setStr("MESH");           break;
			case akas::AssetType::Prefab:    dst.setStr("PREFAB");         break;
			case akas::AssetType::Scene:     dst.setStr("SCENE");          break;
			case akas::AssetType::Sound:     dst.setStr("SOUND");          break;
			case akas::AssetType::Texture:   dst.setStr("TEXTURE");        break;
			case akas::AssetType::Image:     dst.setStr("IMAGE");          break;
			default: throw std::logic_error("Missing asset type mapping"); break;
		}
	}

	inline bool deserialize(akas::AssetType& dst, const akd::PValue& src) {
		try {
			const auto& srcStr = src.asStr();
			if (srcStr == "ANIMATION") { dst = akas::AssetType::Animation; return true; }
			if (srcStr == "MATERIAL" ) { dst = akas::AssetType::Material;  return true; }
			if (srcStr == "MESH"     ) { dst = akas::AssetType::Mesh;      return true; }
			if (srcStr == "PREFAB"   ) { dst = akas::AssetType::Prefab;    return true; }
			if (srcStr == "SCENE"    ) { dst = akas::AssetType::Scene;     return true; }
			if (srcStr == "SOUND"    ) { dst = akas::AssetType::Sound;     return true; }
			if (srcStr == "TEXTURE"  ) { dst = akas::AssetType::Texture;   return true; }
			if (srcStr == "IMAGE"    ) { dst = akas::AssetType::Image;     return true; }
		} catch(const std::logic_error& /*e*/) {}
		return false;
	}

	inline void serialize(akd::PValue& dst, const akas::AssetInfo& src) {
		serialize(dst["identifier"], src.identifier);
		serialize(dst["type"], src.type);
		dst["displayName"].setStr(src.displayName);
		dst["source"].setStr(src.source.str());
	}

	inline bool deserialize(akas::AssetInfo& dst, const akd::PValue& src) {
		try {
			dst = akas::AssetInfo{
				akd::deserialize<akd::SUID>(src["identifier"]),
				akd::deserialize<akas::AssetType>(src["type"]),
				src["displayName"].asStr(),
				akfs::Path(src["source"].asStr())
			};
			return true;
		} catch(const std::logic_error& /*e*/) {
			return false;
		}
	}

}

#endif /* AK_ASSETS_ASSET_HPP_ */
