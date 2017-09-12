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

#ifndef AK_ENGINE_CONFIG_HPP_
#define AK_ENGINE_CONFIG_HPP_

#include <ak/data/PValue.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/event/Event.hpp>
#include <functional>

namespace ak {
	namespace filesystem {
		class CFile;
	}
}

namespace ak {
	namespace data {
		class PValue;
	}
}

namespace ak {
	namespace event {
		class Subscription;
	}
}

namespace ak {
	namespace engine {

		bool reloadConfig(ak::filesystem::CFile& configFile);
		bool saveConfig(ak::filesystem::CFile& configFile);


		class ConfigReloadEvent : public ak::event::Event {
			AK_IMPLEMENT_EVENT("ConfigReloadEvent", false)
			private:
				const ak::data::PValue& m_config;
			public:
				ConfigReloadEvent(const ak::data::PValue& config) : m_config(config) {}
				const ak::data::PValue& config() const { return m_config; }
		};

		void subscribeConfigReload(ak::event::Subscription& subscriber, const std::function<void(ConfigReloadEvent&)>& callback);
		template<typename func_t> inline void subscribeConfigReload(ak::event::Subscription& subscriber, const func_t& callback) { subscribeConfigReload(subscriber, std::function<void(ConfigReloadEvent&)>(callback)); }

		ak::event::SubscriberID subscribeConfigReload(const std::function<void(ConfigReloadEvent&)>& callback);
		template<typename func_t> inline ak::event::SubscriberID subscribeConfigReload(const func_t& callback) { return subscribeConfigReload(std::function<void(ConfigReloadEvent&)>(callback)); }

		void unsubscribeConfigReload(ak::event::Subscription& subscriber);


		class ConfigSaveEvent : public ak::event::Event {
			AK_IMPLEMENT_EVENT("ConfigSaveEvent", false)
			private:
				ak::data::PValue& m_config;
			public:
				ConfigSaveEvent(ak::data::PValue& config) : m_config(config) {}
				ak::data::PValue& config() const { return m_config; }
		};

		void subscribeConfigSave(ak::event::Subscription& subscriber, const std::function<void(ConfigSaveEvent&)>& callback);
		template<typename func_t> inline void subscribeConfigSave(ak::event::Subscription& subscriber, const func_t& callback) { subscribeConfigSave(subscriber, std::function<void(ConfigSaveEvent&)>(callback)); }

		ak::event::SubscriberID subscribeConfigSave(const std::function<void(ConfigSaveEvent&)>& callback);
		template<typename func_t> inline ak::event::SubscriberID subscribeConfigSave(const func_t& callback) { return subscribeConfigSave(std::function<void(ConfigSaveEvent&)>(callback)); }

		void unsubscribeConfigSave(ak::event::Subscription& subscriber);

	}
}

#endif
