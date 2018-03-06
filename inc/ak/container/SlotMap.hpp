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

#ifndef AK_CONTAINER_SLOTMAP_HPP_
#define AK_CONTAINER_SLOTMAP_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <stddef.h>
#include <deque>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace akc {

	struct SlotID final {
		uint32 index;
		uint32 generation;

		SlotID() : index(0), generation(0) {}
		SlotID(uint64 value) : index(value >> 32u), generation(value & 0xFFFFFFFFu) {}
		SlotID(uint32 indexVal, uint32 generationVal) : index(indexVal), generation(generationVal) {}
		SlotID(const SlotID& other) = default;
		SlotID(SlotID&& other) = default;

		uint64 value() const { return static_cast<uint64>(index) << 32u | generation; }
		bool isValid() const { return generation != 0; }

		bool operator<(const SlotID& other)  const { return value() <  other.value(); }
		bool operator>(const SlotID& other)  const { return value() >  other.value(); }
		bool operator==(const SlotID& other) const { return value() == other.value(); }

		bool operator<=(const SlotID& other) const { return value() <= other.value(); }
		bool operator>=(const SlotID& other) const { return value() >= other.value(); }
		bool operator!=(const SlotID& other) const { return value() != other.value(); }

		SlotID& operator=(const SlotID& other) = default;
		SlotID& operator=(SlotID&& other) = default;

		operator bool() const { return isValid(); }
	};
	static_assert(sizeof(SlotID) == sizeof(uint64), "SlotMapIndex isn't the correct size");
	static_assert(std::is_trivially_copyable<SlotID>(), "SlotMapIndex isn't trivially copyable");

	// @todo Consider using the unordered vector class for the data. It'd just make the code a little cleaner.
	template<typename type_t, typename alloc_t = std::allocator<type_t>> class SlotMap final {
		public:
			using value_type = type_t;
			using allocator_type = alloc_t;
			template<typename value_t> using container_type = std::vector<value_t, alloc_t>;
			using iterator = typename container_type<type_t>::iterator;
			using const_iterator = typename container_type<type_t>::const_iterator;
			using reverse_iterator = typename container_type<type_t>::reverse_iterator;
			using const_reverse_iterator = typename container_type<type_t>::const_reverse_iterator;

		private:
			std::vector<SlotID, alloc_t> m_indicies;
			container_type<type_t> m_data;
			std::vector<uint32, alloc_t> m_freeList;
			std::vector<uint32, alloc_t> m_indexLookup;

			void removeEntry(uint32 indexID) {

				auto removeLastEntry = [&]{
					m_indicies[indexID].generation++;
					m_freeList.push_back(indexID);
					m_data.pop_back();
					m_indexLookup.pop_back();
				};

				switch(m_data.size()) {
					case 0: throw std::logic_error("SlotMap: Attempted to remove entry when map contains no data.");

					case 1: {
						removeLastEntry();
					} return;

					default: {
						auto dataID = m_indicies[indexID].index;

						if (dataID + 1 == m_data.size()) { removeLastEntry(); return; }

						auto endIndexID = m_indexLookup.back();

						// Invalidate Index
						m_indicies[indexID].generation++;

						// Move Data
						m_data[dataID] = std::move(m_data.back());
						m_data.pop_back();

						// Update Index
						m_indicies[endIndexID].index = dataID;

						// Update Lookup
						m_indexLookup[dataID] = endIndexID;
						m_indexLookup.pop_back();

						// Free Index
						m_freeList.push_back(indexID);
					}
				}
			}

		public:
			SlotMap() = default;
			~SlotMap() = default;

			// //////////////////// //
			// // Insert Element // //
			// //////////////////// //

			std::pair<SlotID, iterator> insert(const type_t& val) {
				m_data.push_back(val);
				if (m_freeList.empty()) {
					m_indicies.push_back(SlotID(m_data.size() - 1, 1));
					m_indexLookup.push_back(m_indicies.size() - 1);
				} else {
					auto freeId = m_freeList.back(); m_freeList.pop_back();
					m_indicies[freeId].index = m_data.size() - 1;
					m_indexLookup.push_back(freeId);
				}
				return {slotIDFor(std::prev(m_data.end())), std::prev(m_data.end())};
			}

			std::pair<SlotID, iterator> insert(type_t&& val) {
				m_data.push_back(std::move(val));
				if (m_freeList.empty()) {
					m_indicies.push_back(SlotID(m_data.size() - 1, 1));
					m_indexLookup.push_back(m_indicies.size() - 1);
				} else {
					auto freeId = m_freeList.back(); m_freeList.pop_back();
					m_indicies[freeId].index = m_data.size() - 1;
					m_indexLookup.push_back(freeId);
				}
				return {slotIDFor(std::prev(m_data.end())), std::prev(m_data.end())};
			}

			// ///////////////////// //
			// // Remove Elements // //
			// ///////////////////// //

			bool erase(SlotID id) {
				if (!exists(id)) return false;
				removeEntry(id.index);
				return true;
			}

			bool erase(const iterator& iter) {
				auto id = std::distance(m_data.begin(), iter);
				removeEntry(m_indexLookup[id]);
				return true;
			}

			bool erase(const const_iterator& iter) {
				auto id = std::distance(m_data.cbegin(), iter);
				removeEntry(m_indexLookup[id]);
				return true;
			}

			bool erase(const reverse_iterator& iter) {
				auto id = std::distance(m_data.rbegin(), iter);
				removeEntry(m_indexLookup[id]);
				return true;
			}

			bool erase(const const_reverse_iterator& iter) {
				auto id = std::distance(m_data.crbegin(), iter);
				removeEntry(m_indexLookup[id]);
				return true;
			}

			void clear() {
				m_data.clear();
				for(auto iter = m_indexLookup.begin(); iter != m_indexLookup.end(); iter++) {
					m_indicies[*iter].generation++;
					m_freeList.push_back(*iter);
				}
				m_indexLookup.clear();
			}

			void reset() {
				m_data.clear();
				m_indicies.clear();
				m_freeList.clear();
				m_indexLookup.clear();
			}

			// /////////// //
			// // Query // //
			// /////////// //

			bool exists(SlotID id) const { return (id.index < m_indicies.size()) && (m_indicies[id.index].generation == id.generation); }

			iterator find(SlotID id) {
				if (!exists(id)) return m_data.end();
				return m_data.begin() + m_indicies[id.index].index;
			}

			const_iterator find(SlotID id) const {
				if (!exists(id)) return m_data.end();
				return m_data.begin() + m_indicies[id.index].index;
			}

			// //////////// //
			// // SlotID // //
			// /////////// //

			SlotID slotIDFor(akSize id) const {
				if (id >= m_data.size()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return SlotID(m_indexLookup[id], m_indicies[m_indexLookup[id]].generation);
			}

			SlotID slotIDFor(const iterator& iter) const {
				if (iter == const_cast<container_type<type_t>&>(m_data).end()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return slotIDFor(std::distance(const_cast<container_type<type_t>&>(m_data).begin(), iter));
			}

			SlotID slotIDFor(const const_iterator& iter) const {
				if (iter == m_data.end()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return slotIDFor(std::distance(m_data.begin(), iter));
			}

			SlotID slotIDFor(const reverse_iterator& iter) const {
				if (iter == const_cast<container_type<type_t>&>(m_data).rend()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return slotIDFor(std::distance(const_cast<container_type<type_t>&>(m_data).rbegin(), iter));
			}

			SlotID slotIDFor(const const_reverse_iterator& iter) const {
				if (iter == m_data.rend()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return slotIDFor(std::distance(m_data.rbegin(), iter));
			}

			// /////////// //
			// // Entry // //
			// ////////// //

			akSize entryIDFor(SlotID id) const {
				if (!exists(id)) throw std::out_of_range("SlotMap: Attempt to index out of bounds.");
				return m_indicies[id].index;
			}

			akSize entryIDFor(const iterator& iter) const {
				if (iter == const_cast<container_type<type_t>&>(m_data).end()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return m_indicies[std::distance(const_cast<container_type<type_t>&>(m_data).begin(), iter)].index;
			}

			akSize entryIDFor(const const_iterator& iter) const {
				if (iter == m_data.end()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return m_indicies[std::distance(m_data.begin(), iter)].index;
			}

			akSize entryIDFor(const reverse_iterator& iter) const {
				if (iter == const_cast<container_type<type_t>&>(m_data).rend()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return m_indicies[std::distance(const_cast<container_type<type_t>&>(m_data).rbegin(), iter)].index;
			}

			akSize entryIDFor(const const_reverse_iterator& iter) const {
				if (iter == m_data.rend()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return m_indicies[std::distance(m_data.rbegin(), iter)].index;
			}

			// ////////////// //
			// // Iterator // //
			// ////////////// //

			iterator begin() { return m_data.begin(); }
			const_iterator begin() const { return m_data.begin(); }

			iterator end()   { return m_data.end();   }
			const_iterator end()   const { return m_data.end();   }

			reverse_iterator rbegin() { return m_data.rbegin(); }
			const_reverse_iterator rbegin() const { return m_data.rbegin(); }

			reverse_iterator rend()   { return m_data.rend(); }
			const_reverse_iterator rend()   const { return m_data.rend();   }


			// /////////////////// //
			// // Slot Indexing // //
			// /////////////////// //

			type_t& at(SlotID id) {
				if (!exists(id)) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return m_data[m_indicies[id.index].index];
			}

			const type_t& at(SlotID id) const {
				if (!exists(id)) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return m_data[m_indicies[id.index].index];
			}

			type_t& operator[](SlotID id) { return at(id); }
			const type_t& operator[](SlotID id) const { return at(id); }

			// ///////////////////// //
			// // Direct Indexing // //
			// ///////////////////// //

			type_t& at(akSize id) { return m_data[id]; }
			const type_t& at(akSize id) const { return m_data[id]; }

			type_t& operator[](akSize id) { return at(id); }
			const type_t& operator[](akSize id) const { return at(id); }

			// /////////////////// //
			// // Direct Access // //
			// /////////////////// //

			type_t* data() { return m_data.data(); }
			const type_t* data() const { return m_data.data(); }

			// //////////////// //
			// // Properties // //
			// //////////////// //

			bool empty() const { return m_data.size() > 0; }
			akSize size() const { return m_data.size(); }
			size_t capacity() const { return m_data.capacity(); }
	};

}

namespace std {
	template<> struct hash<akc::SlotID> {
		size_t operator()(const akc::SlotID id) const { return id.value(); }
	};
}

#endif
