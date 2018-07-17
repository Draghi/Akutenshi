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



#include <ak/thread/CurrentThread.hpp>

#include <chrono>

#include <ak/thread/Thread.hpp>

using namespace ak;
using namespace akt;

CurrentThread::CurrentThread(akt::Thread* thread, std::thread::id id) : m_thread(thread), m_id(id) {}

bool CurrentThread::schedule(const std::function<void()>& func) {
	if (!m_thread) return false;
	m_thread->schedule(func);
	return true;
}

bool CurrentThread::update() {
	if (!m_thread) return false;
	return m_thread->update();
}

bool CurrentThread::setName(const std::string& name) {
	if (!m_thread) return false;
	m_thread->setName(name);
	return true;
}

bool CurrentThread::yield() const {
	if (m_id != std::this_thread::get_id()) return false;
	std::this_thread::yield(); 
	return true;
}

bool CurrentThread::sleep(int64 microseconds) const {
	if (m_id != std::this_thread::get_id()) return false;
	if (microseconds <= 0) std::this_thread::yield();
	else std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
	return true;
}

bool CurrentThread::isCloseRequested() const {
	if (!m_thread) return true;
	return m_thread->isCloseRequested();
}

std::string CurrentThread::name() const {
	if (!m_thread) return "Unknown";
	return m_thread->name();
}
 
uint64 CurrentThread::id() const {
	if (!m_thread) return 0;
	return m_thread->id();
}

std::thread::id CurrentThread::threadID() const {
	return m_id;
}

