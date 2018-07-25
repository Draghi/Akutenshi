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

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <ak/data/PValue.hpp>
#include <ak/data/SUID.hpp>
#include <ak/math/Serialize.hpp>
#include <ak/math/Types.hpp>
#include <ak/Macros.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/gl/Textures.hpp>

namespace akas {

	AK_DEFINE_SMART_TENUM_CLASS_KV(AlphaMode, uint8,
		Opaque, 0,
		Mask,   1,
		Blend,  2
	)

	struct Sampler final {
		akd::SUID imgAssetID;

		akr::gl::FilterType    minFilter;
		akr::gl::MipFilterType minMipFilter;
		akr::gl::FilterType    magFilter;

		akr::gl::ClampType clampS;
		akr::gl::ClampType clampT;
	};

	struct Material final {
		akm::Vec3 baseColour;
		fpSingle metallicFactor;
		fpSingle roughnessFactor;
		akm::Vec3 emmisiveFactor;

		std::optional<Sampler> baseTexture;
		std::optional<Sampler> metalRoughTexture;
		std::optional<Sampler> normalTexture;
		std::optional<Sampler> occlusionTexture;
		std::optional<Sampler> emissiveTexture;

		AlphaMode alphaMode;
		fpSingle alphaCutoff;
		bool doubleSided;
	};

}

AK_DEFINE_SMART_ENUM_SERIALIZE(akas, AlphaMode)

namespace akd {

	inline void serialize(akd::PValue& dst, const akas::Sampler& src) {
		serialize(dst["imgAssetID"],   src.imgAssetID  );
		serialize(dst["minFilter"],    src.minFilter   );
		serialize(dst["minMipFilter"], src.minMipFilter);
		serialize(dst["magFilter"],    src.magFilter   );
		serialize(dst["clampS"],       src.clampS      );
		serialize(dst["clampT"],       src.clampT      );
	}

	inline bool deserialize(akas::Sampler& dst, const akd::PValue& src) {
		try {
			akas::Sampler result;

			if (!deserialize(result.imgAssetID  , src["imgAssetID"]  )) return false;
			if (!deserialize(result.minFilter   , src["minFilter"]   )) return false;
			if (!deserialize(result.minMipFilter, src["minMipFilter"])) return false;
			if (!deserialize(result.magFilter   , src["magFilter"]   )) return false;
			if (!deserialize(result.clampS      , src["clampS"]      )) return false;
			if (!deserialize(result.clampT      , src["clampT"]      )) return false;

			dst = result;

			return true;
		} catch(const std::logic_error& /*e*/) {
			return false;
		}
	}

	inline void serialize(akd::PValue& dst, const akas::Material& src) {
		serialize(dst["baseColour"], src.baseColour);
		dst["metallicFactor"].setDec(src.metallicFactor);
		dst["roughnessFactor"].setDec(src.roughnessFactor);
		serialize(dst["emmisiveFactor"], src.emmisiveFactor);

		if (src.baseTexture      ) serialize(dst["baseTexture"],       *src.baseTexture);
		if (src.metalRoughTexture) serialize(dst["metalRoughTexture"], *src.metalRoughTexture);
		if (src.normalTexture    ) serialize(dst["normalTexture"],     *src.normalTexture);
		if (src.occlusionTexture ) serialize(dst["occlusionTexture"],  *src.occlusionTexture);
		if (src.emissiveTexture  ) serialize(dst["emissiveTexture"],   *src.emissiveTexture);

		serialize(dst["alphaMode"], src.alphaMode);
		dst["alphaCutoff"].setDec(src.alphaCutoff);
		dst["doubleSided"].setBool(src.doubleSided);
	}

	inline bool deserialize(akas::Material& dst, const akd::PValue& src) {
		try {
			akas::Material result;

			deserialize(result.baseColour, src["baseColour"]);
			result.metallicFactor = src["metallicFactor"].as<fpSingle>();
			result.roughnessFactor = src["roughnessFactor"].as<fpSingle>();
			deserialize(result.emmisiveFactor, src["emmisiveFactor"]);

			if (src.exists("baseTexture"      )) result.baseTexture       = deserialize<akas::Sampler>(src["baseTexture"]      );
			if (src.exists("metalRoughTexture")) result.metalRoughTexture = deserialize<akas::Sampler>(src["metalRoughTexture"]);
			if (src.exists("normalTexture"    )) result.normalTexture     = deserialize<akas::Sampler>(src["normalTexture"]    );
			if (src.exists("occlusionTexture" )) result.occlusionTexture  = deserialize<akas::Sampler>(src["occlusionTexture"] );
			if (src.exists("emissiveTexture"  )) result.emissiveTexture   = deserialize<akas::Sampler>(src["emissiveTexture"]  );

			deserialize(result.alphaMode, src["alphaMode"]);
			result.alphaCutoff = src["alphaCutoff"].as<fpSingle>();
			result.doubleSided = src["doubleSided"].asBool();

			dst = result;
			return true;
		} catch(const std::logic_error& /*e*/) {
			return false;
		}
	}

}

#endif /* AK_ASSETS_MATERIAL_HPP_ */
