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

#include <ak/engine/Config.hpp>

#include <string>

#include <ak/data/Json.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/filesystem/Path.hpp>

using namespace ake;

static const akfs::Path CONFIG_PATH("./akutenshi.config");
static const akfs::Path BACKUP_PATH("./akutenshi.config.akbak");

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
	if (!akd::fromJson(newConfig, configContents)) return ConfigLoadResult::CannotParse;

	setConfig(newConfig);
	return ConfigLoadResult::Success;
}

bool ake::saveConfig() {
	bool alreadyExists = akfs::exists(CONFIG_PATH);
	if (alreadyExists && !backupConfig()) return false;

	akfs::CFile configFile(CONFIG_PATH, akfs::Out | akfs::Truncate);
	if (!configFile) return false;

	akd::PValue configSnapshot = config();
	SaveConfigEvent event(configSnapshot);
	saveConfigDispatcher().send(event);

	std::string contents = akd::toJson(configSnapshot, true);

	if (configFile.write(contents.data(), contents.size()) <= 0) {
		configFile = akfs::CFile();
		if (alreadyExists) restoreBackup();
		return false;
	}

	if (alreadyExists) deleteBackup();
	return true;
}

static bool backupConfig() {
	return akfs::rename(CONFIG_PATH, BACKUP_PATH, true);
}

static bool deleteBackup() {
	return akfs::remove(BACKUP_PATH);
}

static bool restoreBackup() {
	return akfs::rename(BACKUP_PATH, CONFIG_PATH, true);
}

