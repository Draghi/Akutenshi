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

#ifndef AK_ASSETS_MATERIAL_HPP_
#define AK_ASSETS_MATERIAL_HPP_

#include <ak/animation/Fwd.hpp>
#include <ak/animation/Serialize.hpp>
#include <ak/animation/Type.hpp>
#include <ak/assets/Asset.hpp>
#include <ak/assets/Mesh.hpp>
#include <ak/assets/Skin.hpp>
#include <ak/data/Image.hpp>
#include <ak/data/PValue.hpp>
#include <ak/data/SUID.hpp>
#include <ak/math/Serialize.hpp>
#include <ak/math/SphericalCoord.hpp>
#include <ak/math/Transform.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/gl/Textures.hpp>
#include <ak/window/WindowOptions.hpp>
#include <stdexcept>
#include <string>
#include <optional>
#include <utility>

namespace akas {

	enum class AlphaMode : uint8 {
		Opaque = 0,
		Mask   = 1,
		Blend  = 2
	};

	struct Sampler final {
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

		std::optional<std::pair<akd::SUID, Sampler>> baseTexture;
		std::optional<std::pair<akd::SUID, Sampler>> metallicRoughnessTexture;
		std::optional<std::pair<akd::SUID, Sampler>> normalTexture;
		std::optional<std::pair<akd::SUID, Sampler>> occlusionTexture;
		std::optional<std::pair<akd::SUID, Sampler>> emissiveTexture;

		AlphaMode alphaMode;
		fpSingle alphaCutoff;
		bool doubleSided;
	};

}


namespace akd {

	inline void serialize(akd::PValue& dst, const akas::Sampler& src) {
		switch(src.minFilter) {
			case akr::gl::FilterType::Nearest: dst["minFilter"].setStr("NEAREST"); break;
			case akr::gl::FilterType::Linear:  dst["minFilter"].setStr("LINEAR");  break;
			default: throw std::logic_error("Unsupported filter type.");
		}
		switch(src.minMipFilter) {
			case akr::gl::MipFilterType::None:    dst["minMipFilter"].setStr("NONE");    break;
			case akr::gl::MipFilterType::Nearest: dst["minMipFilter"].setStr("NEAREST"); break;
			case akr::gl::MipFilterType::Linear:  dst["minMipFilter"].setStr("LINEAR");  break;
		}
		switch(src.magFilter) {
			case akr::gl::FilterType::Nearest: dst["magFilter"].setStr("NEAREST"); break;
			case akr::gl::FilterType::Linear:  dst["magFilter"].setStr("LINEAR");  break;
			default: throw std::logic_error("Unsupported filter type.");
		}
		switch(src.clampS) {
			case akr::gl::ClampType::Border: dst["clampS"].setStr("BORDER"); break;
			case akr::gl::ClampType::Edge:   dst["clampS"].setStr("EDGE");   break;
			case akr::gl::ClampType::Mirror: dst["clampS"].setStr("MIRROR"); break;
			case akr::gl::ClampType::Repeat: dst["clampS"].setStr("REPEAT"); break;
			default: throw std::logic_error("Unsupported clamp type.");
		}
		switch(src.clampT) {
			case akr::gl::ClampType::Border: dst["clampT"].setStr("BORDER"); break;
			case akr::gl::ClampType::Edge:   dst["clampT"].setStr("EDGE");   break;
			case akr::gl::ClampType::Mirror: dst["clampT"].setStr("MIRROR"); break;
			case akr::gl::ClampType::Repeat: dst["clampT"].setStr("REPEAT"); break;
			default: throw std::logic_error("Unsupported clamp type.");
		}
	}

	inline bool deserialize(akas::Sampler& dst, const akd::PValue& src) {
		try {
			akas::Sampler result;
			{
				auto& minFilterStr = src["minFilter"].asStr();
				     if (minFilterStr == "NEAREST") result.minFilter = akr::gl::FilterType::Nearest;
				else if (minFilterStr == "LINEAR")  result.minFilter = akr::gl::FilterType::Linear;
				else return false;
			}
			{
				auto& minMipFilterStr = src["minMipFilter"].asStr();
			         if (minMipFilterStr == "NONE")    result.minMipFilter = akr::gl::MipFilterType::None;
				else if (minMipFilterStr == "NEAREST") result.minMipFilter = akr::gl::MipFilterType::Nearest;
				else if (minMipFilterStr == "LINEAR")  result.minMipFilter = akr::gl::MipFilterType::Linear;
				else return false;
			}
			{
				auto& magFilterStr = src["magFilter"].asStr();
				     if (magFilterStr == "NEAREST") result.magFilter = akr::gl::FilterType::Nearest;
				else if (magFilterStr == "LINEAR")  result.magFilter = akr::gl::FilterType::Linear;
				else return false;
			}
			{
				auto& clampSStr = src["clampS"].asStr();
					 if (clampSStr == "BORDER") result.clampS = akr::gl::ClampType::Border;
				else if (clampSStr == "EDGE")   result.clampS = akr::gl::ClampType::Edge;
				else if (clampSStr == "MIRROR") result.clampS = akr::gl::ClampType::Mirror;
				else if (clampSStr == "REPEAT") result.clampS = akr::gl::ClampType::Repeat;
				else return false;
			}
			{
				auto& clampTStr = src["clampT"].asStr();
				     if (clampTStr == "BORDER") result.clampT = akr::gl::ClampType::Border;
				else if (clampTStr == "EDGE")   result.clampT = akr::gl::ClampType::Edge;
				else if (clampTStr == "MIRROR") result.clampT = akr::gl::ClampType::Mirror;
				else if (clampTStr == "REPEAT") result.clampT = akr::gl::ClampType::Repeat;
				else return false;
			}
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

		if (src.baseTexture) {
			serialize(dst["baseTexture"]["id"],      src.baseTexture->first);
			serialize(dst["baseTexture"]["sampler"], src.baseTexture->second);
		}

		if (src.metallicRoughnessTexture) {
			serialize(dst["metallicRoughnessTexture"]["id"],      src.metallicRoughnessTexture->first);
			serialize(dst["metallicRoughnessTexture"]["sampler"], src.metallicRoughnessTexture->second);
		}

		if (src.normalTexture) {
			serialize(dst["normalTexture"]["id"],      src.normalTexture->first);
			serialize(dst["normalTexture"]["sampler"], src.normalTexture->second);
		}

		if (src.occlusionTexture) {
			serialize(dst["occlusionTexture"]["id"],      src.occlusionTexture->first);
			serialize(dst["occlusionTexture"]["sampler"], src.occlusionTexture->second);
		}

		if (src.emissiveTexture) {
			serialize(dst["emissiveTexture"]["id"],      src.emissiveTexture->first);
			serialize(dst["emissiveTexture"]["sampler"], src.emissiveTexture->second);
		}

		switch(src.alphaMode) {
			case akas::AlphaMode::Blend:  dst["alphaMode"].setStr("BLEND");  break;
			case akas::AlphaMode::Mask:   dst["alphaMode"].setStr("MASK");   break;
			case akas::AlphaMode::Opaque: dst["alphaMode"].setStr("OPAQUE"); break;
			default: throw std::logic_error("Unsupported alpha mode.");
		}

		dst["alphaCutoff"].setDec(src.alphaCutoff);
		dst["doubleSided"].setBool(src.doubleSided);
	}

	inline bool deserialize(akas::Material& /*dst*/, const akd::PValue& /*src*/) {
		try {
		} catch(const std::logic_error& /*e*/) {}
		return false;
	}

}

#endif /* AK_ASSETS_MATERIAL_HPP_ */
