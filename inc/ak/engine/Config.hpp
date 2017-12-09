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

namespace ak {
	namespace engine {
		AK_DEFINE_EVENT(RegenerateConfigEvent, ak::data::PValue&, false);
		AK_DEFINE_EVENT(LoadConfigEvent,  ak::data::PValue&, false);
		AK_DEFINE_EVENT(SaveConfigEvent,  ak::data::PValue&, false);
		AK_DEFINE_EVENT(SetConfigEvent,  const ak::data::PValue&, false);

		const ak::event::DispatcherProxy<RegenerateConfigEvent>& regenerateConfigDispatch();
		const ak::event::DispatcherProxy<LoadConfigEvent>& loadConfigDispatch();
		const ak::event::DispatcherProxy<SaveConfigEvent>& saveConfigDispatch();
		const ak::event::DispatcherProxy<SetConfigEvent>& setConfigDispatch();

		const ak::data::PValue& config();
		void setConfig(const ak::data::PValue& nConfig);

		void regenerateConfig();
		bool loadConfig();
		bool saveConfig();
	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_ENGINE_ALIAS_DISABLE))
namespace ake = ak::engine;
#endif

#endif
