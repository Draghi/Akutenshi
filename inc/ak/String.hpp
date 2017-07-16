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

#ifndef AK_STRING_HPP_
#define AK_STRING_HPP_

#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace ak {
	void split(const std::string& src, const std::vector<std::string>& delims, std::function<void(const std::string&, const std::string&)> out);

	template<typename func_t> void split(const std::string& src, const std::vector<std::string>& delims, const func_t& out) {
		split(src, delims, std::function<void(const std::string&, const std::string&)>(out));
	}

	template<typename type_t> std::string buildString(std::stringstream& stream, const type_t& value) {
		stream << value;
		return stream.str();
	}

	template<typename type_t, typename... vargs_t> std::string buildString(std::stringstream& stream, const type_t& value, const vargs_t&... vargs) {
		stream << value;
		return buildString<vargs_t...>(stream, vargs...);
	}

	template<typename type_t, typename... vargs_t> std::string buildString(const type_t& value, const vargs_t&... vargs) {
		std::stringstream stream;
		stream << value;
		return buildString<vargs_t...>(stream, vargs...);
	}
}

#endif
