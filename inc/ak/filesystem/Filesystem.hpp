/**
 * Copyright 2017 Michael J. Baker
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

#ifndef AK_FILESYSTEM_FILESYSTEM_HPP_
#define AK_FILESYSTEM_FILESYSTEM_HPP_

#include <ak/data/PValue.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <stx/Filesystem.hpp>
#include <array>
#include <optional>

namespace akfs {
	class CFile;
}

namespace akfs {

	enum class SystemFolder : uint8 {
		none,

		appData,
		appConfig,
		appCache,

		userDesktop,
		userDocuments,
		userDownloads,
		userPictures,
		userVideos,
		userMusic,

		userData,
		userConfig,
		userSaveGames,

		localData,
		localConfig,
		localCache,

		// ///////////////////////// //
		// // Special Directories // //
		// ///////////////////////// //
		searchData,
		searchConfig,
		searchCache,
	};

	constexpr uint8 SYSTEM_FOLDER_ENUM_COUNT = static_cast<uint8>(SystemFolder::localCache) + 1;

	void overrideFolder(SystemFolder folder, const stx::filesystem::path& path);
	void resetFolder(SystemFolder folder);

	std::optional<stx::filesystem::path> resolveFolder(SystemFolder folder);
	akfs::CFile open(SystemFolder folder, const stx::filesystem::path& path, uint8 openFlags);

	void serializeFolders(akd::PValue& root);
	void deserializeFolders(const akd::PValue& root);

	std::optional<stx::filesystem::path> removeBasePath(const stx::filesystem::path& base, const stx::filesystem::path& val);
}

#endif
