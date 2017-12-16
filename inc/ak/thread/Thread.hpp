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

#include <ak/thread/DoubleBuffer.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/thread/Spinlock.hpp>
#include <algorithm>
#include <atomic>
#include <functional>
#include <string>
#include <thread>

namespace akt {
	class CurrentThread;
	CurrentThread& current();

	class Thread final {
		friend CurrentThread& current();
		private:
			Thread(const Thread&) = delete;
			Thread& operator=(const Thread&) = delete;

			std::string m_name;
			uint64 m_id;
			std::thread::id m_threadId;
			akt::Spinlock m_lock;
			std::thread m_thread;
			std::atomic<bool> m_closeRequested;
			std::atomic<bool> m_runLock;

			akt::DoubleBuffer<std::function<void()>> m_scheduledCallbacks;
			akt::Spinlock m_updateLock;

			ak::ScopeGuard performThreadStartup();

		public:
			Thread();
			Thread(const std::string& name);
			~Thread();

			template<typename func_t> bool execute(const func_t& callback) {
				if (m_runLock.exchange(true)) return false;

				m_closeRequested = false;
				m_thread = std::thread([this, callback]() {
					auto threadCleanup = performThreadStartup();
					callback();
				});

				return true;
			}

			template<typename func_t> void schedule(const func_t& func) {
				schedule(std::function<void(Thread&)>(func));
			}

			void schedule(const std::function<void()>& func);
			bool update();

			void setName(const std::string& name);

			Thread& requestClose();
			Thread& cancelClose();

			bool detach();
			bool join();

			bool waitFor() const;

			bool isCloseRequested() const;
			bool isRunning() const;
			bool isCurrent() const;

			const std::string& name() const;
			uint64 id() const;
			std::thread::id threadID() const;
	};

}

#endif
