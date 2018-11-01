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

#ifndef AK_ASSETS_CONVERT_HPP_
#define AK_ASSETS_CONVERT_HPP_

#include <akasset/Animation.hpp>
#include <akasset/Asset.hpp>
#include <akasset/AssetRegistry.hpp>
#include <akasset/Material.hpp>
#include <akasset/Mesh.hpp>
#include <akasset/ShaderProgram.hpp>
#include <akasset/Texture.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/SlotMap.hpp>
#include <akengine/data/Random.hpp>
#include <akengine/data/SmartEnum.hpp>
#include <akengine/data/SUID.hpp>
#include <akengine/filesystem/Path.hpp>
#include <cstdlib>
#include <ctime>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace aka {
	AK_SMART_TENUM_CLASS_KV(AssetSourceType, uint64,
		GLTF,          100,

		ShaderProgram,   4,
		ShaderStage,     3,

		Sound,           2,
		Texture,         1,
		Image,           0
	)

	struct ConversionInfo {
		akd::SUID identifier;
		akfs::Path destination;
		std::string displayName;
	};

	class ConversionHelper final {
		private:
			AssetRegistry m_assetRegistry;
			mutable akd::CMW4096Engine32d m_randomGenerator;

			akc::SlotMap<std::pair<ConversionInfo, akfs::Path>> m_images;
			akc::SlotMap<std::pair<ConversionInfo, aka::Mesh>> m_meshes;
			akc::SlotMap<std::pair<ConversionInfo, aka::Material>> m_materials;
			akc::SlotMap<std::pair<ConversionInfo, aka::Animation>> m_animations;
			akc::SlotMap<std::pair<ConversionInfo, aka::Texture>> m_textures;
			akc::SlotMap<std::pair<ConversionInfo, akfs::Path>> m_shaderStages;
			akc::SlotMap<std::pair<ConversionInfo, aka::ShaderProgram>> m_shaderPrograms;
			akc::SlotMap<std::pair<ConversionInfo, akfs::Path>> m_sounds;

			std::map<akfs::Path, std::pair<aka::AssetType, akc::SlotID>> m_assetsBySource;
			std::map<akfs::Path, std::pair<aka::AssetType, akc::SlotID>> m_assetsByDestination;
			std::map<akd::SUID,  std::pair<aka::AssetType, akc::SlotID>> m_assetsByIdentifier;


		public:
			ConversionHelper(const akfs::Path& searchRoot, bool supressWarnings = false) : m_assetRegistry(searchRoot, supressWarnings), m_randomGenerator(akd::CMW4096Engine32d::default_seed ^ std::rand() ^ std::time(nullptr)) {}

			ConversionInfo findConversionInfo(const std::string defName, const std::optional<akd::SUID>& setIdentifier, const std::optional<akfs::Path>& source, const std::optional<akfs::Path>& setDestination, const akfs::Path& defDestination) const {
				if (setIdentifier) {
					auto info = tryFindAssetInfoByID(*setIdentifier);
					if (info) return *info;
					else return ConversionInfo{*setIdentifier, setDestination.value_or(defDestination), defName};
				}

				if (setDestination) {
					auto info = tryFindAssetInfoByDestination(*setDestination);
					if (info) return *info;
					else return ConversionInfo{akd::generateSUID(m_randomGenerator), *setDestination, defName};
				}

				if (source) {
					auto info = tryFindAssetInfoBySource(*source);
					if (info) return *info;
				}

				auto info = tryFindAssetInfoByDestination(defDestination);
				if (info) return *info;
				else return ConversionInfo{akd::generateSUID(m_randomGenerator), defDestination, defName};

			}

			akfs::Path getDestinationSuggestionFor(const akfs::Path& root, const std::string& name) {
				auto result = root/(name+".akres");
				if ((result.front() == "./") || (result.front() == "../")) result.pop_front();
				result = akfs::Path("data")/result.pop_front();
				return result;
			}

			void registerImage(ConversionInfo info, const akfs::Path& path, const std::optional<akfs::Path>& source) {
				auto id = m_images.insert({info, path}).first;
				if (!m_assetsByDestination.emplace(info.destination, std::make_pair(AssetType::Image, id)).second)  throw std::logic_error("Destination conflict for: " + info.destination.str());
				if (!m_assetsByIdentifier.emplace( info.identifier,  std::make_pair(AssetType::Image, id)).second)  throw std::logic_error("Identifier conflict for: "  + info.destination.str());
				if (source) {
					akl::Logger("ConvertionHelper").test_warn(m_assetsBySource.emplace(*source,  std::make_pair(AssetType::Image, id)).second, "Source conflict for: " + info.destination.str());
				}
			}

			void registerShaderStage(ConversionInfo info, const akfs::Path& path, const std::optional<akfs::Path>& source) {
				auto id = m_shaderStages.insert({info, path}).first;
				if (!m_assetsByDestination.emplace(info.destination, std::make_pair(AssetType::ShaderStage, id)).second)  throw std::logic_error("Destination conflict for: " + info.destination.str());
				if (!m_assetsByIdentifier.emplace( info.identifier,  std::make_pair(AssetType::ShaderStage, id)).second)  throw std::logic_error("Identifier conflict for: "  + info.destination.str());
				if (source) {
					akl::Logger("ConvertionHelper").test_warn(m_assetsBySource.emplace(*source,  std::make_pair(AssetType::ShaderStage, id)).second, "Source conflict for: " + info.destination.str());
				}
			}

			void registerShaderProgram(ConversionInfo info, const aka::ShaderProgram& shaderProgram, const std::optional<akfs::Path>& source) {
				auto id = m_shaderPrograms.insert({info, shaderProgram}).first;
				if (!m_assetsByDestination.emplace(info.destination, std::make_pair(AssetType::ShaderStage, id)).second)  throw std::logic_error("Destination conflict for: " + info.destination.str());
				if (!m_assetsByIdentifier.emplace( info.identifier,  std::make_pair(AssetType::ShaderStage, id)).second)  throw std::logic_error("Identifier conflict for: "  + info.destination.str());
				if (source) {
					akl::Logger("ConvertionHelper").test_warn(m_assetsBySource.emplace(*source,  std::make_pair(AssetType::ShaderProgram, id)).second, "Source conflict for: " + info.destination.str());
				}
			}

			void registerMesh(ConversionInfo info, const aka::Mesh& mesh, const std::optional<akfs::Path>& source) {
				auto id = m_meshes.insert({info, mesh}).first;
				if (!m_assetsByDestination.emplace(info.destination,  std::make_pair(AssetType::Mesh, id)).second)  throw std::logic_error("Path conflict for: "       + info.destination.str());
				if (!m_assetsByIdentifier.emplace( info.identifier,   std::make_pair(AssetType::Mesh, id)).second)  throw std::logic_error("Identifier conflict for: " + info.destination.str());
				if (source) {
					akl::Logger("ConvertionHelper").test_warn(m_assetsBySource.emplace(*source,  std::make_pair(AssetType::Mesh, id)).second, "Source conflict for: " + info.destination.str());
				}
			}

			void registerAnimation(ConversionInfo info, const aka::Animation& mesh, const std::optional<akfs::Path>& source) {
				auto id = m_animations.insert({info, mesh}).first;
				if (!m_assetsByDestination.emplace(info.destination,  std::make_pair(AssetType::Animation, id)).second)  throw std::logic_error("Path conflict for: "       + info.destination.str());
				if (!m_assetsByIdentifier.emplace( info.identifier,   std::make_pair(AssetType::Animation, id)).second)  throw std::logic_error("Identifier conflict for: " + info.destination.str());
				if (source) {
					akl::Logger("ConvertionHelper").test_warn(m_assetsBySource.emplace(*source,  std::make_pair(AssetType::Animation, id)).second, "Source conflict for: " + info.destination.str());
				}
			}

			void registerMaterial(ConversionInfo info, const aka::Material& material, const std::optional<akfs::Path>& source) {
				auto id = m_materials.insert({info, material}).first;
				if (!m_assetsByDestination.emplace(info.destination, std::make_pair(AssetType::Material, id)).second)  throw std::logic_error("Path conflict for: "       + info.destination.str());
				if (!m_assetsByIdentifier.emplace( info.identifier,  std::make_pair(AssetType::Material, id)).second)  throw std::logic_error("Identifier conflict for: " + info.destination.str());
				if (source) {
					akl::Logger("ConvertionHelper").test_warn(m_assetsBySource.emplace(*source,  std::make_pair(AssetType::Material, id)).second, "Source conflict for: " + info.destination.str());
				}
			}

			void registerTexture(ConversionInfo info, const aka::Texture& texture, const std::optional<akfs::Path>& source) {
				auto id = m_textures.insert({info, texture}).first;
				if (!m_assetsByDestination.emplace(info.destination, std::make_pair(AssetType::Texture, id)).second)  throw std::logic_error("Path conflict for: "       + info.destination.str());
				if (!m_assetsByIdentifier.emplace( info.identifier,  std::make_pair(AssetType::Texture, id)).second)  throw std::logic_error("Identifier conflict for: " + info.destination.str());
				if (source) {
					akl::Logger("ConvertionHelper").test_warn(m_assetsBySource.emplace(*source,  std::make_pair(AssetType::Texture, id)).second, "Source conflict for: " + info.destination.str());
				}
			}

			void registerSound(ConversionInfo info, const akfs::Path& sound, const std::optional<akfs::Path>& source) {
				auto id = m_sounds.insert({info, sound}).first;
				if (!m_assetsByDestination.emplace(info.destination, std::make_pair(AssetType::Sound, id)).second)  throw std::logic_error("Path conflict for: "       + info.destination.str());
				if (!m_assetsByIdentifier.emplace( info.identifier,  std::make_pair(AssetType::Sound, id)).second)  throw std::logic_error("Identifier conflict for: " + info.destination.str());
				if (source) {
					akl::Logger("ConvertionHelper").test_warn(m_assetsBySource.emplace(*source,  std::make_pair(AssetType::Sound, id)).second, "Source conflict for: " + info.destination.str());
				}
			}

			void rescan() { m_assetRegistry.rescan(); }

			std::optional<ConversionInfo> tryFindAssetInfoByID(const akd::SUID& identifier) const {
				auto assetInfo = m_assetRegistry.tryGetAssetInfoBySUID(identifier);
				if (assetInfo) return {{assetInfo->first.identifier, assetInfo->second, assetInfo->first.displayName}};

				auto lookupIter = m_assetsByIdentifier.find(identifier);
				if (lookupIter == m_assetsByIdentifier.end()) return {};
				switch(lookupIter->second.first) {
					case AssetType::Mesh:        return     m_meshes[lookupIter->second.second].first;
					case AssetType::Material:    return  m_materials[lookupIter->second.second].first;
					case AssetType::Image:       return     m_images[lookupIter->second.second].first;
					case AssetType::Animation:   return m_animations[lookupIter->second.second].first;
					case AssetType::Prefab:      [[fallthrough]];
					case AssetType::Scene:       [[fallthrough]];
					case AssetType::ShaderStage: return m_shaderStages[lookupIter->second.second].first;
					case AssetType::ShaderProgram: return m_shaderPrograms[lookupIter->second.second].first;
					case AssetType::Sound:       [[fallthrough]];
					case AssetType::Texture:     return m_textures[lookupIter->second.second].first;
					default: return {};
				}
			}

			std::optional<ConversionInfo> tryFindAssetInfoByDestination(const akfs::Path& path) const {
				auto assetInfo = m_assetRegistry.tryGetAssetInfoByDestination(path);
				if (assetInfo) return {{assetInfo->first.identifier, assetInfo->second, assetInfo->first.displayName}};

				auto lookupIter = m_assetsByDestination.find(path);
				if (lookupIter == m_assetsByDestination.end()) return {};
				switch(lookupIter->second.first) {
					case AssetType::Mesh:      return     m_meshes[lookupIter->second.second].first;
					case AssetType::Material:  return  m_materials[lookupIter->second.second].first;
					case AssetType::Image:     return     m_images[lookupIter->second.second].first;
					case AssetType::Animation: return m_animations[lookupIter->second.second].first;
					case AssetType::Prefab:    [[fallthrough]];
					case AssetType::Scene:     [[fallthrough]];
					case AssetType::ShaderStage: return m_shaderStages[lookupIter->second.second].first;
					case AssetType::ShaderProgram: return m_shaderPrograms[lookupIter->second.second].first;
					case AssetType::Sound:     return m_sounds[lookupIter->second.second].first;
					case AssetType::Texture:   return m_textures[lookupIter->second.second].first;
					default: return {};
				}
			}

			std::optional<ConversionInfo> tryFindAssetInfoBySource(const akfs::Path& path) const {
				auto assetInfo = m_assetRegistry.tryGetAssetInfoBySource(path);
				if (assetInfo) return {{assetInfo->first.identifier, assetInfo->second, assetInfo->first.displayName}};

				auto lookupIter = m_assetsBySource.find(path);
				if (lookupIter == m_assetsBySource.end()) return {};
				switch(lookupIter->second.first) {
					case AssetType::Mesh:        return     m_meshes[lookupIter->second.second].first;
					case AssetType::Material:    return  m_materials[lookupIter->second.second].first;
					case AssetType::Image:       return     m_images[lookupIter->second.second].first;
					case AssetType::Animation:   return m_animations[lookupIter->second.second].first;
					case AssetType::Prefab:      [[fallthrough]];
					case AssetType::Scene:       [[fallthrough]];
					case AssetType::ShaderStage: return m_shaderStages[lookupIter->second.second].first;
					case AssetType::ShaderProgram: return m_shaderPrograms[lookupIter->second.second].first;
					case AssetType::Sound:       return m_sounds[lookupIter->second.second].first;
					case AssetType::Texture:     return m_textures[lookupIter->second.second].first;
					default: return {};
				}
			}

			const akc::SlotMap<std::pair<ConversionInfo, akfs::Path>>& images() const { return m_images; }
			const akc::SlotMap<std::pair<ConversionInfo, akfs::Path>>& shaderStages() const { return m_shaderStages; }
			const akc::SlotMap<std::pair<ConversionInfo, aka::ShaderProgram>>& shaderPrograms() const { return m_shaderPrograms; }
			const akc::SlotMap<std::pair<ConversionInfo, aka::Mesh>>& meshes() const { return m_meshes; }
			const akc::SlotMap<std::pair<ConversionInfo, aka::Material>>& materials() const { return m_materials; }
			const akc::SlotMap<std::pair<ConversionInfo, aka::Animation>>& animations() const { return m_animations; }
			const akc::SlotMap<std::pair<ConversionInfo, aka::Texture>>& textures() const { return m_textures; }
			const akc::SlotMap<std::pair<ConversionInfo, akfs::Path>>& sounds() const { return m_sounds; }

			auto& getImages(akSize i) { return m_images[i].second; }
			const auto& getImages(akSize i) const { return m_images[i].second; }
			akSize imageCount() const { return m_images.size(); }

			auto& getShaderStages(akSize i) { return m_shaderStages[i].second; }
			const auto& getShaderStages(akSize i) const { return m_shaderStages[i].second; }
			akSize shaderStageCount() const { return m_shaderStages.size(); }

			auto& getShaderPrograms(akSize i) { return m_shaderPrograms[i].second; }
			const auto& getShaderPrograms(akSize i) const { return m_shaderPrograms[i].second; }
			akSize shaderProgramCount() const { return m_shaderPrograms.size(); }

			auto& getMesh(akSize i) { return m_meshes[i].second; }
			const auto& getMesh(akSize i) const { return m_meshes[i].second; }
			akSize meshCount() const { return m_meshes.size(); }

			auto& getAnimation(akSize i) { return m_animations[i].second; }
			const auto& getAnimations(akSize i) const { return m_animations[i].second; }
			akSize animationCount() const { return m_animations.size(); }

			auto& getMaterial(akSize i) { return m_materials[i].second; }
			const auto& getMaterial(akSize i) const { return m_materials[i].second; }
			akSize materialCount() const { return m_materials.size(); }

			auto& getTexture(akSize i) { return m_textures[i].second; }
			const auto& getTexture(akSize i) const { return m_textures[i].second; }
			akSize textureCount() const { return m_textures.size(); }

			auto& getSound(akSize i) { return m_sounds[i].second; }
			const auto& getSound(akSize i) const { return m_sounds[i].second; }
			akSize soundCount() const { return m_sounds.size(); }
	};

	void convertDirectory(const akfs::Path& dir);

}

AK_SMART_ENUM_SERIALIZE(aka, AssetSourceType)

#endif /* AK_ASSETS_CONVERT_HPP_ */
