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

#include <thread>
#include "ak/PrimitiveTypes.hpp"

namespace ak {
	namespace thread { 
		class Thread;

		class CurrentThread {
			private:
				Thread* m_instance;

			public:
				CurrentThread(Thread* instance);

				void yield() const;
				void sleep(int64 microseconds) const;

				bool isCloseRequested() const;

				std::string name() const;
				uint64 id() const;
				std::thread::id threadID() const;
		};
	}
}

#endif
