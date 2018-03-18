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

#ifndef STX_FILESYSTEM_HPP_
#define STX_FILESYSTEM_HPP_

#if __has_include("filesystem")
#	include <filesystem>
#elif defined(AK_ECLIPSE_INDEXER_ENABLER) || __has_include("experimental/filesystem")
#	include <experimental/filesystem>
#else
#	error "Unsupported environement. Missing filesystem"
#endif

namespace std {
	namespace experimental {
		namespace filesystem {

		}
	}
	namespace filesystem {

	}
}

namespace stx {
	namespace filesystem {
		using namespace std::experimental::filesystem;
		using namespace std::filesystem;
	}
}


#endif
