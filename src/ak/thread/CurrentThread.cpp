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


#include <ak/thread/CurrentThread.hpp>
#include <chrono>

using namespace ak;
using namespace ak::thread;

CurrentThread::CurrentThread(Thread& instance) : m_instance(instance) {}

void CurrentThread::schedule(const std::function<void()>& func) {
	m_instance.schedule(func);
}

bool CurrentThread::update() {
	return m_instance.update();
}

void CurrentThread::setName(const std::string& name) {
	m_instance.setName(name);
}

bool CurrentThread::yield() const {
	if (std::this_thread::get_id() != threadID()) return false;
	std::this_thread::yield(); 
	return true;
}

bool CurrentThread::sleep(int64 microseconds) const {
	if (std::this_thread::get_id() != threadID()) return false;
	if (microseconds <= 0) std::this_thread::yield();
	else std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
	return true;
}

bool CurrentThread::isCloseRequested() const {
	return m_instance.isCloseRequested();
}

std::string CurrentThread::name() const {
	return m_instance.name();
}
 
uint64 CurrentThread::id() const {
	return m_instance.id();
}

std::thread::id CurrentThread::threadID() const {
	return m_instance.threadID();
}

