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

#ifndef AK_THREAD_RECURSIVESPINLOCK_HPP_
#define AK_THREAD_RECURSIVESPINLOCK_HPP_

#include "ak/ScopeGuard.hpp"
#include "ak/thread/Spinlock.hpp"

#include <thread>

namespace ak {
	namespace thread {

		class RecursiveSpinlock final {
			private:
				RecursiveSpinlock(const RecursiveSpinlock&) = delete;
				RecursiveSpinlock& operator=(const RecursiveSpinlock&) = delete;

				Spinlock m_spinlock;

				ak::ScopeGuard m_lockGuard;
				std::thread::id m_lockID;
				std::atomic<int> m_lockCount;

			protected:
				ak::ScopeGuard incrementLock() {
					m_lockCount++;
					return [&]{unlock();};
				}

				void unlock() {
					m_lockCount--;
					if (m_lockCount <= 0) {
						m_lockCount = 0;
						m_lockGuard.execute();
					}
				}

			public:
				RecursiveSpinlock() = default;
				~RecursiveSpinlock() = default;

				ak::ScopeGuard tryLock() {
					auto unlockGuard = incrementLock();

					auto lockGuard = m_spinlock.tryLock();
					if (lockGuard.empty()) {
						return (m_lockID == std::this_thread::get_id())
							? std::move(unlockGuard)
							: ak::ScopeGuard();
					}

					m_lockID = std::this_thread::get_id();
					m_lockGuard = std::move(lockGuard);
					return unlockGuard;
				}

				ak::ScopeGuard lock() {
					ak::ScopeGuard unlockGuard;
					while((unlockGuard = tryLock()).empty());
					return unlockGuard;
				}

		};

		using RSpinlock = RecursiveSpinlock;

	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_THREAD_ALIAS_DISABLE))
namespace akt = ak::thread;
#endif

#endif
