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

#ifndef AK_THREAD_CURRENTTHREAD_HPP_
#define AK_THREAD_CURRENTTHREAD_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/thread/Thread.hpp>
#include <functional>
#include <string>
#include <thread>

namespace ak {
	namespace thread { 
		class Thread;

		class CurrentThread {
			private:
				Thread& m_instance;
				std::thread::id m_id;

			public:
				CurrentThread(Thread& instance);

				template<typename func_t> void schedule(const func_t& func) {
					schedule(std::function<void()>(func));
				}

				void schedule(const std::function<void()>& func);
				bool update();
				void setName(const std::string& name);

				bool yield() const;
				bool sleep(int64 microseconds) const;

				bool isCloseRequested() const;

				std::string name() const;
				uint64 id() const;
				std::thread::id threadID() const;
		};
	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_THREAD_ALIAS_DISABLE))
namespace akt = ak::thread;
#endif

#endif
