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

#ifndef AK_THREAD_SPINLOCK_HPP_
#define AK_THREAD_SPINLOCK_HPP_

#include <AkCommon/ScopeGuard.hpp>
#include <atomic>
#include <thread>

namespace akt {

	class Spinlock final {
		private:
			static constexpr auto LOCK_MEMORY_ORDER = std::memory_order_acq_rel;

			mutable std::atomic<bool> m_lock;
			mutable std::thread::id m_lastAcquiredThread;

		protected:
			void unlock() const {
				m_lock.exchange(false, LOCK_MEMORY_ORDER);
			}

		public:
			Spinlock() : m_lock(false), m_lastAcquiredThread(std::this_thread::get_id()) {}

			ak::ScopeGuard tryLock() const {
				if (m_lock.exchange(true, LOCK_MEMORY_ORDER)) return ak::ScopeGuard();
				m_lastAcquiredThread = std::this_thread::get_id();
				return [&](){unlock();};
			}

			ak::ScopeGuard lock() const {
				ak::ScopeGuard result;
				while((result = tryLock()).empty()) std::this_thread::yield();
				return result;
			}

			std::thread::id lastAcquiredThread() const {
				return m_lastAcquiredThread;
			}

			bool wasLastAcquiredByThisThread() const {
				return m_lastAcquiredThread == std::this_thread::get_id();
			}
	};


}

#endif
