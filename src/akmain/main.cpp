#include <akcommon/ScopeGuard.hpp>
#include <akcommon/String.hpp>
#include <akcommon/Time.hpp>
#include <akengine/debug/Log.hpp>
#include <akengine/entity/Config.hpp>
#include <akengine/thread/CurrentThread.hpp>
#include <akgame/game.hpp>
#include <akmain/Debug.hpp>
#include <iomanip>
#include <sstream>
#include <stdexcept>

static ak::ScopeGuard bootstrapEngine(const akl::Logger& log, bool redirectLogToFile);

int main(int /*argc*/, char* /*argv*/[]) {
	akmain::setupDebugHandling();

	constexpr akl::Logger startLog(AK_STRING_VIEW("Start"));
	auto cleanup = bootstrapEngine(startLog, false);

	akg::runGame();

	return 0;
}

static void cleanupEngine();
static void startupConfig();
static void printLogHeader(const akl::Logger& log);

static ak::ScopeGuard bootstrapEngine(const akl::Logger& log, bool redirectLogToFile) {
	auto cleanup = ak::ScopeGuard(cleanupEngine);

	/* Setup Thread Name */ {
		akt::current().setName("Main");
	}

	log.info("Loading engine config."); {
		startupConfig();
	}

	log.info("Starting log system."); {
		akl::startProcessing();
		if (redirectLogToFile) akl::enableFileOutput();
	}

	log.info("Starting game systems."); {
		akg::startup(log);
	}

	printLogHeader(log);

	return cleanup;
}

// ///////////////////// //
// // Secondary Setup // //
// ///////////////////// //

static void cleanupEngine() {
	constexpr akl::Logger log(AK_STRING_VIEW("Stop"));

	log.info("Engine shutting down.");

	akg::cleanup(log);

	log.info("Saving config.");
	if (!ake::saveConfig()) log.warn("Failed to save config.");

	log.info("Flushing log system.");
	akl::stopProcessing();
	akl::processMessageQueue();
	akl::processMessageQueue();
}

static void startupConfig() {
	constexpr akl::Logger log(AK_STRING_VIEW("Config"));
	switch(ake::loadConfig()) {
		case ake::ConfigLoadResult::Success: break;

		case ake::ConfigLoadResult::CannotOpen: {
			log.warn("Failed to load config. Attempting to regenerate.");
			ake::regenerateConfig();
			if (!ake::saveConfig()) log.warn("Failed to save new config, config may not persist.");
		} break;

		case ake::ConfigLoadResult::CannotRead: {
			log.fatal("Cannot read config. Fix permissions or delete to attempt regeneration.");
			throw std::runtime_error("Startup error, cannot read config.");
		}

		case ake::ConfigLoadResult::CannotParse: {
			log.fatal("Cannot parse config. Fix format or delete to attempt regeneration.");
			throw std::runtime_error("Startup error, cannot parse config.");
		}
	}
}


static void printLogHeader(const akl::Logger& log) {
	auto utc = aku::utcTimestamp();
	std::stringstream dateStream; dateStream << std::put_time(&utc.ctime, "%Y-%m-%d");
	std::stringstream timeStream; timeStream << std::put_time(&utc.ctime, "%H:%M:%S") << "." << std::setw(3) << std::setfill('0') << utc.milliseconds;
	log.raw(R"(+-----------------------------------------------------------------------------+)", '\n',
		    R"(|     _____    __               __                            __       __     |)", '\n',
		    R"(|    // __ \  || |             || |                          || |     ((_)    |)", '\n',
			R"(|   || |_|| | || | __  __  __  || |_   ____   __ ___   ____  || |___   __     |)", '\n',
			R"(|   ||  __  | || |/ / || ||| | || __| // _ \ || '_  \ // __| || '_  \ || |    |)", '\n',
			R"(|   || | || | ||   <  || |_| | || |_ ||  __/ || | | | \\__ \ || ||| | || |    |)", '\n',
			R"(|   ||_| ||_| ||_|\_\ \\___,_|  \\__| \\___| ||_| |_| ||___/ ||_|||_| ||_|    |)", '\n',
			R"(|  _______________________________________________________________________    |)", '\n',
			R"(| /\                                                                      \   |)", '\n',
			R"(| \ \               Game Engine - github.com/draghi/akutenshi              \  |)", '\n',
			R"(|  \ \______________________________________________________________________\ |)", '\n',
			R"(|   \/______________________________________________________________________/ |)", '\n',
			R"(+-----------------------------------------------------------------------------+)", '\n',
			R"(| Engine started on )", dateStream.str(),  " at ", timeStream.str(), "                                |",  '\n',
			R"(+-----------------------------------------------------------------------------+)", '\n');
}
