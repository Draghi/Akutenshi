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

#ifndef AK_EVENT_DISPATCHER_HPP_
#define AK_EVENT_DISPATCHER_HPP_

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <string_view>
#include <vector>

#include <ak/container/SlotMap.hpp>
#include <ak/event/Util.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/thread/CurrentThread.hpp>

namespace akev {
	using SubscriberID = akc::SlotID;

	template<typename event_t> class Dispatcher final {
		Dispatcher(const Dispatcher&) = delete;
		Dispatcher& operator=(const Dispatcher&) = delete;
		public:
			using event_type = event_t;
			using callback_s = void(event_type&);
			using callback_t = std::function<void(event_type&)>;

		private:
			akc::SlotMap<callback_t> m_callbacks;
			akt::CurrentThread& m_mediator;

			std::atomic<int> m_sendCounter;
			std::vector<SubscriberID> m_freelist;

		public:
			Dispatcher() : m_mediator(akt::current()) {}
			Dispatcher(Dispatcher&&) = default;
			Dispatcher& operator=(Dispatcher&&) = default;

			~Dispatcher() {}

			template<typename func_t> SubscriberID subscribe(const func_t& func) { return m_callbacks.insert(callback_t(func)).first; }

			void unsubscribe(SubscriberID subscriber) {
				if (m_sendCounter) m_freelist.push_back(subscriber);
				else m_callbacks.erase(subscriber);
			}

			void send(event_t& event) {
				++m_sendCounter;
				event.m_canceled = false;
				for(const auto& callback : m_callbacks) { callback(event); if (event.isCanceled()) return; }
				if (--m_sendCounter == 0) {
					auto localCopy = std::move(m_freelist); // @todo Might be a race condition here... It'd be hard as hell to hit... Probably...
					for(auto id : localCopy) unsubscribe(id);
				}
			}

			template<typename... vargs_t> void sendEmplace(vargs_t... vargs) {
				event_t event{std::forward<vargs_t...>(vargs...)};
				send(event);
			}

			void mediate(const std::shared_ptr<event_type>& event) { m_mediator.schedule([this, event](){send(*event);}); }
			template<typename... vargs_t> void mediate(vargs_t... vargs) { mediate(std::make_shared<event_type>(std::forward(vargs)...)); }

			akSize subscriberCount() const { return m_callbacks.size(); }

			EventID id() { return event_t::EVENT_ID; }
			std::string_view name() { return event_t::EVENT_NAME; }
	};

	template<typename event_t> class DispatcherProxy final {
		public:
			using dispatcher_type = Dispatcher<event_t>;
			using callback_s = typename dispatcher_type::callback_s;
			using callback_t = typename dispatcher_type::callback_t;

		private:
			dispatcher_type& m_dispatcher;

		public:
			DispatcherProxy(dispatcher_type& dispatcher) : m_dispatcher(dispatcher) {}
			~DispatcherProxy() = default;

			template<typename func_t> SubscriberID subscribe(const func_t& func) const { return m_dispatcher.subscribe(func); }
			void unsubscribe(SubscriberID subscriber) const { return m_dispatcher.unsubscribe(subscriber); }

			EventID id() const { return m_dispatcher.id(); }
			std::string_view name() const { return m_dispatcher.name(); }
	};


}


#endif
