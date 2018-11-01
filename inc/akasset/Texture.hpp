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

#include <akasset/Image.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/data/SUID.hpp>
#include <vector>

namespace aka {
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

AK_SMART_CLASS(aka::ImageReference,
	FIELD, identifier,
	FIELD_DEFAULT, rotate,  aka::ImageRotation::None,
	FIELD_DEFAULT, layer,   0,
	FIELD_DEFAULT, offsetX, 0,
	FIELD_DEFAULT, offsetY, 0,
	FIELD_DEFAULT, cropX,   0,
	FIELD_DEFAULT, cropY,   0
)

AK_SMART_CLASS(aka::Texture,
	FIELD, type,
	FIELD, levels,
	FIELD, format,
	FIELD, storage,
	FIELD_DEFAULT, clampHorz,  akr::gl::ClampType::Edge,
	FIELD_DEFAULT, clampVert,  akr::gl::ClampType::Edge,
	FIELD_DEFAULT, clampDepth, akr::gl::ClampType::Edge,
	FIELD_DEFAULT, filterMin,  akr::gl::FilterType::Nearest,
	FIELD_DEFAULT, filterMax,  akr::gl::FilterType::Nearest,
	FIELD_DEFAULT, filterMip,  akr::gl::MipFilterType::None
)

#endif /* AK_ASSETS_TEXTURE_HPP_ */
