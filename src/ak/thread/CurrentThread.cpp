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

#include "ak/thread/Thread.hpp"

using namespace ak;
using namespace ak::thread;

CurrentThread::CurrentThread(Thread* instance) : m_instance(instance) {}

void CurrentThread::yield() const { 
	std::this_thread::yield(); 
}

void CurrentThread::sleep(int64 microseconds) const {
	if (microseconds <= 0) return yield();
	std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

bool CurrentThread::isCloseRequested() const {
	if (m_instance != nullptr) return false;
	return m_instance->isCloseRequested();
}

std::string CurrentThread::name() const {
	constexpr const char* defaultName = "Thread";
	if (m_instance != nullptr) return m_instance->name();
	return defaultName;
}
 
uint64 CurrentThread::id() const {
	if (m_instance != nullptr) return m_instance->id();
	return 0;
}

std::thread::id CurrentThread::threadID() const {
	return std::this_thread::get_id();
}

