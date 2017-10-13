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

#ifndef AK_EVENT_EVENT_HPP_
#define AK_EVENT_EVENT_HPP_

#include <ak/event/Util.hpp>
#include <ak/String.hpp>
#include <string>
#include <string_view>

#define AK_IMPLEMENT_EVENT(eventName, isCancelableVal) \
	public: static constexpr const std::string_view EVENT_NAME = std::string_view(eventName, ak::clen(eventName)); \
	public: static constexpr ak::event::EventID EVENT_ID = ak::event::calculateEventID(EVENT_NAME); \
	public: virtual ak::event::EventID id() const { return EVENT_ID; } \
	public: virtual std::string name() const { return std::string(EVENT_NAME); } \
	public: virtual bool isCancelable() const { return isCancelableVal; }

namespace ak {
	namespace event {

		class Event {
			template<typename> friend class Dispatcher;

			Event(const Event&) = delete;
			Event& operator=(const Event&) = delete;

			private:
				bool m_canceled;
			public:
				Event() : m_canceled(false) {}
				virtual ~Event() = default;

				bool cancel() { m_canceled = true; return isCancelable(); }
				bool isCanceled() const { return !isCancelable() && m_canceled; }

				virtual EventID id() const = 0;
				virtual std::string name() const = 0;
				virtual bool isCancelable() const = 0;

		};

	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_EVENT_ALIAS_DISABLE))
namespace akev = ak::event;
#endif

#endif
