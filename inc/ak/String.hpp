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

	template<size_t literalLength> constexpr size_t clen(const char(&)[literalLength]) { return literalLength - 1; }


	void split(const std::string& src, const std::vector<std::string>& delims, std::function<void(const std::string&, const std::string&)> out);

	template<typename func_t> void split(const std::string& src, const std::vector<std::string>& delims, const func_t& out) {
		split(src, delims, std::function<void(const std::string&, const std::string&)>(out));
	}

	namespace internal {
		template<typename type_t> void buildString(std::stringstream& sstream, const typename std::enable_if<!(std::is_invocable<type_t>::value || std::is_function<type_t>::value), type_t>::type& val) { sstream << val; }
		template<typename type_t> void buildString(std::stringstream& sstream, const typename std::enable_if<std::is_invocable<type_t>::value || std::is_function<type_t>::value, type_t>::type& val) { sstream << val(); }

	}

	template<typename type_t> std::stringstream& buildString(std::stringstream& sstream, const type_t& val) {
		internal::buildString<type_t>(sstream, val);
		return sstream;
	}

	template<typename type_t, typename... vargs_t> std::stringstream& buildString(std::stringstream& sstream, const type_t& val, const vargs_t&... vargs) {
		internal::buildString<type_t>(sstream, val);
		buildString(sstream, vargs...);
		return sstream;
	}
	template<typename type_t, typename... vargs_t> std::string buildString(const type_t& value, const vargs_t&... vargs) {
		std::stringstream stream;
		return buildString<type_t, vargs_t...>(stream, value, vargs...).str();
	}

	template<typename container_t> bool equalsAny(const std::string& first, const container_t& compare) {
		for(auto iter = compare.begin(); iter != compare.end(); iter++) if (first == *iter) return true;
		return false;
	}

}

#endif
