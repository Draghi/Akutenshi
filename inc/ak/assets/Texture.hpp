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

#ifndef AK_ASSETS_TEXTURE_HPP_
#define AK_ASSETS_TEXTURE_HPP_

#include <stdexcept>
#include <vector>

#include <ak/animation/Fwd.hpp>
#include <ak/animation/Serialize.hpp>
#include <ak/animation/Type.hpp>
#include <ak/assets/Asset.hpp>
#include <ak/assets/Image.hpp>
#include <ak/assets/Material.hpp>
#include <ak/assets/Mesh.hpp>
#include <ak/assets/Skin.hpp>
#include <ak/data/Image.hpp>
#include <ak/data/PValue.hpp>
#include <ak/data/SUID.hpp>
#include <ak/math/Serialize.hpp>
#include <ak/math/SphericalCoord.hpp>
#include <ak/math/Transform.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/gl/Textures.hpp>
#include <ak/window/WindowOptions.hpp>

namespace akas {
	struct ImageReference final {
		akd::SUID identifier;
		ImageRotation rotate;
		uint32 layer;
		int32 offsetX, offsetY;
		int32 cropX, cropY;
	};

	struct Texture final {
		akr::gl::TexTarget type;

		std::vector<std::vector<ImageReference>> levels;

		akr::gl::TexFormat format;
		akr::gl::TexStorage storage;

		akr::gl::ClampType clampHorz, clampVert, clampDepth;
		akr::gl::FilterType filterMin, filterMax;
		akr::gl::MipFilterType filterMip;
	};

}

namespace akd {

	inline void serialize(akd::PValue& dst, const akas::ImageReference& src) {
		serialize(dst["identifier"],  src.identifier);
		serialize(dst["rotate"], src.rotate);
		serialize(dst["layer"], src.layer);
		serialize(dst["offsetX"], src.offsetX);
		serialize(dst["offsetY"], src.offsetY);
		serialize(dst["cropX"], src.cropX);
		serialize(dst["cropY"], src.cropY);
	}

	inline bool deserialize(akas::ImageReference& dst, const akd::PValue& src) {
		try {
			akas::ImageReference result;
			if (!deserialize(result.identifier, src["identifier"])) return false;
			if (!deserialize(result.rotate,     src.atOrDef("rotate" , PValue::from("None"))))    return false;
			if (!deserialize(result.layer,      src.atOrDef("layer"  , PValue::from<uint64>(0)))) return false;
			if (!deserialize(result.offsetX,    src.atOrDef("offsetX", PValue::from<int64> (0)))) return false;
			if (!deserialize(result.offsetY,    src.atOrDef("offsetY", PValue::from<int64> (0)))) return false;
			if (!deserialize(result.cropX,      src.atOrDef("cropX"  , PValue::from<int64> (0)))) return false;
			if (!deserialize(result.cropY,      src.atOrDef("cropY"  , PValue::from<int64> (0)))) return false;
			dst = result;
			return true;
		} catch(const std::logic_error&) {
			return false;
		}
	}

}

namespace akd {

	inline void serialize(akd::PValue& dst, const akas::Texture& src) {
		serialize(dst["type"],       src.type);
		serialize(dst["levels"],     src.levels);
		serialize(dst["format"],     src.format);
		serialize(dst["storage"],    src.storage);
		serialize(dst["clampHorz"],  src.clampHorz);
		serialize(dst["clampVert"],  src.clampVert);
		serialize(dst["clampDepth"], src.clampDepth);
		serialize(dst["filterMin"],  src.filterMin);
		serialize(dst["filterMax"],  src.filterMax);
		serialize(dst["filterMip"],  src.filterMip);
	}

	inline bool deserialize(akas::Texture& dst, const akd::PValue& src) {
		try {
			akas::Texture result;
			if (!deserialize(result.type,       src["type"]      )) return false;
			if (!deserialize(result.levels,     src["levels"]    )) return false;
			if (!deserialize(result.format,     src["format"]    )) return false;
			if (!deserialize(result.storage,    src["storage"]   )) return false;
			if (!deserialize(result.clampHorz,  src.atOrDef("clampHorz" , PValue::from("Edge"))))    return false;
			if (!deserialize(result.clampVert,  src.atOrDef("clampVert" , PValue::from("Edge"))))    return false;
			if (!deserialize(result.clampDepth, src.atOrDef("clampDepth", PValue::from("Edge"))))    return false;
			if (!deserialize(result.filterMin,  src.atOrDef("filterMin" , PValue::from("Nearest")))) return false;
			if (!deserialize(result.filterMax,  src.atOrDef("filterMax" , PValue::from("Nearest")))) return false;
			if (!deserialize(result.filterMip,  src.atOrDef("filterMip" , PValue::from("None"))))    return false;
			dst = result;
			return true;
		} catch(const std::logic_error&) {
			return false;
		}
	}

}


#endif /* AK_ASSETS_TEXTURE_HPP_ */
