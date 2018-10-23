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

#ifndef AK_DATA_PVPATH_HPP_
#define AK_DATA_PVPATH_HPP_

#include <AkCommon/PrimitiveTypes.hpp>
#include <iterator>
#include <string>
#include <vector>

namespace akd {
	struct PathEntry {
		bool isIndex;
		std::string path;
		uint64 index;
	};

	class PVPath final {
		private:
			std::vector<PathEntry> m_path;

			using diff_t = typename std::vector<PathEntry>::difference_type;

		public:
			PVPath() = default;

			PVPath& append(const std::string& path) {
				m_path.push_back(PathEntry{false, path, 0});
				return *this;
			}

			PVPath& append(uint64 index) {
				m_path.push_back(PathEntry{true, std::string(), index});
				return *this;
			}

			PVPath& append(PathEntry entry) {
				m_path.push_back(entry);
				return *this;
			}

			PVPath& append(const PVPath& entry) {
				for(auto iter = entry.m_path.begin(); iter != entry.m_path.end(); iter++) {
					m_path.push_back(*iter);
				}
				return *this;
			}

			PVPath& pop(uint64 count = 1) {
				for(uint64 i = 0; i < count; i++) m_path.pop_back();
				return *this;
			}

			PVPath& remove(diff_t index) {
				m_path.erase(m_path.begin() + index);
				return *this;
			}

			PVPath& clear() {
				m_path.clear();
				return *this;
			}

			template<typename index_f, typename path_f> void forEach(index_f indexCB, path_f pathCB) const {
				for(akSize i = 0; i < m_path.size(); i++) {
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

			PVPath& operator<<(const std::string& path) { return append(path); }
			PVPath& operator<<(uint64 index) { return append(index); }
			PVPath& operator<<(PathEntry entry) { return append(entry); }

			akSize size() const { return m_path.size(); }
			bool empty() const { return m_path.size() <= 0; }

	};

	PVPath parseObjectDotNotation(const std::string& path);
	std::string pathToObjectDotNotation(const PVPath& path);
}

#endif
