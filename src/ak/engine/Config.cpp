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

#include <ak/data/JsonParser.hpp>
#include <ak/engine/Config.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/Log.hpp>
#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <system_error>

using namespace ake;

static const stx::filesystem::path CONFIG_PATH = "./akutenshi.config";
static const stx::filesystem::path BACKUP_PATH = "./akutenshi.config.akbak";

static bool backupConfig();
static bool deleteBackup();
static bool restoreBackup();

static akd::PValue& configInstance() {
	static akd::PValue instance;
	return instance;
}

static akev::Dispatcher<RegenerateConfigEvent>& regenerateConfigDispatcher() { static akev::Dispatcher<RegenerateConfigEvent> instance; return instance; }
static akev::Dispatcher<LoadConfigEvent>&  loadConfigDispatcher()  { static akev::Dispatcher<LoadConfigEvent>  instance; return instance; }
static akev::Dispatcher<SaveConfigEvent>&  saveConfigDispatcher()  { static akev::Dispatcher<SaveConfigEvent>  instance; return instance; }
static akev::Dispatcher<SetConfigEvent>&   setConfigDispatcher()   { static akev::Dispatcher<SetConfigEvent>   instance; return instance; }

const akev::DispatcherProxy<RegenerateConfigEvent>& ake::regenerateConfigDispatch() { static akev::DispatcherProxy<RegenerateConfigEvent> instance(regenerateConfigDispatcher()); return instance; }
const akev::DispatcherProxy<LoadConfigEvent>&  ake::loadConfigDispatch()  { static akev::DispatcherProxy<LoadConfigEvent>  instance(loadConfigDispatcher());  return instance; }
const akev::DispatcherProxy<SaveConfigEvent>&  ake::saveConfigDispatch()  { static akev::DispatcherProxy<SaveConfigEvent>  instance(saveConfigDispatcher());  return instance; }
const akev::DispatcherProxy<SetConfigEvent>&   ake::setConfigDispatch()   { static akev::DispatcherProxy<SetConfigEvent>   instance(setConfigDispatcher());   return instance; }

const akd::PValue& ake::config() {
	return configInstance();
}

void ake::setConfig(const akd::PValue& nConfig) {
	configInstance() = nConfig;

	SetConfigEvent event(nConfig);
	setConfigDispatcher().send(event);
}

void ake::regenerateConfig() {
	akd::PValue newConfig;

	RegenerateConfigEvent event(newConfig);
	regenerateConfigDispatcher().send(event);

	setConfig(newConfig);
}

ConfigLoadResult ake::loadConfig() {
	akfs::CFile configFile(CONFIG_PATH, akfs::In);
	if (!configFile) return ConfigLoadResult::CannotOpen;

	std::string configContents;
	if (configFile.readLine(configContents, false, {}) <= 0) return ConfigLoadResult::CannotRead;

	akd::PValue newConfig;
	std::istringstream sstrean(configContents);
	if (!akd::deserializeJson(newConfig, sstrean)) return ConfigLoadResult::CannotParse;

	setConfig(newConfig);
	return ConfigLoadResult::Success;
}

bool ake::saveConfig() {
	if (!backupConfig()) return false;

	akfs::CFile configFile(CONFIG_PATH, akfs::Out | akfs::Truncate);
	if (!configFile) return false;

	akd::PValue configSnapshot = config();
	SaveConfigEvent event(configSnapshot);
	saveConfigDispatcher().send(event);

	std::string contents = akd::serializeJson(configSnapshot, true);

	if (configFile.write(contents.data(), contents.size()) <= 0) {
		configFile = akfs::CFile();
		restoreBackup();
		return false;
	}

	deleteBackup();
	return true;
}

static bool backupConfig() {
	std::error_code err;
	stx::filesystem::remove(BACKUP_PATH, err);

	err.clear();
	stx::filesystem::rename(CONFIG_PATH, BACKUP_PATH, err);
	return !(err && stx::filesystem::exists(CONFIG_PATH));
}

static bool deleteBackup() {
	std::error_code err;
	stx::filesystem::remove(BACKUP_PATH, err);
	return !stx::filesystem::exists(CONFIG_PATH);
}

static bool restoreBackup() {
	std::error_code err;
	stx::filesystem::remove(CONFIG_PATH, err);

	err.clear();
	stx::filesystem::rename(BACKUP_PATH, CONFIG_PATH, err);
	return !(err && stx::filesystem::exists(BACKUP_PATH));
}


/*static akev::Dispatcher<ConfigReloadEvent>& configReloadDispatcher();
static akd::PValue currentConfig;

static ak::log::Logger configLog("Config");
static stx::filesystem::path configPath;

void ake::setConfigFile(const stx::filesystem::path& cPath) {
	configPath = cPath;
}

bool ake::reloadConfig() {
	auto configFile = akfs::CFile(configPath, akfs::In);
	if (!configFile) return false;

	std::string configContents;
	if (configFile.readLine(configContents, false, {}) <= 0) { configLog.warn("Failed to read global config contents"); return false; }

	akd::PValue gConfig;
	std::stringstream sstream(configContents);
	if (!akd::deserializeJson(gConfig, sstream)) { configLog.warn("Failed to read global config contents"); return false; }

	currentConfig = gConfig;

	ConfigReloadEvent reloadEvent(currentConfig);
	configReloadDispatcher().send(reloadEvent);

	return true;
}

static stx::filesystem::path backupPath() { return stx::filesystem::path(configPath).replace_extension("akbak"); }

static bool backupConfig() {
	auto bPath = backupPath();

	std::error_code err;
	stx::filesystem::remove(bPath, err);

	err.clear();
	stx::filesystem::rename(configPath, bPath, err);
	return !(err && stx::filesystem::exists(configPath));
}

static bool deleteBackup() {
	auto bPath = backupPath();
	std::error_code err;
	stx::filesystem::remove(bPath, err);
	return !stx::filesystem::exists(configPath);
}

static bool restoreBackup() {
	auto bPath = backupPath();

	std::error_code err;
	stx::filesystem::remove(configPath, err);

	err.clear();
	stx::filesystem::rename(bPath, configPath, err);
	return !(err && stx::filesystem::exists(bPath));
}

bool ake::saveConfig() {
	backupConfig();

	auto configFile = akfs::CFile(configPath, akfs::Out | akfs::Truncate);

	if ((!configFile) || (configFile.writeLine(akd::serializeJson(config(), true)) <= 0)) {
		restoreBackup();
		return false;
	}

	deleteBackup();
	return true;
}

akd::PValue& ake::config() {
	return currentConfig;
}

const akev::DispatcherProxy<ConfigReloadEvent>& ake::reloadEvent() {
	static akev::DispatcherProxy<ConfigReloadEvent> dispatchProxy(configReloadDispatcher());
	return dispatchProxy;
}

static akev::Dispatcher<ConfigReloadEvent>& configReloadDispatcher() {
	static akev::Dispatcher<ConfigReloadEvent> sConfigReloadDispatcher;
	return sConfigReloadDispatcher;
}*/

/*void ake::subscribeConfigReload(akev::Subscription& subscriber, const std::function<void(ConfigReloadEvent&)>& callback) {
	configReloadDispatcher.subscribe(subscriber, callback);
}

akev::SubscriberID ake::subscribeConfigReload(const std::function<void(ConfigReloadEvent&)>& callback) {
	return configReloadDispatcher.subscribe(callback);
}

void ake::unsubscribeConfigReload(akev::Subscription& subscriber) {
	configReloadDispatcher.unsubscribe(subscriber);
}*/

