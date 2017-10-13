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

static ak::thread::Thread logProcessingThread("Log");
static ak::filesystem::CFile logFileOutput;
static ak::container::DoubleBuffer<std::string> logMessageBuffer;
static Level logVerboseLevel = Level::DEBUG;

static void proccessMessageQueue() {
	logMessageBuffer.swap();

	logMessageBuffer.iterate([&](size_t, const std::string& message){
		std::cout << message;
		if (logFileOutput) logFileOutput.write(message.c_str(), message.size());
	});

	std::cout.flush();
	if (logFileOutput) logFileOutput.flush();
}

bool Logger::isLevelEnabled(Level logLevel) {
	return static_cast<uint8>(logLevel) <= static_cast<uint8>(logVerboseLevel);
}

bool Logger::isLoggingEnabled() {
	return logProcessingThread.isRunning();
}

void Logger::printMessage(const std::string& str) {
	logMessageBuffer.push_back(str);
}

bool ak::log::startup(ak::filesystem::CFile nFile, Level logLevel) {
	if (logProcessingThread.isRunning()) return false;

	logFileOutput = std::move(nFile);
	logVerboseLevel = logLevel;

	logProcessingThread.execute([]{
		while(!logProcessingThread.isCloseRequested()) {
			ak::thread::current().sleep(1e5);
			proccessMessageQueue();
		}
	});

	logProcessingThread.detach();

	return true;
}

void ak::log::setLogLevel(Level logLevel) {
	logVerboseLevel = logLevel;
}

Level ak::log::getLogLevel() {
	return logVerboseLevel;
}

void ak::log::shutdown() {
	logProcessingThread.requestClose();
	logProcessingThread.join();

	proccessMessageQueue();
	proccessMessageQueue();

	if (logFileOutput) logFileOutput.flush();
}

