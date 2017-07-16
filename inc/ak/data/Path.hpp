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

#ifndef AK_DATA_PATH_HPP_
#define AK_DATA_PATH_HPP_

#include <stddef.h>
#include <iterator>
#include <string>
#include <vector>

#include "ak/PrimitiveTypes.hpp"

namespace ak {
	namespace data {

		struct PathEntry {
			bool isIndex;
			std::string path;
			uint64 index;
		};

		class Path final {
			private:
				std::vector<PathEntry> m_path;

				using diff_t = typename std::vector<PathEntry>::difference_type;

			public:
				Path() = default;

				Path& append(const std::string& path) {
					m_path.push_back(PathEntry{false, path, 0});
					return *this;
				}

				Path& append(uint64 index) {
					m_path.push_back(PathEntry{true, std::string(), index});
					return *this;
				}

				Path& append(PathEntry entry) {
					m_path.push_back(entry);
					return *this;
				}

				Path& pop(uint64 count = 1) {
					for(uint64 i = 0; i < count; i++) m_path.pop_back();
					return *this;
				}

				Path& remove(diff_t index) {
					m_path.erase(m_path.begin() + index);
					return *this;
				}

				template<typename index_f, typename path_f> void forEach(index_f indexCB, path_f pathCB) const {
					for(size_t i = 0; i < m_path.size(); i++) {
						auto e = entry(i);
						if (e.isIndex) {
							indexCB(e.index);
						} else {
							pathCB(e.path);
						}
					}
				}

				PathEntry& entry(uint64 index) { return m_path[index]; }
				PathEntry entry(uint64 index) const { return m_path[index]; }

				std::string entryAsString(uint64 index) const {
					auto& e = m_path[index];
					return (e.isIndex) ? std::to_string(e.index) : e.path;
				}

				PathEntry& operator[](uint64 index) { return entry(index); }
				PathEntry operator[](uint64 index) const { return entry(index); }

				Path& operator<<(const std::string& path) { return append(path); }
				Path& operator<<(uint64 index) { return append(index); }
				Path& operator<<(PathEntry entry) { return append(entry); }

				size_t size() const { return m_path.size(); }
				bool empty() const { return m_path.size() <= 0; }

		};

		Path parseObjectDotNotation(const std::string& path);

	}
}

#endif
