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

#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <stx/Filesystem.hpp>
#include <cstdlib>
#include <optional>

using namespace ak::filesystem;

#ifdef __linux__

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

static stx::filesystem::path getHomeDirectory() {
	char* homeEnv = getenv("HOME");
	if ((homeEnv) && (homeEnv[0]!='\0')) return stx::filesystem::path(homeEnv);

	struct passwd *pw = getpwuid(getuid());
	return stx::filesystem::path(pw->pw_dir);
}

static std::optional<stx::filesystem::path> resolveSystemPath(SystemFolder folder) {
	switch(folder) {
		case SystemFolder::appData: return stx::filesystem::path("./data");
		case SystemFolder::appConfig: return stx::filesystem::path("./config");
		case SystemFolder::appCache: return stx::filesystem::path("./cache");

		case SystemFolder::userDesktop: {
			char* envDir = getenv("XDG_DESKTOP_DIR");
			if ((envDir) && (envDir[0]!='\0')) return stx::filesystem::path(envDir);
			return getHomeDirectory()/"Desktop";
		}

		case SystemFolder::userDocuments: {
			char* envDir = getenv("XDG_DOCUMENTS_DIR");
			if ((envDir) && (envDir[0]!='\0')) return stx::filesystem::path(envDir);
			return getHomeDirectory()/"Documents";
		}

		case SystemFolder::userDownloads: {
			char* envDir = getenv("XDG_DOWNLOAD_DIR");
			if ((envDir) && (envDir[0]!='\0')) return stx::filesystem::path(envDir);
			return getHomeDirectory()/"Downloads";
		}

		case SystemFolder::userPictures: {
			char* envDir = getenv("XDG_PICTURES_DIR");
			if ((envDir) && (envDir[0]!='\0')) return stx::filesystem::path(envDir);
			return getHomeDirectory()/"Pictures";
		}

		case SystemFolder::userVideos: {
			char* envDir = getenv("XDG_VIDEOS_DIR");
			if ((envDir) && (envDir[0]!='\0')) return stx::filesystem::path(envDir);
			return getHomeDirectory()/"Videos";
		}

		case SystemFolder::userMusic: {
			char* envDir = getenv("XDG_MUSIC_DIR");
			if ((envDir) && (envDir[0]!='\0')) return stx::filesystem::path(envDir);
			return getHomeDirectory()/"Music";
		}


		case SystemFolder::localData:
		case SystemFolder::userSaveGames:
		case SystemFolder::userData: {
			char* envDir = getenv("XDG_DATA_HOME");
			if ((envDir) && (envDir[0]!='\0')) return stx::filesystem::path(envDir);
			return getHomeDirectory()/".local"/"share";
		}

		case SystemFolder::localConfig:
		case SystemFolder::userConfig: {
			char* envDir = getenv("XDG_CONFIG_HOME");
			if ((envDir) && (envDir[0]!='\0')) return stx::filesystem::path(envDir);
			return getHomeDirectory()/".config";
		}

		case SystemFolder::localCache: {
			char* envDir = getenv("XDG_CACHE_HOME");
			if ((envDir) && (envDir[0]!='\0')) return stx::filesystem::path(envDir);
			return getHomeDirectory()/".cache";
		}

		case SystemFolder::searchCache:
		case SystemFolder::searchConfig:
		case SystemFolder::searchData:
		default: {
			return std::optional<stx::filesystem::path>();
		}
	}
}

#elif defined(_WIN32)

static std::optional<stx::filesystem::path> resolveSystemPath(SystemFolder folder) {
	return std::optional<stx::filesystem::path>();
}

#else
#	warning "Timer unsupported"
#endif

static std::array<std::optional<stx::filesystem::path>, SYSTEM_FOLDER_ENUM_COUNT>& systemFolders() {
	static std::array<std::optional<stx::filesystem::path>, SYSTEM_FOLDER_ENUM_COUNT>* systemFolders = []{
		auto* ptr = new std::array<std::optional<stx::filesystem::path>, SYSTEM_FOLDER_ENUM_COUNT>();
		for(size_t i = 0; i < ptr->size(); i++) (*ptr)[i] = resolveSystemPath(static_cast<SystemFolder>(i));
		return ptr;
	}();
	return *systemFolders;
}


void ak::filesystem::overrideFolder(SystemFolder folder, const stx::filesystem::path& path) {
	auto index = static_cast<uint8>(folder);
	if (index >= systemFolders().size()) return;
	systemFolders()[index] = path;
}

void ak::filesystem::resetFolder(SystemFolder folder) {
	auto index = static_cast<uint8>(folder);
	if (index >= systemFolders().size()) return;
	systemFolders()[index] = resolveSystemPath(folder);
}

std::optional<stx::filesystem::path> ak::filesystem::resolveFolder(SystemFolder folder) {
	auto index = static_cast<uint8>(folder);
	if (index >= systemFolders().size()) return std::optional<stx::filesystem::path>();
	return systemFolders()[index];
}

ak::filesystem::CFile ak::filesystem::open(SystemFolder folder, const stx::filesystem::path& path, uint8 openFlags) {
	auto index = static_cast<uint8>(folder);
	if (index < systemFolders().size()) return ak::filesystem::CFile(resolveFolder(folder).value()/path, openFlags);

	switch(folder) {
		case SystemFolder::searchCache: {

			ak::filesystem::CFile file;
			uint8 searchFlags = openFlags | OpenFlags::NoCreate;
			for(size_t i = 0; i < 2; i++) {
				file = ak::filesystem::CFile(resolveFolder(SystemFolder::localCache).value()/path, searchFlags);
				if (file) return file;

				file = ak::filesystem::CFile(resolveFolder(SystemFolder::appCache).value()/path, searchFlags);
				if (file) return file;

				searchFlags = openFlags;
			}

			return ak::filesystem::CFile();
		}

		case SystemFolder::searchConfig: {

			ak::filesystem::CFile file;
			uint8 searchFlags = openFlags | OpenFlags::NoCreate;
			for(size_t i = 0; i < 2; i++) {
				file = ak::filesystem::CFile(resolveFolder(SystemFolder::userConfig).value()/path, searchFlags);
				if (file) return file;

				file = ak::filesystem::CFile(resolveFolder(SystemFolder::localConfig).value()/path, searchFlags);
				if (file) return file;

				file = ak::filesystem::CFile(resolveFolder(SystemFolder::appConfig).value()/path, searchFlags);
				if (file) return file;

				searchFlags = openFlags;
			}

			return ak::filesystem::CFile();
		}

		case SystemFolder::searchData: {

			ak::filesystem::CFile file;
			uint8 searchFlags = openFlags | OpenFlags::NoCreate;
			for(size_t i = 0; i < 2; i++) {
				file = ak::filesystem::CFile(resolveFolder(SystemFolder::userData).value()/path, searchFlags);
				if (file) return file;

				file = ak::filesystem::CFile(resolveFolder(SystemFolder::localData).value()/path, searchFlags);
				if (file) return file;

				file = ak::filesystem::CFile(resolveFolder(SystemFolder::appData).value()/path, searchFlags);
				if (file) return file;

				searchFlags = openFlags;
			}

			return ak::filesystem::CFile();
		}

		case SystemFolder::appData:
		case SystemFolder::appConfig:
		case SystemFolder::appCache:
		case SystemFolder::userDesktop:
		case SystemFolder::userDocuments:
		case SystemFolder::userDownloads:
		case SystemFolder::userPictures:
		case SystemFolder::userVideos:
		case SystemFolder::userMusic:
		case SystemFolder::userData:
		case SystemFolder::userConfig:
		case SystemFolder::userSaveGames:
		case SystemFolder::localData:
		case SystemFolder::localConfig:
		case SystemFolder::localCache:
		default:
			return ak::filesystem::CFile();
	}
}

void ak::filesystem::serializeFolders(ak::data::PValue& root) {
	root["appData"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::appData)]);
	root["appConfig"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::appConfig)]);
	root["appCache"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::appCache)]);

	root["userDesktop"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::userDesktop)]);
	root["userDocuments"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::userDocuments)]);
	root["userDownloads"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::userDownloads)]);
	root["userPictures"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::userPictures)]);
	root["userVideos"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::userVideos)]);
	root["userMusic"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::userMusic)]);

	root["userData"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::userData)]);
	root["userConfig"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::userConfig)]);
	root["userSaveGames"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::userSaveGames)]);

	root["localData"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::localData)]);
	root["localConfig"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::localConfig)]);
	root["localCache"].setOptionalString(systemFolders()[static_cast<size_t>(SystemFolder::localCache)]);
}

void ak::filesystem::deserializeFolders(const ak::data::PValue& root) {

	const ak::data::PValue* valueNode = nullptr;

	if (((valueNode = root.tryNavigate("appData")) != nullptr)   && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::appData)]   = root["appData"].stringValue();
	if (((valueNode = root.tryNavigate("appConfig")) != nullptr) && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::appConfig)] = root["appConfig"].stringValue();
	if (((valueNode = root.tryNavigate("appCache")) != nullptr)  && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::appCache)]  = root["appCache"].stringValue();

	if (((valueNode = root.tryNavigate("userDesktop")) != nullptr)   && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::userDesktop)]   = root["userDesktop"].stringValue();
	if (((valueNode = root.tryNavigate("userDocuments")) != nullptr) && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::userDocuments)] = root["userDocuments"].stringValue();
	if (((valueNode = root.tryNavigate("userDownloads")) != nullptr) && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::userDownloads)] = root["userDownloads"].stringValue();
	if (((valueNode = root.tryNavigate("userPictures")) != nullptr)  && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::userPictures)]  = root["userPictures"].stringValue();
	if (((valueNode = root.tryNavigate("userVideos")) != nullptr)    && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::userVideos)]    = root["userVideos"].stringValue();
	if (((valueNode = root.tryNavigate("userMusic")) != nullptr)     && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::userMusic)]     = root["userMusic"].stringValue();

	if (((valueNode = root.tryNavigate("userData")) != nullptr)      && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::userData)]      = root["userData"].stringValue();
	if (((valueNode = root.tryNavigate("userConfig")) != nullptr)    && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::userConfig)]    = root["userConfig"].stringValue();
	if (((valueNode = root.tryNavigate("userSaveGames")) != nullptr) && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::userSaveGames)] = root["userSaveGames"].stringValue();

	if (((valueNode = root.tryNavigate("localData")) != nullptr)   && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::localData)]   = root["localData"].stringValue();
	if (((valueNode = root.tryNavigate("localConfig")) != nullptr) && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::localConfig)] = root["localConfig"].stringValue();
	if (((valueNode = root.tryNavigate("localCache")) != nullptr)  && valueNode->isString()) systemFolders()[static_cast<size_t>(SystemFolder::localCache)]  = root["localCache"].stringValue();
}


