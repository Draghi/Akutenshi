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
#include <ak/thread/Thread.hpp>
#include <stddef.h>

using namespace ak::thread;

static uint64 getNextID();
static Thread*& currentThreadPtr();
 
Thread::Thread()
	: m_name("Thread"),
	m_id(getNextID()),
	m_lock(),
	m_thread(),
	m_closeRequested(false),
	m_runLock() {}

Thread::Thread(const std::string& name)
	: m_name(name),
	  m_id(getNextID()),
	  m_lock(),
	  m_thread(), 
	  m_closeRequested(false), 
	  m_runLock() {}

Thread::~Thread() { requestClose(); waitFor(); }

ak::ScopeGuard Thread::performThreadStartup() {
	currentThreadPtr() = this;
	return ak::ScopeGuard([this] {
		m_runLock.execute();
		m_closeRequested = false;
		delete &current();
	});
}

void Thread::schedule(const std::function<void()>& func) {
	m_scheduledCallbacks.push_back(func);
}

bool Thread::update() {
	if (currentThreadPtr() != this) return false;

	auto lock = m_updateLock.tryLock();
	if (lock.empty()) return false;

	m_scheduledCallbacks.swap();
	m_scheduledCallbacks.iterate([](size_t /*i*/, auto& callback){callback();});
	m_scheduledCallbacks.clear();

	return true;
}

void Thread::setName(const std::string& name) {
	m_name = name;
}

Thread& Thread::requestClose() {
	m_closeRequested = true;
	return *this;
}

Thread& Thread::cancelClose() {
	m_closeRequested = false;
	return *this;
}

bool Thread::waitFor() {
	if (!m_thread.joinable()) {
		return const_cast<const Thread*>(this)->waitFor();
	}

	m_thread.join();
	return true;
}

bool Thread::waitFor() const {
	while (isRunning()) {
		std::this_thread::yield();
	}
	return true;
}

bool Thread::isCloseRequested() const {
	return m_closeRequested;
}

bool Thread::isRunning() const {
	return !m_runLock.empty();
}

bool Thread::isCurrent() const {
	return ak::thread::current().id() == id();
}

const std::string& Thread::name() const {
	return m_name;
}

uint64 Thread::id() const {
	return m_id;
}

std::thread::id Thread::threadID() const {
	return m_thread.get_id();
}

// ///////////////////////// //
// // Current Thread & ID // //
// ///////////////////////// //

static uint64 getNextID() {
	static std::atomic<uint64> CurrentUID = 0;
	return ++CurrentUID;
}

static Thread*& currentThreadPtr() {
	thread_local Thread* thread = nullptr;
	return thread;
}

ak::thread::CurrentThread& ak::thread::current() {
	if (::currentThreadPtr() == nullptr) ::currentThreadPtr() = new Thread();
	thread_local CurrentThread* currentThread = new CurrentThread(*::currentThreadPtr());
	return *currentThread;
}

