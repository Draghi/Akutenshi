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

static std::optional<stx::filesystem::path>& systemFolder(SystemFolder folder) {
	return systemFolders()[static_cast<size_t>(folder)];
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
	root["appData"].trySet<std::string>(systemFolder(SystemFolder::appData));
	root["appConfig"].trySet<std::string>(systemFolder(SystemFolder::appConfig));
	root["appCache"].trySet<std::string>(systemFolder(SystemFolder::appCache));

	root["userDesktop"].trySet<std::string>(systemFolder(SystemFolder::userDesktop));
	root["userDocuments"].trySet<std::string>(systemFolder(SystemFolder::userDocuments));
	root["userDownloads"].trySet<std::string>(systemFolder(SystemFolder::userDownloads));
	root["userPictures"].trySet<std::string>(systemFolder(SystemFolder::userPictures));
	root["userVideos"].trySet<std::string>(systemFolder(SystemFolder::userVideos));
	root["userMusic"].trySet<std::string>(systemFolder(SystemFolder::userMusic));

	root["userData"].trySet<std::string>(systemFolder(SystemFolder::userData));
	root["userConfig"].trySet<std::string>(systemFolder(SystemFolder::userConfig));
	root["userSaveGames"].trySet<std::string>(systemFolder(SystemFolder::userSaveGames));

	root["localData"].trySet<std::string>(systemFolder(SystemFolder::localData));
	root["localConfig"].trySet<std::string>(systemFolder(SystemFolder::localConfig));
	root["localCache"].trySet<std::string>(systemFolder(SystemFolder::localCache));
}

void ak::filesystem::deserializeFolders(const ak::data::PValue& root) {
	root.atOrDef("appData").tryAssign<std::string>(systemFolder(SystemFolder::appData));
	root.atOrDef("appConfig").tryAssign<std::string>(systemFolder(SystemFolder::appConfig));
	root.atOrDef("appCache").tryAssign<std::string>(systemFolder(SystemFolder::appCache));

	root.atOrDef("userDesktop").tryAssign<std::string>(systemFolder(SystemFolder::userDesktop));
	root.atOrDef("userDocuments").tryAssign<std::string>(systemFolder(SystemFolder::userDocuments));
	root.atOrDef("userDownloads").tryAssign<std::string>(systemFolder(SystemFolder::userDownloads));
	root.atOrDef("userPictures").tryAssign<std::string>(systemFolder(SystemFolder::userPictures));
	root.atOrDef("userVideos").tryAssign<std::string>(systemFolder(SystemFolder::userVideos));
	root.atOrDef("userMusic").tryAssign<std::string>(systemFolder(SystemFolder::userMusic));

	root.atOrDef("userData").tryAssign<std::string>(systemFolder(SystemFolder::userData));
	root.atOrDef("userConfig").tryAssign<std::string>(systemFolder(SystemFolder::userConfig));
	root.atOrDef("userSaveGames").tryAssign<std::string>(systemFolder(SystemFolder::userSaveGames));

	root.atOrDef("localData").tryAssign<std::string>(systemFolder(SystemFolder::localData));
	root.atOrDef("localConfig").tryAssign<std::string>(systemFolder(SystemFolder::localData));
	root.atOrDef("localCache").tryAssign<std::string>(systemFolder(SystemFolder::localData));
}


