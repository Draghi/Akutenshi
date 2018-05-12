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

#include <ak/filesystem/Filesystem.hpp>
#include <ak/filesystem/Path.hpp>
#include <ak/Log.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/String.hpp>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

using namespace akfs;

class VFSMounts final {
	private:
		std::unordered_map<std::string, std::string> m_vfsToSys = {
			{"./", "./"},
			{"data/", "./data/"},
			{"config/", "./config/"},
			{"cache/", "./cache/"}

		};
		std::unordered_map<std::string, std::string> m_sysToVfs;

	public:
		bool mount(std::string mountPoint, std::string systemPath) {
			if (mountPoint.back() != '/') mountPoint += '/';
			if (systemPath.back() != '/') systemPath += '/';

			if (!m_vfsToSys.insert({mountPoint, systemPath}).second) return false;
			if (!m_sysToVfs.insert({systemPath, mountPoint}).second) {
				m_vfsToSys.erase(mountPoint);
				return false;
			}
			return true;
		}

		void unmount(std::string mountPoint) {
			if (mountPoint.back() != '/') mountPoint += '/';

			auto iter = m_vfsToSys.find(mountPoint);
			if (iter == m_vfsToSys.end()) return;
			m_sysToVfs.erase(iter->second);
			m_vfsToSys.erase(iter);
		}

		std::string vfsToSys(std::string vfsPoint) const {
			if (vfsPoint.back() != '/') vfsPoint += '/';

			auto iter = m_vfsToSys.find(vfsPoint);
			if (iter == m_vfsToSys.end()) throw std::logic_error(ak::buildString("Unable to resolve vfsMountPoint: ", vfsPoint));
			return iter->second;
		}
};

static VFSMounts& vfsMounts() {
	static VFSMounts instance;
	return instance;
}

bool akfs::makeDirectory(const akfs::Path& path, bool recursive) {
	if (!recursive) {
		auto sysPath = toSystemPath(path);
		#ifdef __linux__
			::mkdir(sysPath.c_str(), 0777);
		#else
			::mkdir(sysPath.c_str());
		#endif
		return exists(path);
	}

	auto cDir = vfsMounts().vfsToSys(path.front());
	for(akSize i = 1; i < path.size(); i++) {
		cDir.append(path.at(i));
		#ifdef __linux__
			::mkdir(cDir.c_str(), 0777);
		#else
			::mkdir(cDir.c_str());
		#endif
	}
	return exists(path);
}

bool akfs::exists(const akfs::Path& path) {
	auto sysPath = toSystemPath(path);
	struct stat st;
	return (stat(sysPath.c_str(), &st) != 0);
}

bool akfs::remove(const akfs::Path& path) {
	auto sysPath = toSystemPath(path);
	return ::remove(path.str().c_str()) != -1;
}

bool akfs::rename(const akfs::Path& src, const akfs::Path& dst, bool overwrite) {
	auto srcSysPath = toSystemPath(src);
	auto dstSysPath = toSystemPath(dst);
	if (overwrite && exists(dst)) remove(dst);
	return ::rename(srcSysPath.c_str(), dstSysPath.c_str()) != -1;
}

akSize akfs::size(const akfs::Path& path) {
	auto sysPath = toSystemPath(path);
	struct stat st;
	return stat(sysPath.c_str(), &st) == 0 ? static_cast<akSize>(st.st_size) : 0;
}

void akfs::iterateDirectory(const akfs::Path& path, const std::function<void(const akfs::Path&)> callback, bool recursive) {
	auto sysPath = toSystemPath(path);

	auto dir = ::opendir(sysPath.c_str());
	if (!dir) return;
	auto dirGuard = ak::ScopeGuard([&]{::closedir(dir);});

	struct dirent* entry = nullptr;
	while((entry = ::readdir(dir))) {
		auto cPath = path/std::string(entry->d_name);

		if (recursive) {
			struct stat st;
			if (stat(cPath.str().c_str(), &st) != 0) continue;
			if (S_ISDIR(st.st_mode)) {
				iterateDirectory(path/std::string(entry->d_name), callback, true);
			}
		}

		callback(cPath);
	}

}

std::string akfs::toSystemPath(const akfs::Path& path) {
	if (path.empty()) return "";
	return vfsMounts().vfsToSys(path.front()) + Path(path).pop_front().str();
}

