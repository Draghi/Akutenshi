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

#ifndef AK_ASSETS_MATERIAL_HPP_
#define AK_ASSETS_MATERIAL_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/data/SmartEnum.hpp>
#include <akengine/data/SUID.hpp>
#include <akmath/Types.hpp>
#include <akrender/gl/Textures.hpp>
#include <optional>

namespace aka {

	AK_SMART_TENUM_CLASS_KV(AlphaMode, uint8,
		Opaque, 0,
		Mask,   1,
		Blend,  2
	)

	struct Sampler final {
		akd::SUID imgAssetID;

		akSize texCoordIndex;

		akr::gl::FilterType    minFilter;
		akr::gl::MipFilterType minMipFilter;
		akr::gl::FilterType    magFilter;

		akr::gl::ClampType clampS;
		akr::gl::ClampType clampT;
	};

	struct Material final {
		akm::Vec4 baseColour;
		fpSingle metallicFactor;
		fpSingle roughnessFactor;
		akm::Vec3 emmisiveFactor;

		std::optional<Sampler> baseTexture;
		std::optional<Sampler> metalRoughnessTexture;
		std::optional<Sampler> normalTexture;
		std::optional<Sampler> occlusionTexture;
		std::optional<Sampler> emissiveTexture;

		AlphaMode alphaMode;
		fpSingle alphaCutoff;
		bool doubleSided;
	};
}

#endif /* AK_ASSETS_MATERIAL_HPP_ */
