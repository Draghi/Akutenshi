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

#include <akengine/data/SmartEnum.hpp>
#include <akengine/data/SUID.hpp>
#include <akengine/filesystem/Path.hpp>
#include <string>

namespace aka {
	AK_SMART_ENUM_CLASS(AssetType,
		Animation,
		Material,
		Mesh,
		Prefab,
		Scene,
		ShaderStage,
		ShaderProgram,
		Sound,
		Texture,
		Image
	)

	struct AssetInfo final {
		akd::SUID identifier;
		AssetType type;
		std::string displayName;
		akfs::Path source;
	};
}

AK_SMART_ENUM_SERIALIZE(aka, AssetType)

AK_SMART_CLASS(aka::AssetInfo,
	FIELD, identifier,
	FIELD, type,
	FIELD, displayName,
	FIELD, source
)

#endif /* AK_ASSETS_ASSET_HPP_ */
