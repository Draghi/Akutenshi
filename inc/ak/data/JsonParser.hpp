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

#ifndef AK_DATA_JSON_PVALUEPARSER_HPP_
#define AK_DATA_JSON_PVALUEPARSER_HPP_

#include <ak/data/PValue.hpp>
#include <string>

namespace ak {
	namespace data {
		std::string serializeJson(const ak::data::PValue& src, bool pretty = false);
		bool deserializeJson(ak::data::PValue& dest, std::istream& jsonStream);
	}
}


#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_DATA_ALIAS_DISABLE))
namespace akd = ak::data;
#endif

#endif
