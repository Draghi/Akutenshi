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

#ifndef AK_EVENT_SUBSCRIPTION_HPP_
#define AK_EVENT_SUBSCRIPTION_HPP_

#include <ak/event/Util.hpp>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ak {
	namespace event {

		class Subscription final {
			template<typename> friend class Dispatcher;
			Subscription(const Subscription&) = delete;
			Subscription& operator=(const Subscription&) = delete;
			private:
				std::unordered_set<internal::IDispatcher*> m_dispatchers;

			protected:
				bool Subscriber_AddDispatcher(internal::IDispatcher* dispatcher) {
					return m_dispatchers.insert(dispatcher).second;
				}

				bool Subscriber_RemoveDispatcher(internal::IDispatcher* dispatcher) {
					return m_dispatchers.erase(dispatcher) > 0;
				}

			public:
				Subscription() = default;

				virtual ~Subscription() {
					for(auto iter = m_dispatchers.begin(); iter != m_dispatchers.end(); iter++) {
						(*iter)->unsubscribe(*this);
					}
				}

		};

	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_EVENT_ALIAS_DISABLE))
namespace akev = ak::event;
#endif

#endif
