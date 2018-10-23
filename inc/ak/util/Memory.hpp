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

#ifndef AK_UTIL_MEMORY_HPP_
#define AK_UTIL_MEMORY_HPP_

#include <algorithm>
#include <memory>
#include <cstring>
#include <type_traits>

#include <ak/Log.hpp>
#include <ak/PrimitiveTypes.hpp>

namespace aku {

	template<typename type_t, typename type2_t, typename... vargs_t> std::unique_ptr<type_t> make_unique(vargs_t&&... vargs) {
		return std::unique_ptr<type_t>(new type2_t(std::forward<vargs_t>(vargs)...));
	}

	template<typename type_t> akSize memcpy(type_t* dst, const type_t* src, akSize count) {
		static_assert(std::is_pod<type_t>(), "type_t must be POD data type.");
		std::memcpy(dst, src, count*sizeof(type_t));
		return count;
	}

	template<typename type_t> akSize memmove(type_t* dst, const type_t* src, akSize count) {
		static_assert(std::is_pod<type_t>(), "type_t must be POD data type.");
		std::memmove(dst, src, count*sizeof(type_t));
		return count;
	}

	template<typename type_t> akSize memset(type_t* dst, const type_t& val, akSize count) {
		static_assert(std::is_pod<type_t>(), "type_t must be POD data type.");
		std::fill_n(dst, count, val);
		return count;
	}

	template<typename type_t> akSize memwrp(
		      type_t* dst,  akSize dstSize, akSize dstOffset,
		const type_t* src,  akSize srcSize, akSize srcOffset, akSize count) {

		akSize written = 0;
		while(written < count) {
			dstOffset %= dstSize;
			srcOffset %= srcSize;

			akSize copied = aku::memcpy(dst + dstOffset, src + srcOffset, std::min(count - written, std::min(dstSize - dstOffset, srcSize - srcOffset)));

			dstOffset += copied;
			srcOffset += copied;
			written   += copied;
		}

		return written;
	}

	template<typename type_t> akSize memwrp(type_t* dst, akSize dstSize, akSize dstOffset, const type_t* src, akSize count) {
		return memwrp(dst, dstSize, dstOffset, src, count, 0, count);
	}

	template<typename type_t> akSize memwrp(type_t* dst, const type_t* src, akSize srcSize, akSize srcOffset, akSize count) {
		return memwrp(dst, count, 0, src, srcSize, srcOffset, count);
	}

}

#endif /* AK_UTIL_MEMORY_HPP_ */
