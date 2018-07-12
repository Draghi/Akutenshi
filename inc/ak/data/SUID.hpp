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

#ifndef AK_DATA_SUID_HPP_
#define AK_DATA_SUID_HPP_

#include <ak/data/PValue.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <cstddef>
#include <stdexcept>
#include <unordered_map>

namespace akd {

	struct SUID {
		SUID(const SUID&) = default;
		SUID& operator=(const SUID& o) = default;

		uint64 high, low;

		SUID() : high(0), low(0) {}
		SUID(uint64 h, uint64 l) : high(h), low(l) {}

		bool operator==(const SUID& o) const { return (high == o.high) && (low == o.low); }
		bool operator <(const SUID& o) const { return (high < o.high) || ((high == o.high) && (low < o.low)); }
		bool operator >(const SUID& o) const { return (high > o.high) || ((high == o.high) && (low > o.low)); }

		bool operator!=(const SUID& o) const { return (high != o.high) || (low != o.low); }
		bool operator<=(const SUID& o) const { return (high <  o.high) || ((high == o.high) && (low <= o.low)); }
		bool operator>=(const SUID& o) const { return (high >  o.high) || ((high == o.high) && (low >= o.low)); }
	};

	template<typename rand_t> SUID generateSUID(rand_t& randSource) {
		static_assert(sizeof(typename rand_t::int_type) == 8, "Rand source must provide 64 bits of randomness");
		return SUID(randSource.nextInt(), randSource.nextInt());
	}

	template<typename rand_t> SUID generateSUID(uint8 seed, rand_t& randSource) {
		static_assert(sizeof(typename rand_t::int_type) == 8, "Rand source must provide 64 bits of randomness");
		return SUID(
			((randSource.nextInt() % 1152921504606846976) & 0x00FFFFFFFFFFFFFF) | (static_cast<uint64>(seed) << 48), // 48 Rand-Bits + 8 Identification Bits (ID Bits to reduce collision chance on decenteralized system)
			randSource.nextInt()                                                                                     // 64 Rand-Bits
		);
	}

	inline void serialize(akd::PValue& dest, const SUID& val) {
		dest.setArr();
		dest[0].setUInt(val.high);
		dest[1].setUInt(val.low);
	}

	inline bool deserialize(SUID& dest, const akd::PValue& val) {
		try {
			dest = SUID(val[0].as<uint64>(), val[1].as<uint64>());
			return true;
		} catch(const std::logic_error& /*e*/) {
			return false;
		}
	}
}

namespace std {
	template<> struct hash<akd::SUID> {
		size_t operator()(const akd::SUID& v) const {
			return v.high ^ v.low;
		}
	};
}

#endif
