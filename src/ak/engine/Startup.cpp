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

#include <ak/engine/Config.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/log/Logger.hpp>
#include <ak/thread/CurrentThread.hpp>
#include <ak/thread/Thread.hpp>
#include <ak/time/Time.hpp>
#include <bits/types/struct_tm.h>
#include "ak/engine/Startup.hpp"
#include <atomic>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace ak::engine;

static ak::log::Logger startupLog("Startup");
static std::atomic<bool> hasStarted = false;

static bool startLogger();
static void printBanner();
static bool bootstrapConfig(const std::string& configLocation);

void ak::engine::startup(const ak::data::PValue& arguments) {
	if (hasStarted.exchange(true)) throw std::runtime_error("Engine is already running");

	ak::thread::current().setName("Main");

	if (!startLogger()) throw std::runtime_error("Could not start logging service");

	printBanner();

	startupLog.info("Engine initialization sequence started");

	auto configLocation = arguments.exists("configLocation") ? arguments["configLocation"].stringValue() : "./startup.config";
	if (!bootstrapConfig(configLocation)) throw std::runtime_error("Could not start load configuration data");

	startupLog.info("Engine initialization sequence finished");
}

static bool startLogger() {
	auto utc = ak::time::utcTimestamp();
	std::stringstream filename;
	filename << "./log_" << std::put_time(&utc.ctime, "%Y%m%d_%H%M%S") << ".txt";
	return ak::log::startup(ak::filesystem::CFile(filename.str(), ak::filesystem::OpenFlags::Out | ak::filesystem::OpenFlags::Truncate));
}

static void printBanner() {
	auto utc = ak::time::utcTimestamp();
	std::stringstream timestamp;
	timestamp << std::put_time(&utc.ctime, "%Y-%m-%d %H:%M:%S") << "." << std::setw(3) << std::setfill('0') << utc.milliseconds;

	startupLog.raw(R"(+-----------------------------------------------------------------------------+)", '\n',
	               R"(|         __                __                              __                |)", '\n',
	               R"(|        /\ \              /\ \__                          /\ \       __      |)", '\n',
	               R"(|    __  \ \ \/'\    __  __\ \ ,_\     __     ___      ____\ \ \___  /\_\     |)", '\n',
	               R"(|  /'__`\ \ \ , <   /\ \/\ \\ \ \/   /'__`\ /' _ `\   /',__\\ \  _ `\\/\ \    |)", '\n',
	               R"(| /\ \_\.\_\ \ \\`\ \ \ \_\ \\ \ \_ /\  __/ /\ \/\ \ /\__, `\\ \ \ \ \\ \ \   |)", '\n',
	               R"(| \ \__/.\_\\ \_\ \_\\ \____/ \ \__\\ \____\\ \_\ \_\\/\____/ \ \_\ \_\\ \_\  |)", '\n',
	               R"(|  \/__/\/_/ \/_/\/_/ \/___/   \/__/ \/____/ \/_/\/_/ \/___/   \/_/\/_/ \/_/  |)", '\n',
	               R"(|  _______________________________________________________________________    |)", '\n',
	               R"(| /\                                                                      \   |)", '\n',
	               R"(| \ \               Game Engine - github.com/draghi/akutenshi              \  |)", '\n',
	               R"(|  \ \______________________________________________________________________\ |)", '\n',
	               R"(|   \/______________________________________________________________________/ |)", '\n',
	               R"(+-----------------------------------------------------------------------------+)", '\n',
	               R"(| Start time: )", timestamp.str(),  "                                         |",  '\n',
	               R"(+-----------------------------------------------------------------------------+)", '\n');
}

static bool bootstrapConfig(const std::string& configLocation) {
	ak::engine::subscribeConfigReload([](ak::engine::ConfigReloadEvent& event){ if(event.config().exists("systemFolders")) ak::filesystem::deserializeFolders(event.config()["systemFolders"]); });
	ak::engine::subscribeConfigReload([](ak::engine::ConfigReloadEvent& event){ if(event.config().exists("logLevel")) ak::log::setLogLevel(static_cast<ak::log::Level>(event.config()["logLevel"].asUnsigned()));});

	auto configFile = ak::filesystem::CFile(configLocation, ak::filesystem::OpenFlags::In);
	if (!configFile) return false;

	return ak::engine::reloadConfig(configFile);
}

