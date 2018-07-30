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
#ifndef AK_DATA_PVALUEFILE_HPP_
#define AK_DATA_PVALUEFILE_HPP_

#include <optional>

#include <ak/data/PValue.hpp>
#include <ak/filesystem/Path.hpp>

namespace akd {
	template<typename type_t, typename callback_t> bool deserializeFromFile(type_t& dst, const akfs::Path& filepath, callback_t callback) {
		akd::PValue data = callback(filepath);
		return deserialize(dst, data);
	}

	template<typename type_t, typename callback_t> std::optional<type_t> tryDeserializeFromFile(const akfs::Path& filepath, callback_t callback) {
		akd::PValue data = callback(filepath);
		return tryDeserialize<type_t>(data);
	}

	template<typename type_t, typename callback_t> type_t deserializeFromFile(const akfs::Path& filepath, callback_t callback) {
		akd::PValue data = callback(filepath);
		return deserialize<type_t>(data);
	}
}



#endif /* AK_DATA_PVALUEFILE_HPP_ */
