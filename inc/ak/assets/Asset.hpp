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

#ifndef AK_ASSETS_ASSET_HPP_
#define AK_ASSETS_ASSET_HPP_

#include <stdexcept>
#include <string>

#include <ak/data/PValue.hpp>
#include <ak/data/SUID.hpp>
#include <ak/filesystem/Path.hpp>
#include <ak/data/SmartEnum.hpp>

namespace akas { struct AssetInfo; }

namespace akd {
	void serialize(akd::PValue& dst, const akas::AssetInfo& src);
	bool deserialize(akas::AssetInfo& dst, const akd::PValue& src);
}

namespace akas {

	AK_SMART_ENUM_CLASS(AssetType,
		Animation,
		Material,
		Mesh,
		Prefab,
		Scene,
		Sound,
		Texture,
		Image
	)

	// @todo Add source field
	struct AssetInfo final {
		akd::SUID identifier;
		AssetType type;
		std::string displayName;
		akfs::Path source;
	};
}

AK_SMART_ENUM_SERIALIZE(akas, AssetType)

namespace akd {

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
				src["displayName"].getStr(),
				akfs::Path(src["source"].getStr())
			};
			return true;
		} catch(const std::logic_error& /*e*/) {
			return false;
		}
	}

}

#endif /* AK_ASSETS_ASSET_HPP_ */
