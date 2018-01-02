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

#include <ak/String.hpp>

#include <stddef.h>
#include <tuple>


static bool containsString(const std::string& src, const std::string& substr, akSize start);
static std::tuple<bool, akSize> searchForDelim(const std::string& src, const std::vector<std::string>& delims, akSize pos);

void ak::split(const std::string& src, const std::vector<std::string>& delims, std::function<void(const std::string&, const std::string&)> out) {
	if (src.size() <= 0) return;

	akSize pos = 0;
	for(akSize i = 0; i < src.size(); i++) {
		auto [foundDelim, delim] = searchForDelim(src, delims, i);

		if (foundDelim) {
			out(delims[delim], src.substr(pos, i - pos));
			pos = i + 1;
		}
	}

	out("", src.substr(pos));
}

static bool containsString(const std::string& src, const std::string& substr, akSize start) {
	if (substr.empty()) return src.empty();

	size_t i = 0;
	for(; ((start + i) < src.size()) && (i < substr.size()); i++) {
		if (src[start + i] != substr[i]) {
			return false;
		}
	}

	return ((start + i) < src.size()) || ((src.size() - start) == substr.size());
}

static std::tuple<bool, akSize> searchForDelim(const std::string& src, const std::vector<std::string>& delims, akSize pos) {
	bool foundDelim = false;
	akSize delim = 0;

	for(akSize j = 0; j < delims.size(); j++) {
		auto replaceCurrentDelim = (!foundDelim) || (delims[j].size() < delims[delim].size());
		if ((replaceCurrentDelim) && (containsString(src, delims[j], pos))) {
			foundDelim = true;
			delim = j;
			if (delims[j].size() == 1) break;
		}
	}

	return std::make_tuple(foundDelim, delim);
}


