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

#include <ak/data/Path.hpp>
#include <ak/String.hpp>
#include <algorithm>
#include <cctype>
#include <stdexcept>

using namespace ak::data;

Path ak::data::parseObjectDotNotation(const std::string& path) {
	if (path.size() <= 0) return Path();

	Path result;

	bool parsingIndex = false;
	ak::split(path, {".", "["}, [&](const std::string& delimStr, const std::string& str) {

		if (parsingIndex) {
			if (delimStr.empty()) {
				throw std::invalid_argument("ak::data::parsePath: Incomplete array index");;
			}

			auto pos = str.find_first_of(']');
			if (pos == std::string::npos) {
				throw std::invalid_argument("ak::data::parsePath: Invalid array index");
			}

			std::string indexStr = str.substr(0, pos);
			if (!std::all_of(indexStr.begin(), indexStr.end(), ::isdigit)) {
				throw std::invalid_argument("ak::data::parsePath: Invalid array index");
			}

			result.append(std::stoull(indexStr));

			parsingIndex = false;
		} else {
			result.append(str);
		}

		if (delimStr == "[") parsingIndex = true;
	});

	return result;
}
