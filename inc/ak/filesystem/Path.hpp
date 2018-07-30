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

#ifndef AK_FILESYSTEM_PATH_HPP_
#define AK_FILESYSTEM_PATH_HPP_

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <ak/data/PValue.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/String.hpp>

namespace akfs {

	class Path final {
		private:
			std::vector<std::string> m_pathTokens;

			std::vector<std::string> tokenize(const std::string& path) {
				std::vector<std::string> result;
				result.reserve(std::count(path.begin(), path.end(), '/'));
				ak::split(path, {{"/"}}, [&](const std::string& delim, const std::string& substr){ if (substr.size() > 0) result.push_back(substr + delim); });
				return result;
			}

		public:
			Path() : m_pathTokens() {}
			Path(const std::string& path) : m_pathTokens(tokenize(path)) {}
			Path(const char* path) : m_pathTokens(tokenize(std::string(path))) {}


			Path& pop_front() { m_pathTokens.erase(m_pathTokens.begin()); return *this; }
			Path& pop_back() { m_pathTokens.pop_back(); return *this; }
			std::string front() const { return m_pathTokens.front(); }
			std::string back() const { return m_pathTokens.back(); }
			Path& erase(akSize i) { m_pathTokens.erase(m_pathTokens.begin() + i); return *this; }
			std::string at(akSize i) const { return m_pathTokens[i]; }
			akSize size() const { return m_pathTokens.size(); }
			bool empty() const { return m_pathTokens.empty(); }

			Path relativeTo(const Path& base) const {
				akSize diffIndex = 0;
				for(; (diffIndex < m_pathTokens.size()) && (diffIndex < base.m_pathTokens.size()); diffIndex++) {
					if (m_pathTokens[diffIndex] != base.m_pathTokens[diffIndex]) break;
				}

				Path result;
				result.m_pathTokens.reserve(m_pathTokens.size() - diffIndex);
				result.m_pathTokens.insert(result.m_pathTokens.end(), m_pathTokens.begin() + diffIndex, m_pathTokens.end());
				return result;
			}

			Path& setExtension(const std::string& ext) {
				if (!isFile()) return *this;
				clearExtension();
				if (ext.size() <= 0) return *this;
				if (ext.front() != '.') m_pathTokens.back() += '.';
				m_pathTokens.back() += ext;
				return *this;
			}

			Path& clearExtension() {
				if (!isFile()) return *this;
				auto index = m_pathTokens.back().find_last_of('.');
				if (index != std::string::npos) m_pathTokens.back() = m_pathTokens.back().substr(0, index);
				return *this;
			}

			std::string extension() const {
				if (!isFile()) return "";
				auto index = m_pathTokens.back().find_last_of('.');
				if (index == std::string::npos) return "";
				return m_pathTokens.back().substr(index);
			}

			std::string filename() const {
				if (!isFile()) return "";
				return m_pathTokens.back();
			}

			Path parent() const {
				Path result(*this);
				if (result.m_pathTokens.size()) result.m_pathTokens.pop_back();
				return result;
			}

			bool isDirectory() const { return m_pathTokens.empty() ? true : m_pathTokens.back().back() == '/'; }
			bool isFile() const { return !isDirectory(); }

			std::string str() const {
				std::stringstream sstream;
				for(const auto& path : m_pathTokens) sstream << path;
				return sstream.str();
			}

			Path& operator/=(const Path& path) { return *this += path; }
			Path& operator+=(const Path& path) {
				m_pathTokens.reserve(path.m_pathTokens.size());
				if (m_pathTokens.back().back() != '/') m_pathTokens.back() += '/';
				m_pathTokens.insert(m_pathTokens.end(), path.m_pathTokens.begin(), path.m_pathTokens.end());
				return *this;
			}

			Path operator/(const Path& path) const { return Path(*this) /= path; }
			Path operator+(const Path& path) const { return Path(*this) += path; }

			bool operator==(const Path& path) const {
				return (m_pathTokens.size() == path.m_pathTokens.size()) && std::equal(m_pathTokens.begin(), m_pathTokens.end(), path.m_pathTokens.begin(), path.m_pathTokens.end());
			}

			bool operator!=(const Path& path) const {
				return (m_pathTokens.size() != path.m_pathTokens.size()) || !std::equal(m_pathTokens.begin(), m_pathTokens.end(), path.m_pathTokens.begin(), path.m_pathTokens.end());
			}

			bool operator< (const Path& path) const {
				for(akSize i = 0; (i < m_pathTokens.size()) && (i < path.m_pathTokens.size()); i++) if (m_pathTokens[i] < path.m_pathTokens[i]) return true;
				return m_pathTokens.size() < path.m_pathTokens.size();
			}

			bool operator> (const Path& path) const {
				for(akSize i = 0; (i < m_pathTokens.size()) && (i < path.m_pathTokens.size()); i++) if (m_pathTokens[i] > path.m_pathTokens[i]) return true;
				return m_pathTokens.size() > path.m_pathTokens.size();
			}

			bool operator<=(const Path& path) const {
				for(akSize i = 0; (i < m_pathTokens.size()) && (i < path.m_pathTokens.size()); i++) if (m_pathTokens[i] <= path.m_pathTokens[i]) return true;
				return m_pathTokens.size() <= path.m_pathTokens.size();
			}

			bool operator>=(const Path& path) const {
				for(akSize i = 0; (i < m_pathTokens.size()) && (i < path.m_pathTokens.size()); i++) if (m_pathTokens[i] >= path.m_pathTokens[i]) return true;
				return m_pathTokens.size() >= path.m_pathTokens.size();
			}
	};

}

namespace akd {

	inline void serialize(akd::PValue& dst, const akfs::Path& src) {
		dst.setStr(src.str());
	}

	inline bool deserialize(akfs::Path& dst, const akd::PValue& src) {
		if (!src.isStr()) return false;
		dst = akfs::Path(src.getStr());
		return true;
	}

}

namespace std {
	template<> struct hash<akfs::Path> {
		using argument_type = akfs::Path;
		using result_type = std::hash<std::string>::result_type;

		hash() = default;
		result_type operator()(const argument_type& s) const noexcept { return std::hash<std::string>()(s.str()); }
	};
}

#endif
