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

#ifndef AK_ENGINE_DATA_HASH_HPP_
#define AK_ENGINE_DATA_HASH_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <type_traits>

namespace akd {
	/**
	 * Calculates the 64-bit FNV-1A hash for given sized array
	 * @param data The data to hash
	 * @param count The number of elements in the data
	 * @return The calculated FNV-1A hash of the array
	 */
	template<typename type_t> constexpr uint64 hash64FNV1A(const type_t* data, akSize count) {
		static_assert(std::is_fundamental<type_t>(), "Data array must be a fundamental type.");

		constexpr uint64 offsetBasis = 0xcbf29ce484222325u;
		constexpr uint64 prime = 1099511628211u;

		akSize totalCount = sizeof(type_t)*count;
		const char* rawData = static_cast<const char*>(data);

		uint64 hash = offsetBasis;
		for(akSize i = 0; i < totalCount; i++) {
			hash ^= static_cast<uint8>(rawData[i]);
			hash *= prime;
		}

		return hash;
	}

	/**
	 * Calculates the 32-bit FNV-1A hash for given sized array
	 * @param data The data to hash
	 * @param count The number of elements in the data
	 * @return The calculated FNV-1A hash of the array
	 */
	template<typename type_t> constexpr uint32 hash32FNV1A(const type_t* data, akSize count) {
		static_assert(std::is_fundamental<type_t>(), "Data array must be a fundamental type.");

		constexpr uint32 offsetBasis = 0x811c9dc5u;
		constexpr uint32 prime = 16777619u;

		akSize totalCount = sizeof(type_t)*count;
		const char* rawData = static_cast<const char*>(data);

		uint32 hash = offsetBasis;
		for(akSize i = 0; i < totalCount; i++) {
			hash ^= static_cast<uint8>(rawData[i]);
			hash *= prime;
		}

		return hash;
	}
}

#endif
