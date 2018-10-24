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

#ifndef AK_ENGINE_DATA_JSON_PVALUEPARSER_HPP_
#define AK_ENGINE_DATA_JSON_PVALUEPARSER_HPP_

#include <akengine/data/PValue.hpp>
#include <akengine/filesystem/Path.hpp>
#include <string>

namespace akd {
	/**
	 * Attempts to serialize a PValue tree as a JSON string
	 * @param src The source PValue tree
	 * @param pretty Should the output be pretty (ie. new lines, tabs etc.)
	 * @return The completed string
	 */
	std::string toJson(const akd::PValue& src, bool pretty = true);

	/**
	 * Attempts to deserialize a JSON string to a PValue tree
	 * @param dest The target PValue to to deserialize to
	 * @param jsonStr The json string to parse
	 * @return If the JSON string was deserialized
	 */
	bool fromJson(akd::PValue& dest, const std::string& jsonStr);

	bool toJsonFile(const akd::PValue& src, const akfs::Path& filepath, bool pretty = true, bool overwrite = true);

	akd::PValue fromJsonFile(const akfs::Path& filepath);
}

#endif
