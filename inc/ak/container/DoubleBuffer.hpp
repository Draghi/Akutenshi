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

#ifndef AK_CONTAINER_DOUBLEBUFFER_HPP_
#define AK_CONTAINER_DOUBLEBUFFER_HPP_

#include <vector>
#include <ak/thread/Spinlock.hpp>

namespace ak {
	namespace container {

		template<typename type_t> class DoubleBuffer final {
			private:
				ak::thread::Spinlock m_writeLock;
				ak::thread::Spinlock m_readLock;

				uint8 m_index;
				std::vector<type_t> m_buffers[2];

			public:
				DoubleBuffer() = default;
				~DoubleBuffer() = default;

				void swap() {
					auto writeLock = m_writeLock.lock();
					auto readLock = m_readLock.lock();
					m_index ^= 0x01;
					m_buffers[m_index].clear();
				}


				void push_back(const type_t& val) {
					auto writeLock = m_writeLock.lock();
					m_buffers[m_index].push_back(val);
				}

				void push_back(type_t&& val) {
					auto writeLock = m_writeLock.lock();
					m_buffers[m_index].push_back(std::move(val));
				}


				type_t& front() {
					auto readLock = m_readLock.lock();
					return m_buffers[m_index ^ 0x01].front();
				}

				type_t& back() {
					auto readLock = m_readLock.lock();
					return m_buffers[m_index ^ 0x01].back();
				}

				type_t& at(size_t index) {
					auto readLock = m_readLock.lock();
					return m_buffers[m_index ^ 0x01][index];
				}

				template<typename func_t> void iterate(const func_t& callback, size_t startIndex = 0, ssize_t count = -1) {
					auto readLock = m_readLock.lock();

					size_t totalCount = (count < 0) ? m_buffers[m_index ^ 0x01].size() : static_cast<size_t>(count);
					for(size_t i = startIndex; i < totalCount; i++) {
						callback(i, m_buffers[m_index ^ 0x01][i]);
					}
				}

				void clear() {
					auto readLock = m_readLock.lock();
					m_buffers[m_index ^ 0x01] = std::vector<type_t>(m_buffers[m_index ^ 0x01].size());
				}

				size_t size() {
					auto readLock = m_readLock.lock();
					return m_buffers[m_index ^ 0x01].size();
				}

				bool empty() {
					auto writeLock = m_writeLock.lock();
					auto readLock = m_readLock.lock();
					return (m_buffers[0].size() <= 0) || (m_buffers[1].size() <= 0);
 				}

		};

	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_CONTAINER_ALIAS_DISABLE))
namespace akc = ak::container;
#endif

#endif
