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

#ifndef AK_COMMON_SLOTMAP_HPP_
#define AK_COMMON_SLOTMAP_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/Traits.hpp>
#include <akcommon/UnorderedVector.hpp>
#include <akengine/debug/Log.hpp>
#include <climits>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace akc {

	template<akSize l> struct SlotID_t final {
		using storage_type = typename aku::traits::IntegerMinBytes<l>::type;
		using value_type   = typename aku::traits::IntegerMinBytes<l*2>::type;

		storage_type index;
		storage_type generation;

		SlotID_t() : index(0), generation(0) {}
		SlotID_t(storage_type indexVal, storage_type generationVal) : index(indexVal), generation(generationVal) {}
		SlotID_t(const SlotID_t& other) = default;
		SlotID_t(SlotID_t&& other) = default;

		value_type value() const { return (static_cast<value_type>(index) << (sizeof(storage_type)*CHAR_BIT)) | generation; }
		bool isValid() const { return generation != 0; }

		bool operator<(const SlotID_t& other)  const { return value() <  other.value(); }
		bool operator>(const SlotID_t& other)  const { return value() >  other.value(); }
		bool operator==(const SlotID_t& other) const { return value() == other.value(); }

		bool operator<=(const SlotID_t& other) const { return value() <= other.value(); }
		bool operator>=(const SlotID_t& other) const { return value() >= other.value(); }
		bool operator!=(const SlotID_t& other) const { return value() != other.value(); }

		SlotID_t& operator=(const SlotID_t& other) = default;
		SlotID_t& operator=(SlotID_t&& other) = default;

		operator bool() const { return isValid(); }
	};
	using SlotID = SlotID_t<4>;

	template<akSize l, typename type_t> class SlotMap_t final {
		public:
			using slot_type = SlotID_t<l>;
			using index_type = typename slot_type::value_type;

			using value_type = type_t;
			template<typename value_t> using container_type = akc::UnorderedVector<value_t>;
			using iterator = typename container_type<type_t>::iterator;
			using const_iterator = typename container_type<type_t>::const_iterator;
			using reverse_iterator = typename container_type<type_t>::reverse_iterator;
			using const_reverse_iterator = typename container_type<type_t>::const_reverse_iterator;

		private:
			std::vector<slot_type> m_indicies;
			container_type<type_t> m_data;
			std::vector<index_type> m_freeList;
			std::vector<index_type> m_indexLookup;

			void removeEntry(index_type indexID) {

				auto dataID = m_indicies[indexID].index;
				if (dataID + 1 == m_data.size()) {
					// Invalidate Index
					if (++(m_indicies[indexID].generation) == 0) akl::Logger("SlotMap").warn("generation overflow.");
					m_data.pop_back();             // Remove Data
					m_indexLookup.pop_back();      // Update Lookup
					m_freeList.push_back(indexID); // Free Index
					return;
				}


				// Invalidate Index
				if (++(m_indicies[indexID].generation) == 0) akl::Logger("SlotMap").warn("generation overflow.");


				m_data.erase(dataID); // Remove Data
				auto endIndexID = m_indexLookup.back(); m_indexLookup.pop_back();
				m_indicies[endIndexID].index = dataID; // Update Index
				m_indexLookup[dataID] = endIndexID;    // Update Lookup
				m_freeList.push_back(indexID);         // Free Index
			}

		public:
			// //////////////////// //
			// // Insert Element // //
			// //////////////////// //

			std::pair<slot_type, iterator> insert(const type_t& val) {
				m_data.insert(val);
				if (m_freeList.empty()) {
					m_indicies.push_back(slot_type(m_data.size() - 1, 1));
					m_indexLookup.push_back(m_indicies.size() - 1);
				} else {
					auto freeId = m_freeList.back(); m_freeList.pop_back();
					m_indicies[freeId].index = m_data.size() - 1;
					m_indexLookup.push_back(freeId);
				}
				return {slotIDFor(std::prev(m_data.end())), std::prev(m_data.end())};
			}

			std::pair<slot_type, iterator> insert(type_t&& val) {
				m_data.insert(std::move(val));
				if (m_freeList.empty()) {
					m_indicies.push_back(slot_type(m_data.size() - 1, 1));
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

			bool erase(slot_type id) {
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

			bool exists(slot_type id) const { return (id.index < m_indicies.size()) && (m_indicies[id.index].generation == id.generation); }

			iterator find(slot_type id) {
				if (!exists(id)) return m_data.end();
				return m_data.begin() + m_indicies[id.index].index;
			}

			const_iterator find(slot_type id) const {
				if (!exists(id)) return m_data.end();
				return m_data.begin() + m_indicies[id.index].index;
			}

			// //////////// //
			// // SlotID // //
			// /////////// //

			slot_type slotIDFor(index_type id) const {
				if (id >= m_data.size()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return slot_type(m_indexLookup[id], m_indicies[m_indexLookup[id]].generation);
			}

			slot_type slotIDFor(const iterator& iter) const {
				if (iter == const_cast<container_type<type_t>&>(m_data).end()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return slotIDFor(std::distance(const_cast<container_type<type_t>&>(m_data).begin(), iter));
			}

			slot_type slotIDFor(const const_iterator& iter) const {
				if (iter == m_data.end()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return slotIDFor(std::distance(m_data.begin(), iter));
			}

			slot_type slotIDFor(const reverse_iterator& iter) const {
				if (iter == const_cast<container_type<type_t>&>(m_data).rend()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return slotIDFor(std::distance(const_cast<container_type<type_t>&>(m_data).rbegin(), iter));
			}

			slot_type slotIDFor(const const_reverse_iterator& iter) const {
				if (iter == m_data.rend()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return slotIDFor(std::distance(m_data.rbegin(), iter));
			}

			// /////////// //
			// // Entry // //
			// /////////// //

			index_type entryIDFor(const iterator& iter) const {
				if (iter == const_cast<container_type<type_t>&>(m_data).end()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return m_indicies[std::distance(const_cast<container_type<type_t>&>(m_data).begin(), iter)].index;
			}

			index_type entryIDFor(const const_iterator& iter) const {
				if (iter == m_data.end()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return m_indicies[std::distance(m_data.begin(), iter)].index;
			}

			index_type entryIDFor(const reverse_iterator& iter) const {
				if (iter == const_cast<container_type<type_t>&>(m_data).rend()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return m_indicies[std::distance(const_cast<container_type<type_t>&>(m_data).rbegin(), iter)].index;
			}

			index_type entryIDFor(const const_reverse_iterator& iter) const {
				if (iter == m_data.rend()) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return m_indicies[std::distance(m_data.rbegin(), iter)].index;
			}

			// ////////////// //
			// // Iterator // //
			// ////////////// //

			iterator begin() { return m_data.begin(); }
			const_iterator begin() const { return m_data.begin(); }
			const_iterator cbegin() const { return m_data.cbegin(); }

			iterator end() { return m_data.end();   }
			const_iterator end() const { return m_data.end(); }
			const_iterator cend() const { return m_data.cend(); }

			reverse_iterator rbegin() { return m_data.rbegin(); }
			const_reverse_iterator rbegin() const { return m_data.rbegin(); }
			const_reverse_iterator crbegin() const { return m_data.crbegin(); }

			reverse_iterator rend() { return m_data.rend(); }
			const_reverse_iterator rend() const { return m_data.rend(); }
			const_reverse_iterator crend() const { return m_data.crend(); }


			// /////////////////// //
			// // Slot Indexing // //
			// /////////////////// //

			type_t& at(slot_type id) {
				if (!exists(id)) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return m_data[m_indicies[id.index].index];
			}

			const type_t& at(slot_type id) const {
				if (!exists(id)) throw std::out_of_range("SlotMap: Attempted to index out of bounds.");
				return m_data[m_indicies[id.index].index];
			}

			type_t& operator[](slot_type id) { return at(id); }
			const type_t& operator[](slot_type id) const { return at(id); }

			// ///////////////////// //
			// // Direct Indexing // //
			// ///////////////////// //

			type_t& at(index_type id) { return m_data[id]; }
			const type_t& at(index_type id) const { return m_data[id]; }

			type_t& operator[](index_type id) { return at(id); }
			const type_t& operator[](index_type id) const { return at(id); }

			// /////////////////// //
			// // Direct Access // //
			// /////////////////// //

			type_t* data() { return m_data.data(); }
			const type_t* data() const { return m_data.data(); }

			// //////////////// //
			// // Properties // //
			// //////////////// //

			const container_type<type_t>& dataContainer() const { return m_data; }
			container_type<type_t> copyData() const { return m_data; }

			void shrink_to_fit() {
				m_indicies.shrink_to_fit();
				m_data.shrink_to_fit();
				m_freeList.shrink_to_fit();
				m_indexLookup.shrink_to_fit();
			}

			bool empty() const { return m_data.empty(); }
			index_type size() const { return m_data.size(); }
			size_t capacity() const { return m_data.capacity(); }
	};

	template<typename type_t> using SlotMap = SlotMap_t<4, type_t>;

}

namespace std {
	template<akSize l> struct hash<akc::SlotID_t<l>> {
		size_t operator()(const akc::SlotID_t<l>& id) const { return id.value(); }
	};
}

#endif
