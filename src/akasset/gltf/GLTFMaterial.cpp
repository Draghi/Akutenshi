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

#include <akasset/Convert.hpp>
#include <akasset/gltf/Asset.hpp>
#include <akasset/gltf/GLTFMaterial.hpp>
#include <akasset/gltf/Material.hpp>
#include <akasset/gltf/Sampler.hpp>
#include <akasset/gltf/Texture.hpp>
#include <akasset/Material.hpp>
#include <akrender/gl/Textures.hpp>
#include <optional>
#include <stdexcept>
#include <vector>

using namespace aka::gltf;

static std::optional<aka::Sampler> extractTextureInfo(const Asset& asset, TextureInfo textureInfo, const std::vector<aka::ConversionInfo>& textureAssetIDs);

aka::Material aka::gltf::proccessGLTFMaterial(const Asset& asset, const Material& material, const std::vector<aka::ConversionInfo>& textureAssetIDs) {

	return aka::Material{
		material.baseFactor,
		material.roughnessFactor,
		material.metallicFactor,
		material.emissiveFactor,

		::extractTextureInfo(asset, material.baseTexture,              textureAssetIDs),
		::extractTextureInfo(asset, material.metallicRoughnessTexture, textureAssetIDs),
		::extractTextureInfo(asset, material.normalTexture,            textureAssetIDs),
		::extractTextureInfo(asset, material.occlusionTexture,         textureAssetIDs),
		::extractTextureInfo(asset, material.emissiveTexture,          textureAssetIDs),

		static_cast<aka::AlphaMode>(material.alphaMode),
		material.alphaCutoff,
		material.doubleSided
	};
}


static std::optional<aka::Sampler> extractTextureInfo(const Asset& asset, TextureInfo textureInfo, const std::vector<aka::ConversionInfo>& textureAssetIDs) {
	if (textureInfo.index < 0) return {};
	auto& texture = asset.textures[textureInfo.index];

	if (texture.sourceID < 0) return {};
	auto& sampler = asset.samplers[texture.sourceID];

	akr::gl::FilterType minFilter;
	akr::gl::MipFilterType mipFilter;
	     if (sampler.minFilter == MinFilter::Nearest               ) { minFilter = akr::gl::FilterType::Nearest; mipFilter = akr::gl::MipFilterType::None;    }
	else if (sampler.minFilter == MinFilter::Nearest_Mipmap_Nearest) { minFilter = akr::gl::FilterType::Nearest; mipFilter = akr::gl::MipFilterType::Nearest; }
	else if (sampler.minFilter == MinFilter::Nearest_Mipmap_Linear ) { minFilter = akr::gl::FilterType::Nearest; mipFilter = akr::gl::MipFilterType::Linear;  }
	else if (sampler.minFilter == MinFilter::Linear                ) { minFilter = akr::gl::FilterType::Linear;  mipFilter = akr::gl::MipFilterType::None;    }
	else if (sampler.minFilter == MinFilter::Linear_Mipmap_Nearest ) { minFilter = akr::gl::FilterType::Linear;  mipFilter = akr::gl::MipFilterType::Nearest; }
	else if (sampler.minFilter == MinFilter::Linear_Mipmap_Linear  ) { minFilter = akr::gl::FilterType::Linear;  mipFilter = akr::gl::MipFilterType::Linear;  }
	else throw std::runtime_error("Unsupported MinFilter.");

	akr::gl::FilterType magFilter;
		 if (sampler.magFilter == MagFilter::Nearest) { magFilter = akr::gl::FilterType::Nearest; }
	else if (sampler.magFilter == MagFilter::Linear ) { magFilter = akr::gl::FilterType::Linear;  }
	else throw std::runtime_error("Unsupported MagFilter.");

	akr::gl::ClampType wrapS;
		 if (sampler.wrapS == Wrap::ClampToEdge   ) { wrapS = akr::gl::ClampType::Edge;   }
	else if (sampler.wrapS == Wrap::MirroredRepeat) { wrapS = akr::gl::ClampType::Mirror; }
	else if (sampler.wrapS == Wrap::Repeat        ) { wrapS = akr::gl::ClampType::Repeat; }
	else throw std::runtime_error("Unsupported WrapS.");

	akr::gl::ClampType wrapT;
		 if (sampler.wrapT == Wrap::ClampToEdge   ) { wrapT = akr::gl::ClampType::Edge;   }
	else if (sampler.wrapT == Wrap::MirroredRepeat) { wrapT = akr::gl::ClampType::Mirror; }
	else if (sampler.wrapT == Wrap::Repeat        ) { wrapT = akr::gl::ClampType::Repeat; }
	else throw std::runtime_error("Unsupported WrapT.");

	return {{textureAssetIDs[texture.sourceID].identifier, textureInfo.texCoordSet, minFilter, mipFilter, magFilter, wrapS, wrapT}};
}
