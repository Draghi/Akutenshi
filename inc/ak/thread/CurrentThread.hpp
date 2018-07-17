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

#ifndef AK_THREAD_CURRENTTHREAD_HPP_
#define AK_THREAD_CURRENTTHREAD_HPP_

#include <functional>
#include <string>
#include <thread>

#include <ak/PrimitiveTypes.hpp>
#include <ak/thread/Thread.hpp>

namespace akt {
	class CurrentThread {
		friend CurrentThread& akt::current();
		private:
			akt::Thread* m_thread;
			std::thread::id m_id;

			CurrentThread(akt::Thread* thread, std::thread::id id);

		public:

			template<typename func_t> bool schedule(const func_t& func) {
				return schedule(std::function<void()>(func));
			}

			bool schedule(const std::function<void()>& func);
			bool update();
			bool setName(const std::string& name);

			bool yield() const;
			bool sleep(int64 microseconds) const;

			bool isCloseRequested() const;

			std::string name() const;
			uint64 id() const;
			std::thread::id threadID() const;
	};
}

#endif
