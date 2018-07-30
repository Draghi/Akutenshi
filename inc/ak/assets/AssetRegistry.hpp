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

#ifndef AK_ASSETS_ASSETREGISTRY_HPP_
#define AK_ASSETS_ASSETREGISTRY_HPP_

#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include <ak/assets/Asset.hpp>
#include <ak/container/SlotMap.hpp>
#include <ak/data/Json.hpp>
#include <ak/data/PValueFile.hpp>
#include <ak/data/SUID.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/filesystem/Path.hpp>
#include <ak/Log.hpp>
#include <ak/String.hpp>

namespace akas {

	class AssetRegistry final {
		private:
			bool m_supressWarnings;
			akfs::Path m_scanRoot;

			akc::SlotMap<std::pair<akas::AssetInfo, akfs::Path>> m_assetInfo;
			std::unordered_map<akd::SUID,  akc::SlotID> m_assetBySUID;
			std::unordered_map<akfs::Path, akc::SlotID> m_assetByDestination;
			std::unordered_map<akfs::Path, akc::SlotID> m_assetBySource;

			bool proccessFile(const akfs::Path& path) {
				if (path.extension() != ".akres") return true;

				auto assetInfo = akd::tryDeserializeFromFile<akas::AssetInfo>(path, &akd::fromJsonFile);
				if (!assetInfo) { akl::Logger("AssetFinder").warn("Failed to parse asset info file: ", path.str()); return true; }

				auto id = m_assetInfo.insert(std::make_pair(*assetInfo, path)).first;
				if (!m_assetBySUID.emplace(assetInfo->identifier, id).second) throw std::runtime_error(ak::buildString("Asset SUID conflict for: ", path.str()));
				if (!assetInfo->source.empty()) akl::Logger("AssetFinder").test_warn(m_assetBySource.emplace(assetInfo->source, id).second || m_supressWarnings, "Asset source conflict for: ", path.str());
				if (!m_assetByDestination.emplace(path, id).second) throw std::runtime_error(ak::buildString("Asset destination conflict for: ", path.str()));

				return true;
			}

		public:
			AssetRegistry(const akfs::Path& scanRoot, bool supressWarnings = false) : m_supressWarnings(supressWarnings), m_scanRoot(scanRoot), m_assetBySUID() { rescan(); }

			void setRoot(const akfs::Path& scanRoot) { m_scanRoot = scanRoot; rescan(); }

			void rescan() {
				m_assetBySUID.clear();
				m_assetByDestination.clear();
				akfs::iterateDirectory(m_scanRoot, std::bind(&AssetRegistry::proccessFile, this, std::placeholders::_1), true);
			}

			std::optional<std::pair<akas::AssetInfo, akfs::Path>> tryGetAssetInfoBySUID(const akd::SUID& suid) const {
				auto iter = m_assetBySUID.find(suid);
				if (iter == m_assetBySUID.end()) return {};
				return {m_assetInfo[iter->second]};
			}

			std::optional<std::pair<akas::AssetInfo, akfs::Path>> tryGetAssetInfoBySource(const akfs::Path& path) const {
				auto iter = m_assetBySource.find(path);
				if (iter == m_assetBySource.end()) return {};
				return {m_assetInfo[iter->second]};
			}

			std::optional<std::pair<akas::AssetInfo, akfs::Path>> tryGetAssetInfoByDestination(const akfs::Path& path) const {
				auto iter = m_assetByDestination.find(path);
				if (iter == m_assetByDestination.end()) return {};
				return {m_assetInfo[iter->second]};
			}

	};

}

#endif /* AK_ASSETS_ASSETREGISTRY_HPP_ */
