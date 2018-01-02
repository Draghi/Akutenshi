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

#ifndef AK_FILESYSTEM_PATH_HPP_
#define AK_FILESYSTEM_PATH_HPP_

#include <stddef.h>
#include <experimental/filesystem>
#include <optional>
#include <string>

namespace akfs {

	inline std::optional<stx::filesystem::path> pathDifference(const stx::filesystem::path& left, const stx::filesystem::path& right) {
		auto lStr = left.native(); auto rStr = right.native();
		size_t pos = rStr.find_first_of(lStr);
		if (pos == std::string::npos) return std::optional<stx::filesystem::path>();
		return rStr.substr(pos + lStr.size());
	}

}

#endif
