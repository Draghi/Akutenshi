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

#ifndef AK_ENGINE_DATA_SERIALIZE_HPP_
#define AK_ENGINE_DATA_SERIALIZE_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/String.hpp>
#include <akengine/data/PValue.hpp>
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

namespace akd {
	namespace internal {
		// Break circular dependency
		void logError(const std::string& name, const std::string& message);
		template<typename... vargs_t> void logError(const std::string& name, const vargs_t&... vargs) { logError(name, akc::buildString(vargs...));  }
	}
}

// /////////////////// //
// // Introspection // //
// /////////////////// //
namespace akd {
	template<typename type_t> constexpr akd::PType serializesTo();

	template<> constexpr akd::PType serializesTo<PValue::obj_t>() { return PType::Object; }
	template<> constexpr akd::PType serializesTo<PValue::arr_t>() { return PType::Array; }
	template<> constexpr akd::PType serializesTo<PValue::str_t>() { return PType::String; }
	template<> constexpr akd::PType serializesTo<PValue::bin_t>() { return PType::Binary; }

	template<> constexpr akd::PType serializesTo<uint8>() { return PType::Unsigned; }
	template<> constexpr akd::PType serializesTo<uint16>() { return PType::Unsigned; }
	template<> constexpr akd::PType serializesTo<uint32>() { return PType::Unsigned; }
	template<> constexpr akd::PType serializesTo<uint64>() { return PType::Unsigned; }

	template<> constexpr akd::PType serializesTo<int8>() { return PType::Signed; }
	template<> constexpr akd::PType serializesTo<int16>() { return PType::Signed; }
	template<> constexpr akd::PType serializesTo<int32>() { return PType::Signed; }
	template<> constexpr akd::PType serializesTo<int64>() { return PType::Signed; }

	template<> constexpr akd::PType serializesTo<fpSingle>() { return PType::Decimal; }
	template<> constexpr akd::PType serializesTo<fpDouble>() { return PType::Decimal; }
}

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
				internal::logError("Pair", "Failed to deserialize entry: ", i);
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
				internal::logError("Array", "Failed to deserialize entry: ", i);
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
				internal::logError("Pair", "Failed to deserialize entry: ", i);
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
			internal::logError("Pair", "PValue is not null or an array.");
			return false;
		}

		std::pair<type_t, type2_t> result;

		if (!deserialize(result.first, val.atOrDef(0))) {
			internal::logError("Pair", "Failed to deserialize entry: ", 0);
			return false;
		}

		if (!deserialize(result.second, val.atOrDef(1))) {
			internal::logError("Pair", "Failed to deserialize entry: ", 1);
			return false;
		}

		dst = result;

		return true;
	}
}

// Map
namespace akd {
	template<typename type_t, typename type2_t> void serialize(akd::PValue& dst, const std::map<type_t, type2_t>& val) {
		if constexpr (serializesTo<type_t>() == PType::String) {
			if (!dst.isObj()) dst.setObj();
			for(const auto& entry : val) serialize(dst[serialize(entry.first).getStr()], entry.second);
		} else {
			if (!dst.isArr()) dst.setArr();
			for(const auto& entry : val) dst.getArr().push_back(serialize(entry));
		}
	}

	template<typename type_t, typename type2_t> bool deserialize(std::map<type_t, type2_t>& dst, const akd::PValue& val) {
		std::map<type_t, type2_t> result;
		if constexpr (serializesTo<type_t>() == PType::String) {
			if (!val.isObj()) return false;
			for(const auto& entry : val.getObj()) result.emplace(deserialize<type_t>(akd::PValue::from(entry.first)), deserialize<type2_t>(entry.second));
		} else {
			if (!val.isArr()) return false;
			for(const auto& entry : val.getArr()) {
				std::pair<type_t, type2_t> entryVal;
				if (!deserialize(entryVal, entry)) {
					internal::logError("Map", "Could not deserialize entry in non-string map");
					return false;
				}
				if (!result.insert(entryVal).second) {
					internal::logError("Map", "Key conflict in non-string map");
					return false;
				}
			}
		}
		dst = result;
		return true;
	}
}

// Unordered Map
namespace akd {
	template<typename type_t, typename type2_t> void serialize(akd::PValue& dst, const std::unordered_map<type_t, type2_t>& val) {
		if constexpr (serializesTo<type_t>() == PType::String) {
			if (!dst.isObj()) dst.setObj();
			for(const auto& entry : val) serialize(dst[serialize(entry.first).getStr()], entry.second);
		} else {
			if (!dst.isArr()) dst.setArr();
			for(const auto& entry : val) dst.getArr().push_back(serialize(entry));
		}
	}

	template<typename type_t, typename type2_t> bool deserialize(std::unordered_map<type_t, type2_t>& dst, const akd::PValue& val) {
		std::unordered_map<type_t, type2_t> result;
		if constexpr (serializesTo<type_t>() == PType::String) {
			if (!val.isObj()) return false;
			for(const auto& entry : val.getObj()) result.emplace(deserialize<type_t>(akd::PValue::from(entry.first)), deserialize<type2_t>(entry.second));
		} else {
			if (!val.isArr()) return false;
			for(const auto& entry : val.getArr()) {
				std::pair<type_t, type2_t> entryVal;
				if (!deserialize(entryVal, entry)) {
					internal::logError("UnorderedMap", "Could not deserialize entry in non-string map");
					return false;
				}
				if (!result.insert(entryVal).second) {
					internal::logError("UnorderedMap", "Key conflict in non-string map");
					return false;
				}
			}
		}
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

#endif /* AK_ENGINE_DATA_SERIALIZE_HPP_ */
