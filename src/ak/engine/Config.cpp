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
#include <ak/event/Dispatcher.hpp>
#include <ak/log/Logger.hpp>
#include <iostream>
#include <string>

using namespace ak::engine;

static ak::event::Dispatcher<ConfigReloadEvent> configReloadDispatcher;
static ak::data::PValue currentConfig;

static ak::log::Logger configLog("Config");
static ak::filesystem::CFile configFile;

void ak::engine::setConfigFile(ak::filesystem::CFile&& newConfigFile) {
	configFile = std::move(newConfigFile);
}

bool ak::engine::reloadConfig() {
	configLog.info("Attempting to load global config");

	{
		std::string configContents;
		if (configFile.readLine(configContents, false, {}) <= 0) { configLog.warn("Failed to read global config contents"); return false; }

		ak::data::PValue gConfig;
		std::stringstream sstream(configContents);
		if (!ak::data::deserializeJson(gConfig, sstream)) { configLog.warn("Failed to read global config contents"); return false; }

		currentConfig = gConfig;
	}

	ConfigReloadEvent reloadEvent(currentConfig);
	configReloadDispatcher.send(reloadEvent);

	configLog.info("Successfully reloaded global config");

	return true;
}

bool ak::engine::saveConfig() {
	return configFile.writeLine(ak::data::serializeJson(config(), true)) > 0;
}

ak::data::PValue& ak::engine::config() {
	return currentConfig;
}

void ak::engine::subscribeConfigReload(ak::event::Subscription& subscriber, const std::function<void(ConfigReloadEvent&)>& callback) {
	configReloadDispatcher.subscribe(subscriber, callback);
}

ak::event::SubscriberID ak::engine::subscribeConfigReload(const std::function<void(ConfigReloadEvent&)>& callback) {
	return configReloadDispatcher.subscribe(callback);
}

void ak::engine::unsubscribeConfigReload(ak::event::Subscription& subscriber) {
	configReloadDispatcher.unsubscribe(subscriber);
}

