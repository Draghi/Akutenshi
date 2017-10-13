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
#include <ak/PrimitiveTypes.hpp>
#include <ak/thread/CurrentThread.hpp>
#include <ak/thread/Thread.hpp>
#include <ak/time/Time.hpp>
#include <ak/window/Window.hpp>
#include "ak/engine/Startup.hpp"
#include <atomic>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace ak::engine;

static ak::log::Logger startupLog("Startup");
static ak::log::Logger shutdownLog("Shutdown");
static std::atomic<bool> hasStarted = false;

using StartupFunc = void(const ak::data::PValue&);
using ShutdownFunc = void();

struct StartupRecord {
	std::string startupName;
	StartupFunc* startupFunc;
	ShutdownFunc* shutdownFunc;
};

static void printBanner();

static void startLogger(const ak::data::PValue& args);
static void startConfig(const ak::data::PValue& args);
static void stopConfig();
static void startWindow(const ak::data::PValue& args);

static std::vector<StartupRecord> startupFunctions({
	StartupRecord{"Config", startConfig, stopConfig},
	StartupRecord{"Log",    startLogger, []{}},
	StartupRecord{"Window", startWindow, ak::window::shutdown},
});

ak::ScopeGuard ak::engine::startup(const ak::data::PValue& arguments) {
	if (hasStarted.exchange(true)) throw std::runtime_error("Engine is already running");
	ak::thread::current().setName("Main");

	startupLog.info("Starting Engine...");
		for(auto iter = startupFunctions.begin(); iter != startupFunctions.end(); iter++) {
			if (!iter->startupFunc) continue;
			startupLog.info("Starting ", iter->startupName, " System...");
				iter->startupFunc(arguments);
			startupLog.info("Started ", iter->startupName, " System.");
		}
	startupLog.info("Started Engine.");

	printBanner();

	return [&]{
		startupLog.info("Stopping Engine...");
			for(auto iter = startupFunctions.rbegin(); iter != startupFunctions.rend(); iter++) {
				if (!iter->shutdownFunc) continue;
				startupLog.info("Stopping ", iter->startupName, " System...");
					iter->shutdownFunc();
				startupLog.info("Stopped ", iter->startupName, " System.");
			}
		startupLog.info("Stopped Engine.");

		ak::log::shutdown();
	};
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

static void startLogger(const ak::data::PValue& /*args*/) {
	auto utc = ak::time::utcTimestamp();
	std::stringstream filename;
	filename << "logs/log_" << std::put_time(&utc.ctime, "%Y%m%d_%H%M%S") << ".txt";
	auto logFile = ak::filesystem::open(ak::filesystem::SystemFolder::appData, filename.str(), ak::filesystem::OpenFlags::Out | ak::filesystem::OpenFlags::Truncate);
	ak::log::startup(std::move(logFile));
}

static void startConfig(const ak::data::PValue& args) {

	auto configLocation = args.getOrNull("configLocation").asStrOr("./startup.config");

	ak::engine::subscribeConfigReload([](ak::engine::ConfigReloadEvent& event){
		auto systemFolders = event.config().getOrNull("systemFolders");
		ak::filesystem::deserializeFolders(systemFolders);
	});

	ak::engine::subscribeConfigReload([](ak::engine::ConfigReloadEvent& event){
		auto logLevel = event.config().getOrNull("logLevel").asIntOr(static_cast<int64>(ak::log::Level::INFO));
		ak::log::setLogLevel(static_cast<ak::log::Level>(logLevel));
	});

	auto configFile = ak::filesystem::CFile(configLocation, ak::filesystem::OpenFlags::In);
	if (!configFile) return;
	ak::engine::setConfigFile(std::move(configFile));
	ak::engine::reloadConfig();
}

static void stopConfig() {
	ak::engine::saveConfig();
}

static void startWindow(const ak::data::PValue& /*args*/) {
	ak::window::init();
}
