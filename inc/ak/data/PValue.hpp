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

#ifndef AK_DATA_PVALUE_HPP_
#define AK_DATA_PVALUE_HPP_

#include <algorithm>
#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <ak/data/PVPath.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/util/String.hpp>

namespace akd {

	class PValue;

	enum class TraverseAction : uint8 {
		ObjectStart,
		ObjectEnd,
		ArrayStart,
		ArrayEnd,
		Value,
	};

	enum class PType : uint8 {
		Null,
		Object,
		Array,
		String,
		Signed,
		Unsigned,
		Decimal,
		Boolean,
		Binary,
	};

	void traversePValue(const PValue& cNode, const std::function<void(const PVPath& path, TraverseAction action, const PValue& value)>& callback);

	class PValue final {
		public:
			using null_t = std::nullptr_t;
			using obj_t = std::map<std::string, PValue>;
			using arr_t = std::deque<PValue>;
			using str_t = std::string;
			using sint_t = int64;
			using uint_t = uint64;
			using dec_t = fpDouble;
			using bool_t = bool;
			using bin_t = std::vector<uint8>;

			using traverse_f = std::function<bool(const PVPath&, const PValue&)>;

		private:
			union ValueContainer {
				obj_t oVal;
				arr_t aVal;
				str_t sVal;

				uint_t uVal;
				sint_t iVal;
				dec_t dVal;
				bool_t bVal;

				bin_t binVal;

				ValueContainer() : uVal(0) {}
				~ValueContainer() {}
			};

			PType m_type;
			ValueContainer m_value;

			static PValue& navigate_internal(PValue& cNode, const PVPath& path);
			static const PValue* navigate_internal(const PValue* currentNode, const PVPath& path);

			PValue& setPValue(const PValue& val);

			std::string toDebugString() const {
				if (isNull()) return "<null>";
				if (isObj())  return aku::buildString("<str:", getObj().size(), ">");
				if (isArr())  return aku::buildString("<str:", getArr().size(), ">");
				if (isBin())  return aku::buildString("<str:", getBin().size(), ">");
				if (isStr())  return aku::buildString("<str:", getStr(), ">");
				if (isSInt()) return aku::buildString("<sint:",getSInt(),">");
				if (isUInt()) return aku::buildString("<uint:",getUInt(),">");
				if (isDec())  return aku::buildString("<dec:", getDec(), ">");
				if (isBool()) return aku::buildString("<bool:",getBool(),">");
				return "<error>";
			}

		public:
			template<typename type_t> static PValue from(const type_t& val) { PValue result; result.set<type_t>(val); return result; }

			PValue() : m_type(PType::Null) {}
			PValue(const PValue& val) : m_type(PType::Null) { setPValue(val); }

			~PValue() { setNull(); }

			PValue& operator=(const PValue& val) { setPValue(val); return *this; }

			bool operator==(const PValue& o) const {
				if (m_type != o.m_type) return false;
				switch(m_type) {
					case PType::Null:     return true;
					case PType::Object:   return getObj()  == o.getObj();
					case PType::Array:    return getArr()  == o.getArr();
					case PType::Binary:   return getBin()  == o.getBin();
					case PType::String:   return getStr()  == o.getStr();
					case PType::Signed:   return getSInt() == o.getSInt();
					case PType::Unsigned: return getUInt() == o.getUInt();
					case PType::Decimal:  return getDec()  == o.getDec();
					case PType::Boolean:  return getBool() == o.getBool();
					default: throw std::logic_error("Unhandled data type.");
				}
			}

			bool operator!=(const PValue& o) const { return !(*this == o); }


			// //////////////// //
			// // Navigation // //
			// //////////////// //

			const PValue& at(const std::string& name) const {
				return getObj().at(name);
			}

			PValue& at(const std::string& name) {
				return getObj().at(name);
			}

			PValue& atOrSet(const std::string& name, const PValue& val = PValue()) {
				return getObjOrSet().emplace(name, val).first->second;
			}

			const PValue& atOrDef(const std::string& name, const PValue& val = PValue()) const {
				if (!isObj()) return val;
				auto iter = getObj().find(name);
				return (iter == getObj().end()) ? val : iter->second;
			}

			const PValue& at(akSize id) const {
				return getArr().at(id);
			}

			PValue& at(akSize id) {
				return getArr().at(id);
			}

			PValue& atOrSet(akSize id, const PValue& val = PValue()) {
				auto& arr = getArrOrSet();
				if (id >= arr.size()) arr.resize(id + 1);
				return arr[id] = val;
			}

			const PValue& atOrDef(akSize id, const PValue& val = PValue()) const {
				if (!isArr()) return val;
				return (id < getArr().size()) ? getArr()[id] : val;
			}

			PValue& operator[](const std::string& name) { return atOrSet(name); }
			PValue& operator[](const akSize& id) { return atOrSet(id); }


			bool exists(const std::string& name) const { return (isObj()) && (getObj().find(name) != getObj().end()); }
			bool exists(akSize id) const { return (isArr()) && (id < getArr().size()); }

			const PValue& operator[](const std::string& name) const { return atOrDef(name); }
			const PValue& operator[](const akSize& id) const { return atOrDef(id); }

			// ///////////// //
			// // Dirrect // //
			// ///////////// //

			PValue& setNull();
			PValue& setObj(const obj_t& val = obj_t());
			PValue& setArr(const arr_t& val = arr_t());
			PValue& setStr(const str_t& val = str_t());
			PValue& setSInt(const sint_t& val = sint_t());
			PValue& setUInt(const uint_t& val = uint_t());
			PValue& setDec(const dec_t& val = dec_t());
			PValue& setBool(const bool_t& val =  bool_t());
			PValue& setBin(const bin_t& val = bin_t());
			PValue& setBin(const void* val, akSize size);

			obj_t&  getObj();
			arr_t&  getArr();
			bin_t&  getBin();
			str_t&  getStr();
			sint_t&  getSInt();
			uint_t& getUInt();
			dec_t&  getDec();
			bool_t& getBool();

			obj_t& getObjOrSet(const obj_t& val = obj_t());
			arr_t& getArrOrSet(const arr_t& val = arr_t());
			bin_t& getBinOrSet(const bin_t& val = bin_t());
			bin_t& getBinOrSet(const void* val, akSize size);
			str_t&  getStrOrSet(const str_t& val = str_t());
			sint_t&  getSIntOrSet(const sint_t& val = sint_t());
			uint_t& getUIntOrSet(const uint_t& val = uint_t());
			dec_t&  getDecOrSet(const dec_t& val = dec_t());
			bool_t& getBoolOrSet(const bool_t& val =  bool_t());

			const obj_t& getObjOrDef(const obj_t& val = obj_t()) const;
			const arr_t& getArrOrDef(const arr_t& val = arr_t()) const;
			const bin_t& getBinOrDef(const bin_t& val = bin_t()) const;
			const bin_t& getBinOrDef(const void* val, akSize size) const;
			const str_t&  getStrOrDef(const str_t& val = str_t()) const;
			const sint_t&  getSIntOrDef(const sint_t& val = sint_t()) const;
			const uint_t& getUIntOrDef(const uint_t& val = uint_t()) const;
			const dec_t&  getDecOrDef(const dec_t& val = dec_t()) const;
			const bool_t& getBoolOrDef(const bool_t& val =  bool_t()) const;

			const obj_t& getObj() const;
			const arr_t& getArr() const;
			const bin_t& getBin() const;
			const str_t& getStr() const;
			sint_t getSInt() const;
			uint_t getUInt() const;
			dec_t getDec() const;
			bool_t getBool() const;

			// //////////// //
			// // Values // //
			// //////////// //


			template<typename type_t> bool assign(type_t& dest) const {
				if (isNull()) return false;
				dest = as<type_t>();
				return true;
			}

			template<typename dest_t, typename type_t> bool assign(type_t& dest) const {
				if (isNull()) return false;
				dest = static_cast<type_t>(as<dest_t>());
				return true;
			}

			template<typename type_t> bool tryAssign(std::optional<type_t>& dest) const {
				if (isNull()) return false;
				dest = as<type_t>();
				return true;
			}

			template<typename dest_t, typename type_t> bool tryAssign(std::optional<type_t>& dest) const {
				if (isNull()) return false;
				dest = static_cast<type_t>(as<dest_t>());
				return true;
			}

			// ////////// //
			// // Info // //
			// ////////// //

			PType type() const { return m_type; }

			bool isNull() const { return type() == PType::Null; }
			bool isObj()  const { return type() == PType::Object; }
			bool isArr()  const { return type() == PType::Array; }
			bool isStr()  const { return type() == PType::String; }
			bool isSInt() const { return type() == PType::Signed; }
			bool isUInt() const { return type() == PType::Unsigned; }
			bool isDec()  const { return type() == PType::Decimal; }
			bool isBool() const { return type() == PType::Boolean; }
			bool isBin() const { return type() == PType::Binary; }

			bool isInteger() const { return isSInt() || isUInt(); }
			bool isNumber() const { return isSInt() || isUInt() || isDec(); }
			bool isPrimitive() const { return isSInt() || isUInt() || isDec() || isBool(); }

			akSize size() const { return getArr().size(); }

			// ///////// //
			// // Get // //
			// ///////// //
			template<typename type_t> typename std::enable_if<std::is_same<type_t, obj_t>::value, std::optional<type_t>>::type tryGet() const {
				return isObj() ? std::optional<type_t>(getObj()) : std::optional<type_t>();
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, arr_t>::value, std::optional<type_t>>::type tryGet() const {
				return isArr() ? std::optional<type_t>(getArr()) : std::optional<type_t>();
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, bin_t>::value, std::optional<type_t>>::type tryGet() const {
				return isBin() ? std::optional<type_t>(getBin()) : std::optional<type_t>();
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, str_t>::value, std::optional<type_t>>::type tryGet() const {
				return isStr() ? std::optional<type_t>(getStr()) : std::optional<type_t>();
			}

			template<typename type_t> typename std::enable_if<std::is_integral<type_t>::value && std::is_signed<type_t>::value, std::optional<type_t>>::type tryGet() const {
				return isSInt() ? std::optional<type_t>(getSInt()) : std::optional<type_t>();
			}

			template<typename type_t> typename std::enable_if<std::is_integral<type_t>::value && std::is_unsigned<type_t>::value && !std::is_same<type_t, std::optional<type_t>>::value, type_t>::type tryGet() const {
				return isUInt() ? std::optional<type_t>(getUInt()) : std::optional<type_t>();
			}

			template<typename type_t> typename std::enable_if<std::is_floating_point<type_t>::value, std::optional<type_t>>::type tryGet() const {
				return isDec() ? std::optional<type_t>(getDec()) : std::optional<type_t>();
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, bool>::value, std::optional<type_t>>::type tryGet() const {
				return isBool() ? std::optional<type_t>(getBool()) : std::optional<type_t>();
			}

			template<typename type_t> type_t get() const {
				auto result = tryGet<type_t>();
				throw std::logic_error(aku::buildString("Failed to get value from PValue containing ", toDebugString()));
				return *result;
			}

			template<typename type_t> type_t getOrDef(const type_t& val) const {
				auto result = tryGet<type_t>();
				if (!result) return val;
				return *result;
			}

			template<typename type_t> type_t getOrSet(const type_t& val) const {
				auto result = tryGet<type_t>();
				if (!result) return set<type_t>(val).template get<type_t>();
				return *result;
			}

			// //////////////// //
			// // Assignment // //
			// //////////////// //
			template<typename type_t> PValue& set(const type_t& val) {
				if constexpr(std::is_same<type_t, obj_t>::value) return setObj(val);
				if constexpr(std::is_same<type_t, arr_t>::value) return setArr(val);
				if constexpr(std::is_same<type_t, bin_t>::value) return setBin(val);
				if constexpr(std::is_same<type_t, str_t>::value) return setStr(val);
				if constexpr(std::is_same<typename std::decay<type_t>::type, char*>::value) return setStr(val); // required to handle char*/char[] constants, otherwise they get converted.
				if constexpr(std::is_same<type_t, bool_t>::value) return setBool(val); // Ensure ahead of implicit conversions (ie. ints)
				if constexpr(std::is_integral<type_t>::value && std::is_signed<type_t>::value) return setSInt(val);
				if constexpr(std::is_integral<type_t>::value && std::is_unsigned<type_t>::value) return setUInt(val);
				if constexpr(std::is_floating_point<type_t>::value) return setDec(val);
				throw std::logic_error("Invalid set type provided.");
			}

			template<typename type_t> PValue& trySet(const std::optional<type_t>& val) {
				if (val) return set<type_t>(*val);
				return setNull();
			}

			template<typename dest_t, typename type_t> PValue& trySet(const std::optional<type_t>& val) {
				if (val) return set<dest_t>(static_cast<dest_t>(*val));
				return setNull();
			}

			// //////////////// //
			// // Conversion // //
			// //////////////// //
			template<typename type_t> typename std::enable_if<std::is_same<type_t, obj_t>::value, std::optional<type_t>>::type tryAs()  const { // Object
				return isObj() ? getObj() : std::optional<type_t>();
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, arr_t>::value, std::optional<type_t>>::type tryAs()  const { // Array
				return isArr() ? getArr() : std::optional<type_t>();
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, bin_t>::value, std::optional<type_t>>::type tryAs()  const { // Binary
				return isBin() ? getBin() : std::optional<type_t>();
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, str_t>::value, std::optional<type_t>>::type tryAs()  const { // String
				return isStr() ? getStr() : std::optional<type_t>();
			}

			template<typename type_t> typename std::enable_if<std::is_arithmetic<type_t>::value && !std::is_same<type_t, bool>::value, std::optional<type_t>>::type tryAs() const { // Number
				if (isSInt()) return static_cast<type_t>(getSInt());
				if (isUInt()) return static_cast<type_t>(getUInt());
				if (isDec())  return static_cast<type_t>(getDec());
				return std::optional<type_t>();
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, bool_t>::value, std::optional<type_t>>::type tryAs() const { // Boolean
				if (isBool()) return static_cast<type_t>(getBool());
				return std::optional<type_t>();
			}

			template<typename type_t> type_t as() const {
				auto result = tryAs<type_t>();
				if (!result) throw std::logic_error(aku::buildString("Failed to convert value from PValue containing ", toDebugString()));
				return *result;
			}

			template<typename type_t> type_t asOrDef(const type_t& val) const {
				auto result = tryAs<type_t>();
				if (!result) return val;
				return *result;
			}

			template<typename type_t> type_t asOrSet(const type_t& val) {
				auto result = tryGet<type_t>();
				if (!result) return set<type_t>(val).template as<type_t>();
				return *result;
			}
	};

	class PVBuilder final {
		private:
			PValue m_result;
			std::deque<PValue*> m_stack;

		public:
			PVBuilder() : m_result(), m_stack({&m_result}) {}

			PVBuilder& pushKey(const std::string& name) {
				if (!m_stack.back()->isObj()) m_stack.back()->setObj();
				m_stack.push_back(&((*m_stack.back())[name]));
				return *this;
			}

			PVBuilder& pushIndex() {
				if (!m_stack.back()->isArr()) m_stack.back()->setArr();
				m_stack.back()->getArr().push_back(PValue());
				m_stack.push_back(&(m_stack.back()->getArr().back()));
				return *this;
			}

			PVBuilder& pop() {
				m_stack.pop_back();
				return *this;
			}

			template<typename type_t> PVBuilder& with(const std::string& keyName, const type_t& val) {
				if (!m_stack.back()->isObj()) m_stack.back()->setObj();
				(*m_stack.back())[keyName] = PValue::from<type_t>(val);
				return *this;
			}

			template<typename type_t> PVBuilder& insert(const type_t& val) {
				if (!m_stack.back()->isArr()) m_stack.back()->setArr();
				m_stack.back()->getArr().push_back(PValue::from<type_t>(val));
				return *this;
			}

			void mvBuild(PValue& dest) { dest = std::move(m_result); m_result = PValue(); m_stack.clear(); m_stack.push_back(&m_result); }
			const PValue& build() const { return m_result; }
	};
}



#endif
