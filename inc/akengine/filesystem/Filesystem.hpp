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

#ifndef AK_FILESYSTEM_FILESYSTEM_HPP_
#define AK_FILESYSTEM_FILESYSTEM_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/filesystem/Path.hpp>
#include <functional>
#include <string>

namespace akfs {

	bool makeDirectory(const akfs::Path& path, bool recursive = true);

	bool exists(const akfs::Path& path);
	bool remove(const akfs::Path& path);
	bool rename(const akfs::Path& src, const akfs::Path& dst, bool overwrite = false);
	bool copy(const akfs::Path& src, const akfs::Path& dst, akSize bufferSize = 4194304);

	int64 modifiedTime(const akfs::Path& path);
	akSize size(const akfs::Path& path);

	bool iterateDirectory(const akfs::Path& path, const std::function<bool(const akfs::Path&, bool)> callback, bool recursive);

	std::string toSystemPath(const akfs::Path& path);

}

#endif
