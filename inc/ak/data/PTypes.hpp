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

#ifndef AK_DATA_PTYPES_HPP_
#define AK_DATA_PTYPES_HPP_

#include <ak/PrimitiveTypes.hpp>

namespace akd {
	enum PTreeNodeType : uint16 {
		// //////////////// //
		//  Concrete types  //
		// //////////////// //
		PTREE_DNE  = 0x0001,
		PTREE_NULL = 0x0002,

		PTREE_PATH = 0x0004,
		PTREE_ARRAY = 0x0008,

		PTREE_SIGNED_INT  = 0x0010,
		PTREE_UNSIGNED_INT = 0x0020,

		PTREE_FP_SINGLE = 0x0040,
		PTREE_FP_DOUBLE = 0x0080,
		PTREE_STRING = 0x0100

	};

	template<PTreeNodeType target> constexpr inline bool isNodeType(PTreeNodeType val) {
		return (val & target) == target;
	}
}

#endif
