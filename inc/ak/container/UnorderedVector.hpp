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

#ifndef AK_CONTAINER_UNORDEREDVECTOR_HPP_
#define AK_CONTAINER_UNORDEREDVECTOR_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <vector>

namespace akc {

	template<typename type_t, typename alloc_t = std::allocator<type_t>> class UnorderedVector final {
		private:
			using container_t = std::vector<type_t, alloc_t>;

			std::vector<type_t, alloc_t> m_vec;

		public:
			using value_type = type_t;
			using size_type = typename container_t::size_type;
			using iterator = typename container_t::iterator;
			using const_iterator = typename container_t::const_iterator;
			using reverse_iterator = typename container_t::reverse_iterator;
			using const_reverse_iterator = typename container_t::const_reverse_iterator;

			UnorderedVector() = default;
			UnorderedVector(const UnorderedVector&) = default;
			UnorderedVector(UnorderedVector&& other) : m_vec(std::move(other.m_vec)) {}
			UnorderedVector(const std::vector<type_t, alloc_t>& other) : m_vec(other) {}
			UnorderedVector(std::vector<type_t, alloc_t>&& other) : m_vec(std::move(other)) {}

			type_t& at(size_type index) { return m_vec.at(index); }
			const type_t& at(size_type index) const { return m_vec.at(index); }

			type_t& front() { return m_vec.front(); }
			type_t& back() { return m_vec.back(); }
			const type_t& front() const { return m_vec.front(); }
			const type_t& back() const { return m_vec.back(); }

			void insert(const type_t& val) { m_vec.push_back(val); }
			void insert(type_t&& val) { m_vec.push_back(std::move(val)); }

			void erase(iterator iter) { erase(std::distance(m_vec.begin(), iter)); }
			void erase(const_iterator iter) { erase(std::distance(m_vec.cbegin(), iter)); }
			void erase(reverse_iterator iter) { erase(std::distance(m_vec.rbegin(), iter)); }
			void erase(const_reverse_iterator iter) { erase(std::distance(m_vec.rcbegin(), iter)); }
			void erase(size_type index) {
				if (index+1 != m_vec.size()) m_vec[index] = std::move(m_vec.back());
				m_vec.pop_back();
			}

			iterator begin() { return m_vec.begin(); }
			iterator end() { return m_vec.end(); }
			const_iterator begin() const { return m_vec.cbegin(); }
			const_iterator end() const { return m_vec.end(); }
			const_iterator cbegin() const { return m_vec.cbegin(); }
			const_iterator cend() const { return m_vec.cend(); }

			reverse_iterator rbegin() { return m_vec.rbegin(); }
			reverse_iterator rend() { return m_vec.rend(); }
			const_reverse_iterator rbegin() const { return m_vec.crbegin(); }
			const_reverse_iterator rend() const { return m_vec.crend(); }
			const_reverse_iterator crbegin() const { return m_vec.crbegin(); }
			const_reverse_iterator crend() const { return m_vec.crend(); }

			void shrink_to_fit() { m_vec.shrink_to_fit(); }
			bool empty() const { return m_vec.empty(); }
			size_type size() const { return m_vec.size(); }
			size_type capacity() const { return m_vec.capacity(); }

			type_t& operator[](size_type index) { return m_vec[index]; }
			const type_t& operator[](size_type index) const { return m_vec[index]; }

			UnorderedVector& operator=(const std::vector<type_t, alloc_t>& other) { m_vec = other; }
			UnorderedVector& operator=(std::vector<type_t, alloc_t>&& other) { m_vec = std::move(other); }

			UnorderedVector& operator=(const UnorderedVector& other) = default;
			UnorderedVector& operator=(UnorderedVector&& other) { m_vec = std::move(other.m_vec); return *this; }


	};

}

#endif
