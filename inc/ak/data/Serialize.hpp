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
#ifndef AK_DATA_SERIALIZE_HPP_
#define AK_DATA_SERIALIZE_HPP_

#include <stddef.h>
#include <array>
#include <deque>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ak/data/PValue.hpp>
#include <ak/filesystem/Path.hpp>
#include <ak/Log.hpp>
#include <ak/PrimitiveTypes.hpp>

// ///////////// //
// // Helpers // //
// ///////////// //
namespace akd {
	template<typename type_t> PValue serialize(const type_t& val) {
		PValue result;
		serialize(result, val);
		return result;
	}

	template<typename type_t> std::optional<type_t> tryDeserialize(const PValue& root) {
		type_t result;
		return deserialize(result, root) ?  std::optional<type_t>{result} :  std::optional<type_t>{};
	}

	template<typename type_t> bool tryDeserialize(type_t& dst, const PValue& root, const type_t& def) {
		auto result = tryDeserialize<type_t>(root);
		dst = result.value_or(def);
		return result;
	}

	template<typename type_t> type_t tryDeserialize(const PValue& root, const type_t& def) {
		auto result = tryDeserialize<type_t>(root);
		if (result) return *result;
		else return def;
	}

	template<typename type_t> type_t deserialize(const PValue& root) {
		auto result = tryDeserialize<type_t>(root);
		if (result) return *result;
		else throw std::logic_error("Failed to deserialize value.");
	}
}

// ///////// //
// // STL // //
// ///////// //

// Vector
namespace akd {
	template<typename type_t, typename alloc_t> void serialize(akd::PValue& dst, const std::vector<type_t, alloc_t>& val) {
		if (!dst.isArr()) dst.setArr();
		for(akSize i = 0; i < val.size(); i++) {
			serialize(dst[i], val[i]);
		}
	}

	template<typename type_t, typename alloc_t> bool deserialize(std::vector<type_t, alloc_t>& dst, const akd::PValue& val) {
		if (!val.isArr()) return false;
		std::vector<type_t, alloc_t> result;
		for(akSize i = 0; i < val.size(); i++) {
			if (i >= result.size()) result.push_back(type_t());
			if (!deserialize(result[i], val[i])) {
				akl::Logger("Pair").error("Failed to deserialize entry: ", i);
				return false;
			}
		}
		dst = result;
		return true;
	}
}

// Array
namespace akd {
	template<typename type_t, size_t l> void serialize(akd::PValue& dst, const std::array<type_t, l>& val) {
		if (!dst.isArr()) dst.setArr();
		for(akSize i = 0; i < val.size(); i++) {
			serialize(dst[i], val[i]);
		}
	}

	template<typename type_t, size_t l> bool deserialize(std::array<type_t, l>& dst, const akd::PValue& val) {
		if (!val.isArr()) return false;
		std::array<type_t, l> result;
		for(akSize i = 0; i < val.size(); i++) {
			if (!deserialize(result[i], val[i])) {
				akl::Logger("Array").error("Failed to deserialize entry: ", i);
				return false;
			}
		}
		dst = result;
		return true;
	}
}

// Deque
namespace akd {
	template<typename type_t, typename alloc_t> void serialize(akd::PValue& dst, const std::deque<type_t, alloc_t>& val) {
		if (!dst.isArr()) dst.setArr();
		for(akSize i = 0; i < val.size(); i++) {
			serialize(dst[i], val[i]);
		}
	}

	template<typename type_t, typename alloc_t> bool deserialize(std::deque<type_t, alloc_t>& dst, const akd::PValue& val) {
		if (!val.isArr()) return false;
		std::deque<type_t, alloc_t> result;
		for(akSize i = 0; i < val.size(); i++) {
			if (i >= result.size()) result.push_back(type_t());
			if (!deserialize(result[i], val[i])) {
				akl::Logger("Pair").error("Failed to deserialize entry: ", i);
				return false;
			}
		}
		dst = result;
		return true;
	}
}

// Pair
namespace akd {
	template<typename type_t, typename type2_t> void serialize(akd::PValue& dst, const std::pair<type_t, type2_t>& val) {
		if (!dst.isArr()) dst.setArr();
		if (dst.size() != 2) dst.getArr().resize(2);
		serialize(dst[0], val.first);
		serialize(dst[1], val.second);
	}

	template<typename type_t, typename type2_t> bool deserialize(std::pair<type_t, type2_t>& dst, const akd::PValue& val) {
		if (!val.isNull() && !val.isArr()) {
			akl::Logger("Pair").error("PValue is not null or an array.");
			return false;
		}

		std::pair<type_t, type2_t> result;

		if (!deserialize(result.first, val.atOrDef(0))) {
			akl::Logger("Pair").error("Failed to deserialize entry: ", 0);
			return false;
		}

		if (!deserialize(result.second, val.atOrDef(1))) {
			akl::Logger("Pair").error("Failed to deserialize entry: ", 1);
			return false;
		}

		dst = result;

		return true;
	}
}

// Map
namespace akd {
	template<typename type_t, typename alloc_t> void serialize(akd::PValue& dst, const std::map<std::string, type_t>& val) {
		if (!dst.isArr()) dst.setArr();
		for(const auto& entry : val) {
			serialize(dst[entry.first.asStr()], entry.second);
		}
	}

	template<typename type_t, typename type2_t> bool deserialize(std::map<type_t, type2_t>& dst, const akd::PValue& val) {
		if (!val.isObj()) return false;

		std::map<type_t, type2_t> result;
		for(auto& entry : dst.asObj()) result.emplace(deserialize<type_t>(entry.first), deserialize<type2_t>(entry.second));

		dst = result;
		return true;
	}
}

// Unordered Map
namespace akd {
	template<typename type_t, typename alloc_t> void serialize(akd::PValue& dst, const std::unordered_map<std::string, type_t>& val) {
		if (!dst.isArr()) dst.setArr();
		for(const auto& entry : val) {
			serialize(dst[entry.first.asStr()], entry.second);
		}
	}

	template<typename type_t, typename type2_t> bool deserialize(std::unordered_map<type_t, type2_t>& dst, const akd::PValue& val) {
		if (!val.isObj()) return false;

		std::unordered_map<type_t, type2_t> result;
		for(auto& entry : dst.asObj()) result.emplace(deserialize<type_t>(entry.first), deserialize<type2_t>(entry.second));

		dst = result;
		return true;
	}
}

// Optional
namespace akd {
	template<typename type_t> void serialize(akd::PValue& dst, const std::optional<type_t>& val) {
		if (!val) dst.setNull();
		else serialize(dst, *val);
	}

	template<typename type_t> bool deserialize(std::optional<type_t>& dst, const akd::PValue& val) {
		if (val.isNull()) {
			dst = {};
			return true;
		}
		type_t tmpVal;
		if (!deserialize(tmpVal, val)) return false;
		dst = std::move(tmpVal);
		return true;
	}
}

// /////////////////////// //
// // PValue Primitives // //
// /////////////////////// //
namespace akd {
	namespace internal {
		template<typename type_t> void methodSerialize(PValue& src, const type_t& dst) {
			src = PValue::from<type_t>(dst);
		}
		template<typename type_t> bool methodDeserialize(type_t& dst, const PValue& src) {
			auto tmp = src.tryAs<type_t>();
			if (tmp) dst = *tmp;
			return tmp.has_value();
		}
	}

	inline void serialize(PValue& dst, const PValue::obj_t& src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(PValue::obj_t& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const PValue::arr_t& src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(PValue::arr_t& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const PValue::bin_t& src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(PValue::bin_t& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const PValue::str_t& src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(PValue::str_t& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const int8&  src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(int8& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const int16& src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(int16& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const int32& src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(int32& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const int64& src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(int64& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const uint8&  src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(uint8& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const uint16& src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(uint16& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const uint32& src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(uint32& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const uint64& src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(uint64& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const fpSingle&   src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(fpSingle& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const fpDouble&   src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(fpDouble& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }

	inline void serialize(PValue& dst, const PValue::bool_t&  src) { return internal::methodSerialize(dst, src); }
	inline bool deserialize(PValue::bool_t& dst, const PValue& src) { return internal::methodDeserialize(dst, src); }
}

#endif /* AK_DATA_SERIALIZE_HPP_ */
