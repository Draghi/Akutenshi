#include <ak/data/JsonParser.hpp>
#include <ak/data/PValue.hpp>
#include <ak/engine/Config.hpp>
#include <ak/engine/Startup.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/log/Logger.hpp>
#include <ak/ScopeGuard.hpp>
#include <string_view>

#if defined(__linux)
#define BACKWARD_HAS_BFD 1
#include "backward.hpp"
namespace backward { static backward::SignalHandling sh; }
#endif

static void engineShutdown();

int main() {
	ak::engine::startup(ak::data::PValue());
	auto scopeGuard = ak::ScopeGuard(engineShutdown);

	constexpr ak::log::Logger log(std::string_view("PValueTest", 10));

	ak::data::PValue pTree;

	ak::filesystem::serializeFolders(pTree);
	log.info([&]{return ak::data::serializeJson(pTree);});

	pTree["appData"].setString("./modi");
	log.info([&]{return ak::data::serializeJson(pTree);});
	ak::filesystem::deserializeFolders(pTree);

	ak::filesystem::serializeFolders(pTree);
	log.info([&]{return ak::data::serializeJson(pTree);});

	ak::filesystem::overrideFolder(ak::filesystem::SystemFolder::appConfig, "./config/override");
	ak::filesystem::resetFolder(ak::filesystem::SystemFolder::appData);
	ak::filesystem::serializeFolders(pTree);
	log.info([&]{return ak::data::serializeJson(pTree);});

	//auto configFile = ak::filesystem::CFile("./startup.config", ak::filesystem::OpenFlags::Out | ak::filesystem::OpenFlags::Truncate);
	//if (!configFile) return -1;
	//ak::engine::saveConfig(configFile);
}

static void engineShutdown() {
	constexpr ak::log::Logger log(std::string_view("Shutdown", 8));

	log.info("Engine shutdown sequence started");
	ak::log::shutdown();
}
