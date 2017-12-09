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

#include <ak/container/DoubleBuffer.hpp>
#include <ak/engine/Config.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/Log.hpp>
#include <ak/ScopeGuard.hpp>
#include <ak/thread/RecursiveSpinlock.hpp>
#include <stddef.h>
#include <algorithm>
#include <atomic>
#include <experimental/filesystem>
#include <iostream>
#include <utility>

using namespace ak::log;

static ak::thread::Thread loggingThread("Log");

static ak::thread::RecursiveSpinlock messageQueueProcessLock;
static ak::container::DoubleBuffer<std::pair<Level, std::string>> logMessageBuffer;
static Level consoleFilterLevel = Level::ALL;
static Level fileFilterLevel = Level::ALL;

static std::atomic<bool> isRedirrectingStd = false;

static ak::thread::RecursiveSpinlock logFileLock;
static ak::filesystem::CFile logFile;

bool ak::log::startProcessing(uint64 delayUS) {
	if (loggingThread.isRunning()) return false;

	loggingThread.execute([=]{
		while(!ak::thread::current().isCloseRequested()) {
			processMessageQueue();
			ak::thread::current().sleep(static_cast<int64>(delayUS));
		}
	});

	return true;
}

bool ak::log::stopProcessing() {
	if (!loggingThread.isRunning()) return false;
	loggingThread.requestClose();
	loggingThread.join();
	return true;
}

bool ak::log::isProcessing() {
	return loggingThread.isRunning();
}


void ak::log::processMessageQueue() {
	auto processLock = messageQueueProcessLock.lock();

	logMessageBuffer.swap();
	logMessageBuffer.iterate([&](size_t, const std::pair<Level, std::string>& record){
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


bool ak::log::enableFileOutput() {
	auto fileLock = logFileLock.lock();

	auto utc = ak::util::utcTimestamp();
	std::stringstream filename;
	filename << "logs/log_" << std::put_time(&utc.ctime, "%Y%m%d_%H%M%S") << ".txt";

	auto newLogFile = ak::filesystem::open(akfs::SystemFolder::appData, filename.str(), ak::filesystem::OpenFlags::Out | ak::filesystem::OpenFlags::Truncate);
	if (!newLogFile) return false;
	logFile = std::move(newLogFile);

	return true;
}

void ak::log::disableFileOutput() {
	auto fileLock = logFileLock.lock();
	logFile = ak::filesystem::CFile();
}

void ak::log::setConsoleFilterLevel(Level logLevel) {
	consoleFilterLevel = logLevel;
}

bool ak::log::isConsoleFilterLevelEnabled(Level logLevel) {
	return static_cast<uint8>(logLevel) <= static_cast<uint8>(consoleFilterLevel);
}

Level ak::log::getConsoleFilterLevel() {
	return consoleFilterLevel;
}


void ak::log::setFileFilterLevel(Level logLevel) {
	fileFilterLevel = logLevel;
}

bool ak::log::isFileFilterLevelEnabled(Level logLevel) {
	return static_cast<uint8>(logLevel) <= static_cast<uint8>(fileFilterLevel);
}

Level ak::log::getFileFilterLevel() {
	return fileFilterLevel;
}

bool ak::log::isFilterLevelEnabled(Level logLevel) {
	return isFileFilterLevelEnabled(logLevel) || isConsoleFilterLevelEnabled(logLevel);
}

void ak::log::captureStandardStreams() {
	//@todo Implement
}

void ak::log::restoreStandardStreams() {
	//@todo Implement
}



void Logger::printMessage(Level logLevel, const std::string& str) {
	logMessageBuffer.push_back(std::make_pair(logLevel, str));
}


static ak::event::SubscriberID logSInitRegenerateConfigHook = ak::engine::regenerateConfigDispatch().subscribe([](ak::engine::RegenerateConfigEvent& event){
	event.data()["log"]["consoleFilterLevel"].set<uint8>(static_cast<int8>(ak::log::Level::DEBUG));
	event.data()["log"]["fileFilterLevel"].set<uint8>(static_cast<int8>(ak::log::Level::DEBUG));
});

static ak::event::SubscriberID logSInitRegisterConfigHooks = ak::engine::setConfigDispatch().subscribe([](ak::engine::SetConfigEvent& event){
	ak::log::setConsoleFilterLevel(static_cast<ak::log::Level>(event.data().atOrDef("log").atOrDef("consoleFilterLevel", static_cast<int64>(ak::log::Level::DEBUG)).as<uint8>()));
	ak::log::setFileFilterLevel(static_cast<ak::log::Level>(event.data().atOrDef("log").atOrDef("fileFilterLevel", static_cast<int64>(ak::log::Level::DEBUG)).as<uint8>()));
});

