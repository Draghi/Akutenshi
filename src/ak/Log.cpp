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

#include <ak/Log.hpp>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <utility>

#include <ak/data/PValue.hpp>
#include <ak/engine/Config.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/event/Event.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/thread/DoubleBuffer.hpp>
#include <ak/thread/Spinlock.hpp>

using namespace akl;

static akt::Thread loggingThread("Log");

static akt::Spinlock messageQueueProcessLock;
static akt::DoubleBuffer<std::pair<Level, std::string>> logMessageBuffer;
static Level consoleFilterLevel = Level::Debug;
static Level fileFilterLevel = Level::Debug;

static std::atomic<bool> isRedirrectingStd = false;

static akt::Spinlock logFileLock;
static akfs::CFile logFile;

bool akl::startProcessing(uint64 delayUS) {
	if (loggingThread.isRunning()) return false;

	loggingThread.execute([=]{
		while(!akt::current().isCloseRequested()) {
			processMessageQueue();
			akt::current().sleep(delayUS);
		}
	});

	return true;
}

bool akl::stopProcessing() {
	if (!loggingThread.isRunning()) return false;
	loggingThread.requestClose();
	loggingThread.join();
	return true;
}

bool akl::isProcessing() {
	return loggingThread.isRunning();
}


void akl::processMessageQueue() {
	auto processLock = messageQueueProcessLock.lock();

	logMessageBuffer.swap();
	logMessageBuffer.iterate([&](akSize, const std::pair<Level, std::string>& record){
		if (isConsoleFilterLevelEnabled(record.first)) {
			if (isRedirrectingStd) {
				//@todo Implement
			} else {
				std::cout << record.second << std::flush;
			}
		}

		if (!isFileFilterLevelEnabled(record.first)) return;

		auto scopeLock = logFileLock.lock();
		if (logFile) {
			logFile.write(record.second.c_str(), record.second.size());
			logFile.flush();
		}
	});
}


bool akl::enableFileOutput() {
	auto fileLock = logFileLock.lock();

	auto utc = aku::utcTimestamp();
	std::stringstream filename;
	filename << "data/logs/log_" << std::put_time(&utc.ctime, "%Y%m%d_%H%M%S") << ".txt";

	auto newLogFile = akfs::CFile(filename.str(), akfs::OpenFlags::Out | akfs::OpenFlags::Truncate);
	if (!newLogFile) return false;
	logFile = std::move(newLogFile);

	return true;
}

void akl::disableFileOutput() {
	auto fileLock = logFileLock.lock();
	logFile = akfs::CFile();
}

void akl::setConsoleLevel(Level logLevel) {
	consoleFilterLevel = logLevel;
}

bool akl::isConsoleFilterLevelEnabled(Level logLevel) {
	return static_cast<uint8>(logLevel) <= static_cast<uint8>(consoleFilterLevel);
}

Level akl::getConsoleFilterLevel() {
	return consoleFilterLevel;
}


void akl::setFileLevel(Level logLevel) {
	fileFilterLevel = logLevel;
}

bool akl::isFileFilterLevelEnabled(Level logLevel) {
	return static_cast<uint8>(logLevel) <= static_cast<uint8>(fileFilterLevel);
}

Level akl::getFileFilterLevel() {
	return fileFilterLevel;
}

bool akl::isFilterLevelEnabled(Level logLevel) {
	return isFileFilterLevelEnabled(logLevel) || isConsoleFilterLevelEnabled(logLevel);
}

void akl::captureStandardStreams() {
	//@todo Implement
}

void akl::restoreStandardStreams() {
	//@todo Implement
}



void akl::internal::printMessage(Level logLevel, const std::string& str) {
	logMessageBuffer.push_back(std::make_pair(logLevel, str));
}

static akev::SubscriberID logSInitRegenerateConfigHook = ake::regenerateConfigDispatch().subscribe([](ake::RegenerateConfigEvent& event){
	akd::serialize(event.data()["log"]["consoleLevel"], akl::Level::Debug);
	akd::serialize(event.data()["log"]["fileLevel"],   akl::Level::Debug);
});

static akev::SubscriberID logSInitRegisterConfigHooks = ake::setConfigDispatch().subscribe([](ake::SetConfigEvent& event) {
	akl::Level tmp;
	if (akd::deserialize(tmp, event.data().atOrDef("log").atOrDef("consoleLevel"))) setConsoleLevel(tmp);
	if (akd::deserialize(tmp, event.data().atOrDef("log").atOrDef("fileLevel"))) setFileLevel(tmp);
});


