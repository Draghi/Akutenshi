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

using namespace ak::engine;

static const stx::filesystem::path CONFIG_PATH = "./akutenshi.config";
static const stx::filesystem::path BACKUP_PATH = "./akutenshi.config.akbak";

static bool backupConfig();
static bool deleteBackup();
static bool restoreBackup();

static ak::data::PValue& configInstance() {
	static ak::data::PValue instance;
	return instance;
}

static ak::event::Dispatcher<RegenerateConfigEvent>& regenerateConfigDispatcher() { static ak::event::Dispatcher<RegenerateConfigEvent> instance; return instance; }
static ak::event::Dispatcher<LoadConfigEvent>&  loadConfigDispatcher()  { static ak::event::Dispatcher<LoadConfigEvent>  instance; return instance; }
static ak::event::Dispatcher<SaveConfigEvent>&  saveConfigDispatcher()  { static ak::event::Dispatcher<SaveConfigEvent>  instance; return instance; }
static ak::event::Dispatcher<SetConfigEvent>&   setConfigDispatcher()   { static ak::event::Dispatcher<SetConfigEvent>   instance; return instance; }

const ak::event::DispatcherProxy<RegenerateConfigEvent>& ak::engine::regenerateConfigDispatch() { static ak::event::DispatcherProxy<RegenerateConfigEvent> instance(regenerateConfigDispatcher()); return instance; }
const ak::event::DispatcherProxy<LoadConfigEvent>&  ak::engine::loadConfigDispatch()  { static ak::event::DispatcherProxy<LoadConfigEvent>  instance(loadConfigDispatcher());  return instance; }
const ak::event::DispatcherProxy<SaveConfigEvent>&  ak::engine::saveConfigDispatch()  { static ak::event::DispatcherProxy<SaveConfigEvent>  instance(saveConfigDispatcher());  return instance; }
const ak::event::DispatcherProxy<SetConfigEvent>&   ak::engine::setConfigDispatch()   { static ak::event::DispatcherProxy<SetConfigEvent>   instance(setConfigDispatcher());   return instance; }

const ak::data::PValue& ak::engine::config() {
	return configInstance();
}

void ak::engine::setConfig(const ak::data::PValue& nConfig) {
	configInstance() = nConfig;

	SetConfigEvent event(nConfig);
	setConfigDispatcher().send(event);
}

void ak::engine::regenerateConfig() {
	ak::data::PValue newConfig;

	RegenerateConfigEvent event(newConfig);
	regenerateConfigDispatcher().send(event);

	setConfig(newConfig);
}

bool ak::engine::loadConfig() {
	ak::filesystem::CFile configFile(CONFIG_PATH, ak::filesystem::In);
	if (!configFile) return false;

	std::string configContents;
	if (configFile.readLine(configContents, false, {}) <= 0) return false;

	ak::data::PValue newConfig;
	std::istringstream sstrean(configContents);
	if (!ak::data::deserializeJson(newConfig, sstrean)) return false;

	setConfig(newConfig);

	return true;
}

bool ak::engine::saveConfig() {
	if (!backupConfig()) return false;

	ak::filesystem::CFile configFile(CONFIG_PATH, ak::filesystem::Out | ak::filesystem::Truncate);
	if (!configFile) return false;

	ak::data::PValue configSnapshot = config();
	SaveConfigEvent event(configSnapshot);
	saveConfigDispatcher().send(event);

	std::string contents = ak::data::serializeJson(configSnapshot, true);

	if (configFile.write(contents.data(), contents.size()) <= 0) {
		configFile = ak::filesystem::CFile();
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


/*static ak::event::Dispatcher<ConfigReloadEvent>& configReloadDispatcher();
static ak::data::PValue currentConfig;

static ak::log::Logger configLog("Config");
static stx::filesystem::path configPath;

void ak::engine::setConfigFile(const stx::filesystem::path& cPath) {
	configPath = cPath;
}

bool ak::engine::reloadConfig() {
	auto configFile = ak::filesystem::CFile(configPath, ak::filesystem::In);
	if (!configFile) return false;

	std::string configContents;
	if (configFile.readLine(configContents, false, {}) <= 0) { configLog.warn("Failed to read global config contents"); return false; }

	ak::data::PValue gConfig;
	std::stringstream sstream(configContents);
	if (!ak::data::deserializeJson(gConfig, sstream)) { configLog.warn("Failed to read global config contents"); return false; }

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

bool ak::engine::saveConfig() {
	backupConfig();

	auto configFile = ak::filesystem::CFile(configPath, ak::filesystem::Out | ak::filesystem::Truncate);

	if ((!configFile) || (configFile.writeLine(ak::data::serializeJson(config(), true)) <= 0)) {
		restoreBackup();
		return false;
	}

	deleteBackup();
	return true;
}

ak::data::PValue& ak::engine::config() {
	return currentConfig;
}

const ak::event::DispatcherProxy<ConfigReloadEvent>& ak::engine::reloadEvent() {
	static ak::event::DispatcherProxy<ConfigReloadEvent> dispatchProxy(configReloadDispatcher());
	return dispatchProxy;
}

static ak::event::Dispatcher<ConfigReloadEvent>& configReloadDispatcher() {
	static ak::event::Dispatcher<ConfigReloadEvent> sConfigReloadDispatcher;
	return sConfigReloadDispatcher;
}*/

/*void ak::engine::subscribeConfigReload(ak::event::Subscription& subscriber, const std::function<void(ConfigReloadEvent&)>& callback) {
	configReloadDispatcher.subscribe(subscriber, callback);
}

ak::event::SubscriberID ak::engine::subscribeConfigReload(const std::function<void(ConfigReloadEvent&)>& callback) {
	return configReloadDispatcher.subscribe(callback);
}

void ak::engine::unsubscribeConfigReload(ak::event::Subscription& subscriber) {
	configReloadDispatcher.unsubscribe(subscriber);
}*/

