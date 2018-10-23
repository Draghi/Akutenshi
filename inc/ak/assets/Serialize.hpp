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

#include <ak/animation/Serialize.hpp>
#include <ak/data/Serialize.hpp>
#include <ak/math/Serialize.hpp>

#include <ak/assets/Asset.hpp>
#include <ak/assets/Convert.hpp>
#include <ak/assets/Image.hpp>
#include <ak/assets/Material.hpp>
#include <ak/assets/Mesh.hpp>
#include <ak/assets/Skin.hpp>
#include <ak/assets/Texture.hpp>
#include <ak/data/SmartClass.hpp>
#include <ak/data/SmartEnum.hpp>
#include <ak/render/gl/Textures.hpp>

AK_SMART_ENUM_SERIALIZE(akas, AlphaMode)
AK_SMART_ENUM_SERIALIZE(akas, AssetSourceType)
AK_SMART_ENUM_SERIALIZE(akas, AssetType)
AK_SMART_ENUM_SERIALIZE(akas, ImageRotation)

AK_SMART_CLASS(akas::AssetInfo,
	FIELD, identifier,
	FIELD, type,
	FIELD, displayName,
	FIELD, source
)

AK_SMART_CLASS(akas::Sampler,
	FIELD, imgAssetID,
	FIELD, texCoordIndex,
	FIELD, minFilter,
	FIELD, minMipFilter,
	FIELD, magFilter,
	FIELD, clampS,
	FIELD, clampT
)

AK_SMART_CLASS(akas::Material,
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

AK_SMART_CLASS(akas::ImageReference,
	FIELD, identifier,
	FIELD_DEFAULT, rotate,  akas::ImageRotation::None,
	FIELD_DEFAULT, layer,   0,
	FIELD_DEFAULT, offsetX, 0,
	FIELD_DEFAULT, offsetY, 0,
	FIELD_DEFAULT, cropX,   0,
	FIELD_DEFAULT, cropY,   0
)

AK_SMART_CLASS(akas::Texture,
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

AK_SMART_CLASS(akas::VertexSurfaceData,
	FIELD, position,
	FIELD, tangent,
	FIELD, bitangent,
	FIELD, normal
)

AK_SMART_CLASS(akas::VertexWeightData,
	FIELD, bones,
	FIELD, weights
)

AK_SMART_CLASS(akas::Joint,
	FIELD, name,
	FIELD, children,
	FIELD, inverseBindMatrix,
	FIELD, position,
	FIELD, rotation,
	FIELD, scale
)

AK_SMART_CLASS(akas::Skin,
	FIELD, root,
	FIELD, joints,
	FIELD, mapping
)

AK_SMART_CLASS(akas::Primitive,
	FIELD, drawType,
	FIELD, indexData,
	FIELD, surfaceData,
	FIELD, texCoordData,
	FIELD, colourData,
	FIELD, skinningData,
	FIELD, materialAssetID
)

AK_SMART_CLASS(akas::Mesh,
	FIELD, primitives,
	FIELD, skin
)

#endif
