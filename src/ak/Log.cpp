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

using namespace ak::log;

static akt::Thread loggingThread("Log");

static akt::Spinlock messageQueueProcessLock;
static akt::DoubleBuffer<std::pair<Level, std::string>> logMessageBuffer;
static Level consoleFilterLevel = Level::ALL;
static Level fileFilterLevel = Level::ALL;

static std::atomic<bool> isRedirrectingStd = false;

static akt::Spinlock logFileLock;
static akfs::CFile logFile;

bool ak::log::startProcessing(uint64 delayUS) {
	if (loggingThread.isRunning()) return false;

	loggingThread.execute([=]{
		while(!akt::current().isCloseRequested()) {
			processMessageQueue();
			akt::current().sleep(delayUS);
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


bool ak::log::enableFileOutput() {
	auto fileLock = logFileLock.lock();

	auto utc = aku::utcTimestamp();
	std::stringstream filename;
	filename << "data/logs/log_" << std::put_time(&utc.ctime, "%Y%m%d_%H%M%S") << ".txt";

	auto newLogFile = akfs::CFile(filename.str(), akfs::OpenFlags::Out | akfs::OpenFlags::Truncate);
	if (!newLogFile) return false;
	logFile = std::move(newLogFile);

	return true;
}

void ak::log::disableFileOutput() {
	auto fileLock = logFileLock.lock();
	logFile = akfs::CFile();
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


static akev::SubscriberID logSInitRegenerateConfigHook = ake::regenerateConfigDispatch().subscribe([](ake::RegenerateConfigEvent& event){
	// @todo replace indicies with names
	event.data()["log"]["consoleFilterLevel"].set<uint8>(static_cast<int8>(ak::log::Level::DEBUG));
	event.data()["log"]["fileFilterLevel"].set<uint8>(static_cast<int8>(ak::log::Level::DEBUG));
});

static akev::SubscriberID logSInitRegisterConfigHooks = ake::setConfigDispatch().subscribe([](ake::SetConfigEvent& event){
	ak::log::setConsoleFilterLevel(static_cast<ak::log::Level>(event.data().atOrDef("log").atOrDef("consoleFilterLevel", akd::PValue::from(static_cast<int8>(ak::log::Level::DEBUG))).as<uint8>()));
	ak::log::setFileFilterLevel(static_cast<ak::log::Level>(event.data().atOrDef("log").atOrDef("fileFilterLevel", akd::PValue::from(static_cast<int8>(ak::log::Level::DEBUG))).as<uint8>()));
});


