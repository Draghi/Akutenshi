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

#ifndef AKCOMMON_OBJECTPOOL_HPP_
#define AKCOMMON_OBJECTPOOL_HPP_

#include <akcommon/DynamicBitset.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <crtdefs.h>
#include <algorithm>
#include <deque>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace akc {
	template<typename type_t, typename size_t = akSize> class ObjectPool final {
		ObjectPool(const ObjectPool&) = delete;
		ObjectPool& operator=(const ObjectPool&) = delete;

		public:
			using value_type = type_t;
			using  size_type = size_t;

		private:
			using storage_block = typename std::aligned_union<0, value_type>::type;

			akc::DynamicBitset     m_allocated;
			std::deque<storage_block> m_values;
			std::vector<size_type>  m_freelist;

			size_type newAt(size_type index, const type_t& val);
			size_type newAt(size_type index, type_t&& val);
			template<typename... vargs_t> size_type newAt(size_type index, vargs_t&&... val);
			void deleteAt(size_type index);

			size_type allocateFreeIndex();

			void shrinkTo(size_type newSize);
			void growTo(size_type newSize);

		public:
			ObjectPool();
			~ObjectPool();

			size_type insert(const type_t& val);
			size_type insert(type_t&& val);
			template<typename... vargs_t> size_type emplace(vargs_t&&... vargs);

			type_t& at(size_t index);
			const type_t& at(size_t index) const;

			type_t* tryAt(size_t index);
			const type_t* tryAt(size_t index) const;

			template<typename func_t> void visit(const func_t& callback);
			template<typename func_t> void visit(const func_t& callback) const;

			bool erase(size_type index);

			void trim();
			void resize(size_type maxSize);
			void reserve(size_type count);

			bool isAllocated(size_type index) const;

			size_type allocated() const;
			size_type unallocated() const;
			size_type capacity() const;

			type_t& operator[](size_type index);
			const type_t& operator[](size_type index) const;
	};
}

namespace akc {
	template<typename type_t, typename size_t> ObjectPool<type_t, size_t>::ObjectPool() = default;

	template<typename type_t, typename size_t> ObjectPool<type_t, size_t>::~ObjectPool() {
		for(size_type i = 0; i < m_values.size(); i++) if (m_allocated.at(i)) deleteAt(i);
	}



	template<typename type_t, typename size_t> typename ObjectPool<type_t, size_t>::size_type ObjectPool<type_t, size_t>::insert(const type_t& val) {
		return newAt(allocateFreeIndex(), val);
	}

	template<typename type_t, typename size_t> typename ObjectPool<type_t, size_t>::size_type ObjectPool<type_t, size_t>::insert(type_t&& val) {
		return newAt(allocateFreeIndex(), std::move(val));
	}

	template<typename type_t, typename size_t> template<typename... vargs_t> typename ObjectPool<type_t, size_t>::size_type ObjectPool<type_t, size_t>::emplace(vargs_t&&... vargs) {
		return newAt<vargs_t...>(allocateFreeIndex(), std::forward<vargs_t>(vargs)...);
	}



	template<typename type_t, typename size_t> type_t& ObjectPool<type_t, size_t>::at(size_type index) {
		if (!isAllocated(index)) throw std::out_of_range("Attempt to access unallocated object.");
		return reinterpret_cast<type_t&>(m_values.at(index));
	}

	template<typename type_t, typename size_t> const type_t& ObjectPool<type_t, size_t>::at(size_type index) const {
		if (!isAllocated(index)) throw std::out_of_range("Attempt to access unallocated object.");
		return reinterpret_cast<const type_t&>(m_values.at(index));
	}



	template<typename type_t, typename size_t> type_t* ObjectPool<type_t, size_t>::tryAt(size_type index) {
		if (!isAllocated(index)) return nullptr;
		return &m_values[index];
	}

	template<typename type_t, typename size_t> const type_t* ObjectPool<type_t, size_t>::tryAt(size_type index) const {
		if (!isAllocated(index)) return nullptr;
		return &m_values[index];
	}



	template<typename type_t, typename size_t> template<typename func_t> void ObjectPool<type_t, size_t>::visit(const func_t& callback) {
		for(auto i = 0; i < m_values.size(); i++) {
			if (!isAllocated(i)) continue;
			if (!callback(i, m_values[i])) return;
		}
	}

	template<typename type_t, typename size_t> template<typename func_t> void ObjectPool<type_t, size_t>::visit(const func_t& callback) const {
		for(auto i = 0; i < m_values.size(); i++) {
			if (!isAllocated(i)) continue;
			if (!callback(i, m_values[i])) return;
		}
	}



	template<typename type_t, typename size_t> bool ObjectPool<type_t, size_t>::erase(size_type index) {
		if (!isAllocated(index)) return false;
		m_allocated.flip(index);
		m_freelist.push_back(index);
		deleteAt(index);
		return true;
	}



	template<typename type_t, typename size_t> void ObjectPool<type_t, size_t>::trim() {
		size_type maxSize = m_allocated.length();

		std::vector<size_type> newFreelist(m_freelist);
		bool shrunk = true;
		while(shrunk) {
			shrunk = false;
			for(auto iter : newFreelist) {
				if (*iter + 1 != maxSize) continue;
				iter = newFreelist.erase(iter);
				shrunk = true;
				maxSize--;
			}
		}
		m_freelist = std::move(newFreelist);

		shrinkTo(maxSize);
	}

	template<typename type_t, typename size_t> void ObjectPool<type_t, size_t>::resize(size_type maxSize) {
		if (maxSize < m_values.size()) shrinkTo(maxSize);
		else growTo(maxSize);
	}

	template<typename type_t, typename size_t> void ObjectPool<type_t, size_t>::reserve(size_type count) {
		if (count <= unallocated()) return;
		growTo(capacity() + (count - unallocated()));
	}



	template<typename type_t, typename size_t> bool ObjectPool<type_t, size_t>::isAllocated(size_type index) const {
		return index >= m_allocated.length() ? false : m_allocated.at(index);
	}



	template<typename type_t, typename size_t> typename ObjectPool<type_t, size_t>::size_type ObjectPool<type_t, size_t>::allocated() const {
		return capacity() - unallocated();
	}

	template<typename type_t, typename size_t> typename ObjectPool<type_t, size_t>::size_type ObjectPool<type_t, size_t>::unallocated() const {
		return m_freelist.size();
	}

	template<typename type_t, typename size_t> typename ObjectPool<type_t, size_t>::size_type ObjectPool<type_t, size_t>::capacity() const {
		return m_values.size();
	}



	template<typename type_t, typename size_t> type_t& ObjectPool<type_t, size_t>::operator[](size_type index) {
		return at(index);
	}

	template<typename type_t, typename size_t> const type_t& ObjectPool<type_t, size_t>::operator[](size_type index) const {
		return at(index);
	}
}



namespace akc {
	template<typename type_t, typename size_t> typename ObjectPool<type_t, size_t>::size_type ObjectPool<type_t, size_t>::newAt(size_type index, const type_t& val) {
		new (static_cast<void*>(&m_values.at(index))) type_t(val);
		return index;
	}

	template<typename type_t, typename size_t> typename ObjectPool<type_t, size_t>::size_type ObjectPool<type_t, size_t>::newAt(size_type index, type_t&& val) {
		new (static_cast<void*>(&m_values.at(index))) type_t(std::move(val));
		return index;
	}

	template<typename type_t, typename size_t> template<typename... vargs_t> typename ObjectPool<type_t, size_t>::size_type ObjectPool<type_t, size_t>::newAt(size_type index, vargs_t&&... val) {
		new (static_cast<void*>(&m_values.at(index))) type_t(std::forward<vargs_t>(val)...);
		return index;
	}

	template<typename type_t, typename size_t> void ObjectPool<type_t, size_t>::deleteAt(size_type index) {
		reinterpret_cast<type_t&>(m_values[index]).~type_t();
	}

	template<typename type_t, typename size_t> typename ObjectPool<type_t, size_t>::size_type ObjectPool<type_t, size_t>::allocateFreeIndex() {
		if (m_freelist.empty()) {
			m_values.emplace(m_values.end());
			m_allocated.setLength(m_values.size());
			m_allocated.set(m_values.size() - 1, true);
			return m_values.size() - 1;
		} else {
			auto index = m_freelist.back();
			m_freelist.pop_back();
			m_allocated.flip(index);
			return index;
		}
	}

	template<typename type_t, typename size_t> void ObjectPool<type_t, size_t>::shrinkTo(size_type newSize) {
		if (newSize >= m_values.size()) return;
		for(auto iter : m_freelist) if (*iter + 1 > newSize) iter = m_freelist.erase(iter);

		while(m_values.size() > newSize) {
			if (m_allocated.at(m_values.size() - 1)) deleteAt(m_values.size() - 1);
			m_values.pop_back();
		}

		m_allocated.setLength(newSize);
	}

	template<typename type_t, typename size_t> void ObjectPool<type_t, size_t>::growTo(size_type newSize) {
		if (newSize >= m_values.size()) return;
		while(m_values.size() < newSize) m_values.emplace(m_values.end());
		m_allocated.setLength(m_values.size());
	}
}


#endif /* AKCOMMON_OBJECTPOOL_HPP_ */
