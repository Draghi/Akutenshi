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

#ifndef AK_EVENT_EVENT_HPP_
#define AK_EVENT_EVENT_HPP_

#include <AkEngine/event/Dispatcher.hpp>
#include <AkEngine/event/Util.hpp>
#include <string_view>

#define AK_DEFINE_EVENT(eventName, dataType, canCancel) \
	namespace AK_DEFINE_EVENT_INTERNAL { \
		static inline constexpr std::string_view getNameOf ## eventName () { \
			return std::string_view(#eventName, sizeof(#eventName) - 1); \
		} \
		using eventName = ::akev::Event<dataType, canCancel, getNameOf ## eventName>; \
	} \
	class eventName final : public AK_DEFINE_EVENT_INTERNAL:: eventName { using AK_DEFINE_EVENT_INTERNAL:: eventName ::Event; }

namespace akev {

	namespace internal {
		class IEvent {
			IEvent(const IEvent&) = delete;
			IEvent& operator=(const IEvent&) = delete;
			public:
				IEvent() = default;
				virtual ~IEvent() = default;

				virtual bool cancel() = 0;
				virtual bool isCanceled() const = 0;

				virtual akev::EventID id() const = 0;
				virtual std::string_view name() const = 0;
				virtual bool isCancelable() const = 0;
		};
	}

	using EventNameFuncSig = std::string_view(*)();

	template<typename data_t, bool isCancelableVal, EventNameFuncSig name_f> class Event : public internal::IEvent {
		template<typename> friend class Dispatcher;
		public:
			static constexpr const std::string_view EVENT_NAME = name_f();
			static constexpr akev::EventID EVENT_ID = akev::calculateEventID(EVENT_NAME);

		private:
			data_t m_data;
			bool m_canceled;

		public:
			Event(const data_t& eventData) : m_data(eventData), m_canceled(false) {}
			virtual ~Event() = default;

			data_t& data() { return m_data; }
			const data_t& data() const { return m_data; }

			virtual bool cancel() { m_canceled = true; return isCancelable(); }
			virtual bool isCanceled() const { return !isCancelable() && m_canceled; }

			virtual akev::EventID id() const { return EVENT_ID; }
			virtual std::string_view name() const { return EVENT_NAME; }
			virtual bool isCancelable() const { return isCancelableVal; }

	};

	template<bool isCancelableVal, EventNameFuncSig name_f> class Event<void, isCancelableVal, name_f> : public internal::IEvent {
		template<typename> friend class Dispatcher;
		public:
			static constexpr const std::string_view EVENT_NAME = name_f();
			static constexpr akev::EventID EVENT_ID = akev::calculateEventID(EVENT_NAME);

		private:
			bool m_canceled;

		public:
			Event() : m_canceled(false) {}
			virtual ~Event() = default;

			virtual bool cancel() { m_canceled = true; return isCancelable(); }
			virtual bool isCanceled() const { return !isCancelable() && m_canceled; }

			virtual akev::EventID id() const { return EVENT_ID; }
			virtual std::string_view name() const { return EVENT_NAME; }
			virtual bool isCancelable() const { return isCancelableVal; }

	};

}

#endif
