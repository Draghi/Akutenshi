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

#ifndef AK_EVENT_DISPATCHER_HPP_
#define AK_EVENT_DISPATCHER_HPP_

#include <ak/event/Subscription.hpp>
#include <ak/event/Util.hpp>
#include <ak/thread/CurrentThread.hpp>
#include <ak/thread/RecursiveSpinlock.hpp>
#include <ak/thread/Thread.hpp>
#include <ordered_map.h>
#include <stddef.h>
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace ak {
	namespace event {

		using SubscriberID = uint32;

		template<typename event_t> class Dispatcher final : public internal::IDispatcher {
			public:
				using callback_s = void(event_t&);
				using callback_t = std::function<void(event_t&)>;

				SubscriberID m_id;
				tsl::ordered_map<SubscriberID, callback_t> m_callbacks;
				tsl::ordered_map<Subscription*, SubscriberID> m_subscriptions;

				ak::thread::CurrentThread& m_mediator;

			public:
				Dispatcher() : m_id(0), m_mediator(ak::thread::current()) {}

				virtual ~Dispatcher() {
					auto localCopy = m_subscriptions;
					for(auto iter = localCopy.begin(); iter != localCopy.end(); iter++) {
						iter->first->Subscriber_RemoveDispatcher(this);
					}
				}

				SubscriberID subscribe(Subscription& subscriber, const callback_t& func) {
					auto iter = m_subscriptions.find(&subscriber);
					if (iter != m_subscriptions.end()) return 0;
					if (!subscriber.Subscriber_AddDispatcher(this)) return 0;

					auto id = ++m_id;
					m_subscriptions.insert(std::make_pair(&subscriber, id));
					m_callbacks.insert(std::make_pair(id, func));

					return id;
				}

				virtual void unsubscribe(Subscription& subscriber) {
					auto iter = m_subscriptions.find(&subscriber);
					if (iter == m_subscriptions.end()) return;

					m_callbacks.erase(iter->second);
					m_subscriptions.erase(iter);

					subscriber.Subscriber_RemoveDispatcher(this);
				}

				SubscriberID subscribe(const callback_t& func) {
					auto id = ++m_id;
					m_callbacks.insert(std::make_pair(id, func));
					return id;
				}

				void unsubscribe(SubscriberID subscriber) {
					m_callbacks.erase(subscriber);
				}

				ssize_t send(event_t& event) {
					auto localCopy = m_callbacks;

					event.m_canceled = false;

					ssize_t dispatchCount = 0;
					for(auto iter = localCopy.begin(); iter != localCopy.end(); iter++, dispatchCount++) {
						iter->second(event);
						if (event.isCanceled()) return dispatchCount;
					}

					return dispatchCount;
				}

				void mediate(const std::shared_ptr<event_t>& event) { m_mediator.schedule([this, event](){send(*event);}); }

				EventID id() { return event_t::EVENT_ID; }
				std::string_view name() { return std::string_view(event_t::EVENT_NAME, sizeof(event_t::EVENT_NAME)); }

				template<typename func_t> bool subscribe(Subscription& subscriber, const func_t& func) { return subscribe(subscriber, callback_t(func)); }
				template<typename func_t> bool subscribe(const func_t& func) { return subscribe(callback_t(func)); }
				template<typename... vargs_t> void mediate(vargs_t... vargs) { mediate(std::shared_ptr<event_t>(new event_t(vargs...))); }
		};


	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_EVENT_ALIAS_DISABLE))
namespace akev = ak::event;
#endif

#endif
