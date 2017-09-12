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
#include <ak/log/Logger.hpp>
#include <stddef.h>
#include <algorithm>
#include <iostream>

using namespace ak::log;

static ak::thread::Thread logThread("Log");

struct LoggerData {
	ak::filesystem::CFile file;
	ak::container::DoubleBuffer<std::string> messages;
};
static std::shared_ptr<LoggerData> logData = std::make_shared<LoggerData>();

static Level minLogLevel = Level::DEBUG;

static void proccessMessageQueue(LoggerData& data) {
	data.messages.swap();
	data.messages.iterate([&](size_t, const std::string& message){
		std::cout << message << std::flush;
		if (data.file) {
			data.file.write(message.c_str(), message.size());
			data.file.flush();
		}
	});
}

bool Logger::isLevelEnabled(Level logLevel) {
	return static_cast<uint8>(logLevel) <= static_cast<uint8>(minLogLevel);
}

bool Logger::isLoggingEnabled() {
	return logThread.isRunning();
}

void Logger::printMessage(const std::string& str) {
	logData->messages.push_back(str);
}

bool ak::log::startup(ak::filesystem::CFile file, Level logLevel) {
	if (logThread.isRunning()) return false;

	logData->file = std::move(file);
	minLogLevel = logLevel;

	logThread.execute([]{
		auto dataLock = logData;
		while(!logThread.isCloseRequested()) {
			ak::thread::current().sleep(1e5);
			proccessMessageQueue(*dataLock);
		}
	});

	return true;
}

void ak::log::setLogLevel(Level logLevel) {
	minLogLevel = logLevel;
}

Level ak::log::getLogLevel() {
	return minLogLevel;
}

void ak::log::shutdown() {
	logThread.requestClose();
	logThread.waitFor();

	proccessMessageQueue(*logData);
	proccessMessageQueue(*logData);

	if (logData && logData->file) logData->file.flush();
}

