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

#ifndef AK_THREAD_THREAD_HPP_
#define AK_THREAD_THREAD_HPP_

#include <atomic>
#include <thread>
#include <string>

#include "ak/ScopeGuard.hpp"
#include "ak/thread/Spinlock.hpp"
#include "ak/thread/CurrentThread.hpp"

namespace ak {

	namespace thread {
		class CurrentThread;

		class Thread final {
			private:
				Thread(const Thread&) = delete;
				Thread& operator=(const Thread&) = delete;

				std::string m_name;
				uint64 m_id;
				ak::thread::Spinlock m_lock;
				std::thread m_thread;
				std::atomic<bool> m_closeRequested;
				ak::ScopeGuard m_runLock;

				ak::ScopeGuard performThreadStartup();

			public:
				Thread();
				Thread(const std::string& name);
				~Thread();

				template<typename func_t> bool execute(const func_t& func) {
					auto runningLock = m_lock.tryLock();
					if (runningLock.empty()) return false;

					m_closeRequested = false;
					m_runLock = std::move(runningLock);

					m_thread = std::thread([this, func]() {
						auto threadCleanup = performThreadStartup();
						func();
					});

					return true;
				}

				Thread& requestClose();
				Thread& cancelClose();

				bool waitFor();
				bool waitFor() const;

				bool isCloseRequested() const;
				bool isRunning() const;
				bool isCurrent() const;

				const std::string& name() const;
				uint64 id() const;
				std::thread::id threadID() const;
		};

		CurrentThread& current();
	}
}
#endif
