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

#include <akasset/AssetRegistry.hpp>
#include <akcommon/String.hpp>
#include <akengine/data/Json.hpp>
#include <akengine/data/PValue.hpp>
#include <akengine/filesystem/Filesystem.hpp>
#include <functional>
#include <stdexcept>
#include <string>

using namespace aka;

bool AssetRegistry::proccessFile(const akfs::Path& path) {
	if (path.extension() != ".akres") return true;

	auto assetInfo = akd::tryDeserialize<aka::AssetInfo>(akd::fromJsonFile(path));
	if (!assetInfo) {
		akl::Logger("AssetFinder").warn("Failed to parse asset info file: ", path.str());
		return true;
	}

	auto id = m_assetInfo.insert(std::make_pair(*assetInfo, path)).first;

	if (!m_assetBySUID.emplace(assetInfo->identifier, id).second) {
		throw std::runtime_error(aku::buildString("Asset SUID conflict for: ", path.str()));
	}

	if (!assetInfo->source.empty() && !(m_assetBySource.emplace(assetInfo->source, id).second || m_supressWarnings)) {
		akl::Logger("AssetFinder").warn("Asset source conflict for: ", path.str());
	}

	if (!m_assetByDestination.emplace(path, id).second) {
		throw std::runtime_error(aku::buildString("Asset destination conflict for: ", path.str()));
	}

	return true;
}

AssetRegistry::AssetRegistry(const akfs::Path& scanRoot, bool supressWarnings) : m_supressWarnings(supressWarnings), m_scanRoot(scanRoot), m_assetBySUID() {
	rescan();
}

void AssetRegistry::setRoot(const akfs::Path& scanRoot) {
	m_scanRoot = scanRoot; rescan();
}

void AssetRegistry::rescan() {
	m_assetBySUID.clear();
	m_assetByDestination.clear();
	akfs::iterateDirectory(m_scanRoot, std::bind(&AssetRegistry::proccessFile, this, std::placeholders::_1), true);
}

std::optional<std::pair<aka::AssetInfo, akfs::Path>> AssetRegistry::tryGetAssetInfoBySUID(const akd::SUID& suid) const {
	auto iter = m_assetBySUID.find(suid);
	if (iter == m_assetBySUID.end()) return {};
	return {m_assetInfo[iter->second]};
}

std::optional<std::pair<aka::AssetInfo, akfs::Path>> AssetRegistry::tryGetAssetInfoBySource(const akfs::Path& path) const {
	auto iter = m_assetBySource.find(path);
	if (iter == m_assetBySource.end()) return {};
	return {m_assetInfo[iter->second]};
}

std::optional<std::pair<aka::AssetInfo, akfs::Path>> AssetRegistry::tryGetAssetInfoByDestination(const akfs::Path& path) const {
	auto iter = m_assetByDestination.find(path);
	if (iter == m_assetByDestination.end()) return {};
	return {m_assetInfo[iter->second]};
}

