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

#ifndef AK_EVENT_UTIL_HPP_
#define AK_EVENT_UTIL_HPP_

#include <ak/data/Hash.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <string_view>

namespace ak {
	namespace event {

		using EventID = uint64;
		class Subscription;

		namespace internal {
			class IDispatcher {
				IDispatcher(const IDispatcher&) = delete;
				IDispatcher& operator=(const IDispatcher&) = delete;
				public:
					IDispatcher() = default;
					virtual ~IDispatcher() = default;

					virtual void unsubscribe(Subscription&) = 0;
			};
		}

		inline constexpr EventID calculateEventID(const std::string_view& eventName) {
			return ak::data::calculateFNV1AHash(eventName.data(), eventName.size());
		}

	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_EVENT_ALIAS_DISABLE))
namespace akev = ak::event;
#endif

#endif
