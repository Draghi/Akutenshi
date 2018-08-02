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

#include <optional>
#include <unordered_map>
#include <utility>

#include <ak/assets/Asset.hpp>
#include <ak/container/SlotMap.hpp>
#include <ak/data/SUID.hpp>
#include <ak/filesystem/Path.hpp>

namespace akas {

	class AssetRegistry final {
		private:
			bool m_supressWarnings;
			akfs::Path m_scanRoot;

			akc::SlotMap<std::pair<akas::AssetInfo, akfs::Path>> m_assetInfo;
			std::unordered_map<akd::SUID,  akc::SlotID> m_assetBySUID;
			std::unordered_map<akfs::Path, akc::SlotID> m_assetByDestination;
			std::unordered_map<akfs::Path, akc::SlotID> m_assetBySource;

			bool proccessFile(const akfs::Path& path);

		public:
			AssetRegistry(const akfs::Path& scanRoot, bool supressWarnings = false);

			void setRoot(const akfs::Path& scanRoot);
			void rescan();

			std::optional<std::pair<akas::AssetInfo, akfs::Path>> tryGetAssetInfoBySUID(const akd::SUID& suid) const;
			std::optional<std::pair<akas::AssetInfo, akfs::Path>> tryGetAssetInfoBySource(const akfs::Path& path) const;
			std::optional<std::pair<akas::AssetInfo, akfs::Path>> tryGetAssetInfoByDestination(const akfs::Path& path) const;
	};

}

#endif /* AK_ASSETS_ASSETREGISTRY_HPP_ */
