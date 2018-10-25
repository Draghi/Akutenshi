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
#ifndef AK_ASSETS_SERIALIZE_HPP_
#define AK_ASSETS_SERIALIZE_HPP_

#include <akasset/Animation.hpp>
#include <akasset/Asset.hpp>
#include <akasset/Convert.hpp>
#include <akasset/Image.hpp>
#include <akasset/Material.hpp>
#include <akasset/Mesh.hpp>
#include <akasset/ShaderProgram.hpp>
#include <akasset/Skin.hpp>
#include <akasset/Texture.hpp>
#include <akengine/data/Serialize.hpp>
#include <akengine/data/SmartClass.hpp>
#include <akengine/data/SmartEnum.hpp>
#include <akengine/data/SUID.hpp>
#include <akengine/filesystem/Path.hpp>
#include <akmath/Serialize.hpp>
#include <akrender/gl/Textures.hpp>
#include <akrender/gl/Types.hpp>
#include <akrender/window/WindowOptions.hpp>

AK_SMART_ENUM_SERIALIZE(aka, AlphaMode)
AK_SMART_ENUM_SERIALIZE(aka, AssetSourceType)
AK_SMART_ENUM_SERIALIZE(aka, AssetType)
AK_SMART_ENUM_SERIALIZE(aka, ImageRotation)

AK_SMART_CLASS(aka::AssetInfo,
	FIELD, identifier,
	FIELD, type,
	FIELD, displayName,
	FIELD, source
)

AK_SMART_CLASS(aka::Sampler,
	FIELD, imgAssetID,
	FIELD, texCoordIndex,
	FIELD, minFilter,
	FIELD, minMipFilter,
	FIELD, magFilter,
	FIELD, clampS,
	FIELD, clampT
)

AK_SMART_CLASS(aka::Material,
	FIELD, baseColour,
	FIELD, metallicFactor,
	FIELD, roughnessFactor,
	FIELD, emmisiveFactor,
	FIELD, baseTexture,
	FIELD, metalRoughnessTexture,
	FIELD, normalTexture,
	FIELD, occlusionTexture,
	FIELD, emissiveTexture,
	FIELD, alphaMode,
	FIELD, alphaCutoff,
	FIELD, doubleSided
)

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

AK_SMART_CLASS(aka::VertexSurfaceData,
	FIELD, position,
	FIELD, tangent,
	FIELD, bitangent,
	FIELD, normal
)

AK_SMART_CLASS(aka::VertexWeightData,
	FIELD, bones,
	FIELD, weights
)

AK_SMART_CLASS(aka::Joint,
	FIELD, name,
	FIELD, children,
	FIELD, inverseBindMatrix,
	FIELD, position,
	FIELD, rotation,
	FIELD, scale
)

AK_SMART_CLASS(aka::Skin,
	FIELD, root,
	FIELD, joints,
	FIELD, mapping
)

AK_SMART_CLASS(aka::Primitive,
	FIELD, drawType,
	FIELD, indexData,
	FIELD, surfaceData,
	FIELD, texCoordData,
	FIELD, colourData,
	FIELD, skinningData,
	FIELD, materialAssetID
)

AK_SMART_CLASS(aka::Mesh,
	FIELD, primitives,
	FIELD, skin
)

#endif