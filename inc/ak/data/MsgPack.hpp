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

#ifndef AK_DATA_MSGPACK_HPP_
#define AK_DATA_MSGPACK_HPP_

#include <vector>

#include <ak/filesystem/Path.hpp>
#include <ak/PrimitiveTypes.hpp>

namespace akd {
	class PValue;
} /* namespace akd */

namespace akd {
	std::vector<uint8> toMsgPack(const akd::PValue& src);
	bool fromMsgPack(akd::PValue& dest, const std::vector<uint8>& msgPackStream);

	bool toMsgPackFile(const akd::PValue& src, const akfs::Path& filepath, bool compress = true, bool overwrite = true);
	akd::PValue fromMsgPackFile(const akfs::Path& filepath, bool decompress);
}

#endif
