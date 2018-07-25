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
		akd::SUID image;
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

		akr::gl::ClampDir clampHorz, clampVert, clampDepth;
		akr::gl::FilterType filterMin, filterMax;
		akr::gl::MipFilterType filterMip;
	};

}

namespace akd {

	inline void serialize(akd::PValue& dst, const akas::ImageReference& src) {
		serialize(dst["image"],  src.image);
		serialize(dst["rotate"], src.rotate);
		  dst["layer"].setUInt(src.layer);
		dst["offsetX"].setInt( src.offsetX);
		dst["offsetY"].setInt( src.offsetY);
		  dst["cropX"].setInt( src.cropX);
		  dst["cropY"].setInt( src.cropY);
	}

	inline bool deserialize(akas::ImageReference& dst, const akd::PValue& src) {
		try {
			akas::ImageReference result;
			deserialize(result.image,  src["image"]);
			deserialize(result.rotate, src["rotate"]);
			result.layer =   src["layer"].as<uint32>();
			result.offsetX = src["offsetX"].as<int32>();
			result.offsetY = src["offsetY"].as<int32>();
			result.cropX =   src["cropX"].as<int32>();
			result.cropY =   src["cropY"].as<int32>();
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
			deserialize(result.type,       src["type"]      );
			deserialize(result.levels,     src["levels"]    );
			deserialize(result.format,     src["format"]    );
			deserialize(result.storage,    src["storage"]   );
			deserialize(result.clampHorz,  src["clampHorz"] );
			deserialize(result.clampVert,  src["clampVert"] );
			deserialize(result.clampDepth, src["clampDepth"]);
			deserialize(result.filterMin,  src["filterMin"] );
			deserialize(result.filterMax,  src["filterMax"] );
			deserialize(result.filterMip,  src["filterMip"] );
			dst = result;
			return true;
		} catch(const std::logic_error&) {
			return false;
		}
	}

}


#endif /* AK_ASSETS_TEXTURE_HPP_ */
