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

namespace ake {
	AK_DEFINE_EVENT(RegenerateConfigEvent, akd::PValue&, false);
	AK_DEFINE_EVENT(LoadConfigEvent,  akd::PValue&, false);
	AK_DEFINE_EVENT(SaveConfigEvent,  akd::PValue&, false);
	AK_DEFINE_EVENT(SetConfigEvent,  const akd::PValue&, false);

	const akev::DispatcherProxy<RegenerateConfigEvent>& regenerateConfigDispatch();
	const akev::DispatcherProxy<LoadConfigEvent>& loadConfigDispatch();
	const akev::DispatcherProxy<SaveConfigEvent>& saveConfigDispatch();
	const akev::DispatcherProxy<SetConfigEvent>& setConfigDispatch();

	const akd::PValue& config();
	void setConfig(const akd::PValue& nConfig);

	void regenerateConfig();
	bool loadConfig();
	bool saveConfig();
}

#endif
