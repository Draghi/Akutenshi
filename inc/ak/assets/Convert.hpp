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

#include <cstdlib>
#include <ctime>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <ak/assets/Asset.hpp>
#include <ak/assets/AssetRegistry.hpp>
#include <ak/assets/Material.hpp>
#include <ak/assets/Mesh.hpp>
#include <ak/container/SlotMap.hpp>
#include <ak/data/Rand.hpp>
#include <ak/data/SUID.hpp>
#include <ak/filesystem/Path.hpp>
#include <ak/Log.hpp>
#include <ak/PrimitiveTypes.hpp>

namespace akas {

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
			akc::SlotMap<std::pair<ConversionInfo, akas::Mesh>> m_meshes;
			akc::SlotMap<std::pair<ConversionInfo, akas::Material>> m_materials;

			std::map<akfs::Path, std::pair<akas::AssetType, akc::SlotID>> m_assetsBySource;
			std::map<akfs::Path, std::pair<akas::AssetType, akc::SlotID>> m_assetsByDestination;
			std::map<akd::SUID,  std::pair<akas::AssetType, akc::SlotID>> m_assetsByIdentifier;


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

			void registerAsset(ConversionInfo info, const akfs::Path& path, const std::optional<akfs::Path>& source) {
				auto id = m_images.insert({info, path}).first;
				if (!m_assetsByDestination.emplace(info.destination, std::make_pair(AssetType::Image, id)).second)  throw std::logic_error("Destination conflict for: " + info.destination.str());
				if (!m_assetsByIdentifier.emplace( info.identifier,  std::make_pair(AssetType::Image, id)).second)  throw std::logic_error("Identifier conflict for: "  + info.destination.str());
				if (source) {
					akl::Logger("ConvertionHelper").test_warn(m_assetsBySource.emplace(*source,  std::make_pair(AssetType::Image, id)).second, "Source conflict for: " + info.destination.str());
				}

			}

			void registerAsset(ConversionInfo info, const akas::Mesh& mesh, const std::optional<akfs::Path>& source) {
				auto id = m_meshes.insert({info, mesh}).first;
				if (!m_assetsByDestination.emplace(info.destination,  std::make_pair(AssetType::Mesh, id)).second)  throw std::logic_error("Path conflict for: "       + info.destination.str());
				if (!m_assetsByIdentifier.emplace( info.identifier,   std::make_pair(AssetType::Mesh, id)).second)  throw std::logic_error("Identifier conflict for: " + info.destination.str());
				if (source) {
					akl::Logger("ConvertionHelper").test_warn(m_assetsBySource.emplace(*source,  std::make_pair(AssetType::Mesh, id)).second, "Source conflict for: " + info.destination.str());
				}
			}

			void registerAsset(ConversionInfo info, const akas::Material& material, const std::optional<akfs::Path>& source) {
				auto id = m_materials.insert({info, material}).first;
				if (!m_assetsByDestination.emplace(info.destination, std::make_pair(AssetType::Material, id)).second)  throw std::logic_error("Path conflict for: "       + info.destination.str());
				if (!m_assetsByIdentifier.emplace( info.identifier,  std::make_pair(AssetType::Material, id)).second)  throw std::logic_error("Identifier conflict for: " + info.destination.str());
				if (source) {
					akl::Logger("ConvertionHelper").test_warn(m_assetsBySource.emplace(*source,  std::make_pair(AssetType::Material, id)).second, "Source conflict for: " + info.destination.str());
				}
			}

			void rescan() { m_assetRegistry.rescan(); }

			std::optional<ConversionInfo> tryFindAssetInfoByID(const akd::SUID& identifier) const {
				auto assetInfo = m_assetRegistry.tryGetAssetInfoBySUID(identifier);
				if (assetInfo) return {{assetInfo->first.identifier, assetInfo->second, assetInfo->first.displayName}};

				auto lookupIter = m_assetsByIdentifier.find(identifier);
				if (lookupIter == m_assetsByIdentifier.end()) return {};
				switch(lookupIter->second.first) {
					case AssetType::Mesh:      return    m_meshes[lookupIter->second.second].first;
					case AssetType::Material:  return m_materials[lookupIter->second.second].first;
					case AssetType::Image:     return    m_images[lookupIter->second.second].first;
					case AssetType::Animation: [[fallthrough]];
					case AssetType::Prefab:    [[fallthrough]];
					case AssetType::Scene:     [[fallthrough]];
					case AssetType::Sound:     [[fallthrough]];
					case AssetType::Texture:   [[fallthrough]];
					default: return {};
				}
			}

			std::optional<ConversionInfo> tryFindAssetInfoByDestination(const akfs::Path& path) const {
				auto assetInfo = m_assetRegistry.tryGetAssetInfoByDestination(path);
				if (assetInfo) return {{assetInfo->first.identifier, assetInfo->second, assetInfo->first.displayName}};

				auto lookupIter = m_assetsByDestination.find(path);
				if (lookupIter == m_assetsByDestination.end()) return {};
				switch(lookupIter->second.first) {
					case AssetType::Mesh:      return    m_meshes[lookupIter->second.second].first;
					case AssetType::Material:  return m_materials[lookupIter->second.second].first;
					case AssetType::Image:     return    m_images[lookupIter->second.second].first;
					case AssetType::Animation: [[fallthrough]];
					case AssetType::Prefab:    [[fallthrough]];
					case AssetType::Scene:     [[fallthrough]];
					case AssetType::Sound:     [[fallthrough]];
					case AssetType::Texture:   [[fallthrough]];
					default: return {};
				}
			}

			std::optional<ConversionInfo> tryFindAssetInfoBySource(const akfs::Path& path) const {
				auto assetInfo = m_assetRegistry.tryGetAssetInfoBySource(path);
				if (assetInfo) return {{assetInfo->first.identifier, assetInfo->second, assetInfo->first.displayName}};

				auto lookupIter = m_assetsBySource.find(path);
				if (lookupIter == m_assetsBySource.end()) return {};
				switch(lookupIter->second.first) {
					case AssetType::Mesh:      return    m_meshes[lookupIter->second.second].first;
					case AssetType::Material:  return m_materials[lookupIter->second.second].first;
					case AssetType::Image:     return    m_images[lookupIter->second.second].first;
					case AssetType::Animation: [[fallthrough]];
					case AssetType::Prefab:    [[fallthrough]];
					case AssetType::Scene:     [[fallthrough]];
					case AssetType::Sound:     [[fallthrough]];
					case AssetType::Texture:   [[fallthrough]];
					default: return {};
				}
			}

			const akc::SlotMap<std::pair<ConversionInfo, akfs::Path>>& images() const { return m_images; }
			const akc::SlotMap<std::pair<ConversionInfo, akas::Mesh>>& meshes() const { return m_meshes; }
			const akc::SlotMap<std::pair<ConversionInfo, akas::Material>>& materials() const { return m_materials; }

			auto& getImage(akSize i) { return m_images[i].second; }
			const auto& getImage(akSize i) const { return m_images[i].second; }
			akSize imageCount() const { return m_images.size(); }

			auto& getMesh(akSize i) { return m_meshes[i].second; }
			const auto& getMesh(akSize i) const { return m_meshes[i].second; }
			akSize meshCount() const { return m_meshes.size(); }

			auto& getMaterial(akSize i) { return m_materials[i].second; }
			const auto& getMaterial(akSize i) const { return m_materials[i].second; }
			akSize materialCount() const { return m_materials.size(); }
	};

	void convertDirectory(const akfs::Path& dir);

}



#endif /* AK_ASSETS_CONVERT_HPP_ */
