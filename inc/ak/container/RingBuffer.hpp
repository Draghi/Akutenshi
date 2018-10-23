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

#ifndef AK_CONTAINER_RINGBUFFER_HPP_
#define AK_CONTAINER_RINGBUFFER_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/thread/Spinlock.hpp>
#include <ak/util/Memory.hpp>
#include <algorithm>
#include <atomic>
#include <vector>

namespace akc {

	template<typename type_t> class RingBuffer final {
		private:
			std::vector<type_t> m_data;
			akSize m_capacity;

			std::atomic<akSize> m_readOffset, m_writeOffset;

			static akSize distanceBetween(akSize from, akSize to, akSize size) {
				from = from % size; to = to % size;
				return (from <= to) ? to - from : (size - from) + to;
			}

			void moveReadOffset(akSSize count) {
				m_readOffset.fetch_add(count);
			}

			void moveWriteOffset(akSize count) {
				akSize remainingDist = remaining();
				if (remainingDist < count) moveReadOffset(count - remainingDist); // @TODO Threading issue? Start read, move read & start write. Could write catch up to read? Unlikely, unless cores have unbalanced loads.
				m_writeOffset.fetch_add(count);
			}

		public:
			RingBuffer(akSize capacity) : m_data(capacity, type_t()), m_capacity(capacity), m_readOffset(0), m_writeOffset(0) {}

			RingBuffer(const RingBuffer& o) {
				m_data = o.m_data;
				m_capacity = o.m_capacity;
				m_readOffset.exchange(o.m_readOffset);
				m_writeOffset.exchange(o.m_writeOffset);
			}

			RingBuffer& operator=(const RingBuffer& o) {
				m_data = o.m_data;
				m_capacity = o.m_capacity;
				m_readOffset.exchange(o.m_readOffset);
				m_writeOffset.exchange(o.m_writeOffset);
				return *this;
			}

			void write(const type_t* src, akSize count) {
				aku::memwrp(m_data.data(), m_capacity, m_writeOffset, src, count);
				moveWriteOffset(count);
			}

			akSize read(type_t* dst, akSize count) {
				count = std::min(count, size());
				aku::memwrp(dst, m_data.data(), m_capacity, m_readOffset, count);
				moveReadOffset(count);
				return count;
			}

			akSize peek(type_t& dst, akSize count) const {
				count = std::min(count, size());
				aku::memwrp(dst, m_data.data(), m_capacity, m_readOffset, count);
				return count;
			}

			akSize remaining() const {
				return m_capacity - distanceBetween(m_readOffset, m_writeOffset, m_capacity);
			}

			akSize size() const {
				return distanceBetween(m_readOffset, m_writeOffset, m_capacity);
			}

			akSize capacity() const {
				return m_capacity;
			}
	};

}



#endif /* AK_CONTAINER_RINGBUFFER_HPP_ */
