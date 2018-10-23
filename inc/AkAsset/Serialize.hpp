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

#include <AkAsset/Animation.hpp>
#include <AkAsset/Asset.hpp>
#include <AkAsset/Convert.hpp>
#include <AkAsset/Image.hpp>
#include <AkAsset/Material.hpp>
#include <AkAsset/Mesh.hpp>
#include <AkAsset/ShaderProgram.hpp>
#include <AkAsset/Skin.hpp>
#include <AkAsset/Texture.hpp>
#include <AkEngine/data/Serialize.hpp>
#include <AkEngine/data/SmartClass.hpp>
#include <AkEngine/data/SmartEnum.hpp>
#include <AkEngine/data/SUID.hpp>
#include <AkEngine/filesystem/Path.hpp>
#include <AkMath/Serialize.hpp>
#include <AkRender/gl/Textures.hpp>
#include <AkRender/gl/Types.hpp>
#include <AkRender/window/WindowOptions.hpp>

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
