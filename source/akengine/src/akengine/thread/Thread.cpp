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

#include <akengine/thread/CurrentThread.hpp>
#include <akengine/thread/Thread.hpp>

using namespace akt;

static uint64 getNextID();
static Thread*& currentThreadPtr();
 
Thread::Thread()
	: m_name("Thread"),
	m_id(getNextID()),
	m_threadId(),
	m_lock(),
	m_thread(),
	m_closeRequested(false),
	m_runLock(false) {}

Thread::Thread(const std::string& name)
	: m_name(name),
	  m_id(getNextID()),
	  m_threadId(),
	  m_lock(),
	  m_thread(), 
	  m_closeRequested(false), 
	  m_runLock(false) {}

Thread::~Thread() {
	requestClose();
	if (!join()) waitFor();
}

ak::ScopeGuard Thread::performThreadStartup() {
	currentThreadPtr() = this;
	return ak::ScopeGuard([this] {
		m_runLock = false;
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
	m_scheduledCallbacks.iterate([](akSize /*i*/, auto& callback){callback();});
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

bool Thread::detach() {
	if (m_thread.joinable()) m_thread.detach();
	return true;
}

bool Thread::join() {
	if (!m_thread.joinable()) return false;
	m_thread.join();
	return true;
}

bool Thread::waitFor() const {
	while (isRunning()) { std::this_thread::yield(); }
	return true;
}

bool Thread::isCloseRequested() const {
	return m_closeRequested;
}

bool Thread::isRunning() const {
	return m_runLock;
}

bool Thread::isCurrent() const {
	return akt::current().id() == id();
}

const std::string& Thread::name() const {
	return m_name;
}

uint64 Thread::id() const {
	return m_id;
}

std::thread::id Thread::threadID() const {
	if (m_threadId != std::thread::id()) return m_threadId;
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

akt::CurrentThread& akt::current() {
	if (::currentThreadPtr() == nullptr) ::currentThreadPtr() = new Thread();
	thread_local CurrentThread* currentThread = new CurrentThread(::currentThreadPtr(), std::this_thread::get_id());
	return *currentThread;
}

