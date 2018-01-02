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

#ifndef AK_DATA_PVALUE_HPP_
#define AK_DATA_PVALUE_HPP_

#include <ak/data/Path.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

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
		Integer,
		Unsigned,
		Decimal,
		Boolean,
		Binary,
	};

	void traversePValue(const PValue& cNode, const std::function<void(const Path& path, TraverseAction action, const PValue& value)>& callback);

	template<typename type_t> type_t deserialize(const PValue& root);

	class PValue final {
		public:
			using null_t = std::nullptr_t;
			using obj_t = std::map<std::string, PValue>;
			using arr_t = std::deque<PValue>;
			using str_t = std::string;
			using int_t = int64;
			using uint_t = uint64;
			using dec_t = fpDouble;
			using bool_t = bool;
			using bin_t = std::vector<uint8>;

			using traverse_f = std::function<bool(const Path&, const PValue&)>;

		private:
			union ValueContainer {
				obj_t oVal;
				arr_t aVal;
				str_t sVal;

				uint_t uVal;
				int_t iVal;
				dec_t dVal;
				bool_t bVal;

				bin_t binVal;

				ValueContainer() : uVal(0) {}
				~ValueContainer() {}
			};

			PType m_type;
			ValueContainer m_value;

			static PValue& navigate_internal(PValue& cNode, const Path& path);
			static const PValue* navigate_internal(const PValue* currentNode, const Path& path);

		public:
			PValue() : m_type(PType::Null) {}
			PValue(const PValue& val) : m_type(PType::Null) { setPValue(val); }
			PValue(const obj_t& val) : m_type(PType::Null)  { setObj(val); }
			PValue(const arr_t& val) : m_type(PType::Null)  { setArr(val); }
			PValue(const str_t& val) : m_type(PType::Null)  { setStr(val); }
			PValue(const int_t& val) : m_type(PType::Null)  { setInt(val); }
			PValue(const uint_t& val) : m_type(PType::Null) { setUInt(val); }
			PValue(const dec_t& val) : m_type(PType::Null)  { setDec(val); }
			PValue(const bool_t& val) : m_type(PType::Null) { setBool(val); }
			PValue(const bin_t& val) : m_type(PType::Null) { setBin(val); }

			template<typename type_t> static PValue from(const type_t& val) { PValue result; result.set<type_t>(val); return result; }

			~PValue() { setNull(); }

			// //////////////// //
			// // Navigation // //
			// //////////////// //

			PValue& at(const std::string& name) { return asObj().at(name); }
			PValue& at(akSize id) { return asArr().at(id); }

			PValue& atOrSet(const std::string& name, const PValue& val = PValue()) {
				return asObjOrSet().insert(std::make_pair(name, val)).first->second;
			}

			const PValue& atOrDef(const std::string& name, const PValue& val = PValue()) const {
				if (!isObj()) return val;
				auto iter = asObj().find(name);
				return (iter == asObj().end()) ? val : iter->second;
			}

			PValue& atOrSet(akSize id, const PValue& val = PValue()) {
				if (isArr() && id < asArr().size()) return asArr()[id];
				asArrOrSet().resize(id + 1);
				return asArr()[id] = val;
			}

			const PValue& atOrDef(akSize id, const PValue& val = PValue()) const {
				if (!isArr()) return val;
				return (id < asArr().size()) ? asArr()[id] : val;
			}

			PValue& operator[](const std::string& name) { return atOrSet(name); }
			PValue& operator[](const akSize& id) { return atOrSet(id); }

			const PValue& at(const std::string& name) const { return asObj().at(name); }
			const PValue& at(akSize id) const { return asArr().at(id); }

			bool exists(const std::string& name) const { return (isObj()) && (asObj().find(name) != asObj().end()); }
			bool exists(akSize id) const { return (isArr()) && (id < asArr().size()); }

			const PValue& operator[](const std::string& name) const { return atOrDef(name); }
			const PValue& operator[](const akSize& id) const { return atOrDef(id); }

			// ///////////// //
			// // Dirrect // //
			// ///////////// //

			PValue& setNull();
			PValue& setPValue(const PValue& val);

			PValue& setObj();
			PValue& setObj(const obj_t& val);

			PValue& setArr();
			PValue& setArr(const arr_t& val);

			PValue& setStr(const str_t& val);
			PValue& setInt(const int_t& val);
			PValue& setUInt(const uint_t& val);
			PValue& setDec(const dec_t& val);
			PValue& setBool(const bool_t& val);
			PValue& setBin(const bin_t& val);

			obj_t& asObj();
			arr_t& asArr();
			str_t& asStr();
			int_t& asInt();
			uint_t& asUInt();
			dec_t& asDec();
			bool_t& asBool();
			bin_t& asBin();

			obj_t& asObjOrSet(const obj_t& val = obj_t());
			arr_t& asArrOrSet(const arr_t& val = arr_t());
			str_t& asStrOrSet(const str_t& val = str_t());
			int_t& asIntOrSet(int_t val);
			uint_t& asUIntOrSet(uint_t val);
			dec_t& asDecOrSet(dec_t val);
			bool_t& asBoolOrSet(bool_t val);
			bin_t& asBinOrSet(const bin_t& val = bin_t());

			const obj_t& asObj() const;
			const arr_t& asArr() const;
			const str_t& asStr() const;
			int_t asInt() const;
			uint_t asUInt() const;
			dec_t asDec() const;
			bool_t asBool() const;
			const bin_t& asBin() const;

			const obj_t& asObjOrDef(const obj_t& val = obj_t()) const;
			const arr_t& asArrOrDef(const arr_t& val = arr_t()) const;
			const str_t& asStrOrDef(const str_t& val = str_t()) const;
			int_t asIntOrDef(int_t val) const;
			uint_t asUIntOrDef(uint_t val) const;
			dec_t asDecOrDef(dec_t val) const;
			bool_t asBoolOrDef(bool_t val) const;
			const bin_t& asBinOrDef(const bin_t& val) const;


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
			bool isSInt() const { return type() == PType::Integer; }
			bool isUInt() const { return type() == PType::Unsigned; }
			bool isDec()  const { return type() == PType::Decimal; }
			bool isBool() const { return type() == PType::Boolean; }
			bool isBin() const { return type() == PType::Binary; }

			bool isInteger() const { return isSInt() || isUInt(); }
			bool isNumber() const { return isSInt() || isUInt() || isDec(); }
			bool isPrimitive() const { return isSInt() || isUInt() || isDec() || isBool(); }

			akSize size() const { return static_cast<akSize>(asArr().size()); }

			// //////////////// //
			// // Assignment // //
			// //////////////// //


			template<typename type_t> PValue& set(const typename std::enable_if<std::is_same<type_t, PValue>::value, type_t>::type& val) { return setPValue(val); }

			template<typename type_t> PValue& set(const typename std::enable_if<std::is_same<type_t, obj_t>::value, type_t>::type& val) { return setObj(val); }
			template<typename type_t> PValue& set(const typename std::enable_if<std::is_same<type_t, arr_t>::value, type_t>::type& val) { return setArr(val); }
			template<typename type_t> PValue& set(const typename std::enable_if<std::is_same<type_t, str_t>::value || std::is_same<typename std::decay<type_t>::type, char*>::value, type_t>::type& val) { return setStr(val); }

			template<typename type_t> PValue& set(const typename std::enable_if<std::is_integral<type_t>::value && std::is_signed<type_t>::value, type_t>::type& val) { return setInt(static_cast<int_t>(val)); }
			template<typename type_t> PValue& set(const typename std::enable_if<std::is_integral<type_t>::value && std::is_unsigned<type_t>::value && !std::is_same<type_t, bool>::value, type_t>::type& val) { return setUInt(static_cast<uint_t>(val)); }
			template<typename type_t> PValue& set(const typename std::enable_if<std::is_floating_point<type_t>::value, type_t>::type& val) { return setDec(static_cast<dec_t>(val)); }
			template<typename type_t> PValue& set(const typename std::enable_if<std::is_same<type_t, bool>::value, type_t>::type& val) { return setBool(static_cast<bool_t>(val)); }

			template<typename type_t> PValue& set(const typename std::enable_if<std::is_same<type_t, bin_t>::value, type_t>::type& val) { return setBin(static_cast<bin_t>(val)); }

			template<typename type_t> PValue& trySet(const std::optional<type_t>& val) {
				if (val) return set<type_t>(*val);
				return setNull();
			}

			template<typename dest_t, typename type_t> PValue& trySet(const std::optional<type_t>& val) {
				if (val) return set<dest_t>(static_cast<dest_t>(*val));
				return setNull();
			}

			PValue& operator=(const PValue& val) { return setPValue(val); }
			PValue& operator=(const obj_t& val) { return setObj(val); }
			PValue& operator=(const arr_t& val) { return setArr(val); }
			PValue& operator=(const str_t& val) { return setStr(val); }
			template<typename type_t> PValue& operator=(const type_t& val) { return set<type_t>(val); }

			// //////////////// //
			// // Conversion // //
			// //////////////// //
			template<typename type_t> typename std::enable_if<std::is_same<type_t, obj_t>::value, type_t>::type as() const { return asObj(); }
			template<typename type_t> typename std::enable_if<std::is_same<type_t, arr_t>::value, type_t>::type as() const { return asArr(); }
			template<typename type_t> typename std::enable_if<std::is_same<type_t, str_t>::value || std::is_same<typename std::decay<type_t>::type, char*>::value, std::string>::type as() const { return asStr(); }
			template<typename type_t> typename std::enable_if<std::is_same<type_t, bin_t>::value, type_t>::type as() const { return asBin(); }

			template<typename type_t> typename std::enable_if<std::is_arithmetic<type_t>::value && !std::is_same<type_t, bool>::value, type_t>::type as() const {
				if (isSInt()) return static_cast<type_t>(m_value.iVal);
				if (isUInt()) return static_cast<type_t>(m_value.uVal);
				if (isDec())  return static_cast<type_t>(m_value.dVal);
				if (isBool()) return static_cast<type_t>(m_value.bVal);
				throw std::logic_error("PValue does not contain a primitive value.");
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, bool>::value, type_t>::type as() const {
				if (isBool()) return static_cast<type_t>(m_value.bVal);
				if (isSInt()) return static_cast<type_t>(m_value.iVal);
				if (isUInt()) return static_cast<type_t>(m_value.uVal);
				if (isDec())  return static_cast<type_t>(m_value.dVal);
				throw std::logic_error("PValue does not contain a primitive value.");
			}



			template<typename type_t> typename std::enable_if<std::is_same<type_t, obj_t>::value, type_t>::type asOrSet(const type_t& val)  {
				if (!isObj()) setObj(val);
				return m_value.oVal;
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, arr_t>::value, type_t>::type asOrSet(const type_t& val)  {
				if (!isArr()) setArr(val);
				return m_value.aVal;
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, bin_t>::value, type_t>::type asOrSet(const type_t& val)  {
				if (!isBin()) setBin(val);
				return m_value.binVal;
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, str_t>::value || std::is_same<typename std::decay<type_t>::type, char*>::value, std::string>::type asOrSet(const type_t& val)  {
				if (!isStr()) asStr(val);
				return m_value.sVal;
			}

			template<typename type_t> typename std::enable_if<std::is_integral<type_t>::value && std::is_signed<type_t>::value, type_t>::type asOrSet(const type_t& val) {
				if (isSInt()) return static_cast<type_t>(m_value.iVal);
				if (isUInt()) return static_cast<type_t>(m_value.uVal);
				if (isDec())  return static_cast<type_t>(m_value.dVal);
				if (isBool()) return static_cast<type_t>(m_value.bVal);
				setInt(static_cast<int_t>(val));
				return static_cast<type_t>(m_value.iVal);
			}

			template<typename type_t> typename std::enable_if<std::is_integral<type_t>::value && std::is_unsigned<type_t>::value, type_t>::type asOrSet(const type_t& val) {
				if (isSInt()) return static_cast<type_t>(m_value.iVal);
				if (isUInt()) return static_cast<type_t>(m_value.uVal);
				if (isDec())  return static_cast<type_t>(m_value.dVal);
				if (isBool()) return static_cast<type_t>(m_value.bVal);
				setUInt(static_cast<uint_t>(val));
				return static_cast<type_t>(m_value.uVal);
			}

			template<typename type_t> typename std::enable_if<std::is_floating_point<type_t>::value, type_t>::type asOrSet(const type_t& val) {
				if (isSInt()) return static_cast<type_t>(m_value.iVal);
				if (isUInt()) return static_cast<type_t>(m_value.uVal);
				if (isDec())  return static_cast<type_t>(m_value.dVal);
				if (isBool()) return static_cast<type_t>(m_value.bVal);
				setDec(static_cast<dec_t>(val));
				return static_cast<type_t>(m_value.dVal);
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, bool>::value, type_t>::type asOrSet(const type_t& val) {
				if (isSInt()) return static_cast<type_t>(m_value.iVal);
				if (isUInt()) return static_cast<type_t>(m_value.uVal);
				if (isDec())  return static_cast<type_t>(m_value.dVal);
				if (isBool()) return static_cast<type_t>(m_value.bVal);
				setBool(static_cast<bool_t>(val));
				return static_cast<type_t>(m_value.bVal);
			}



			template<typename type_t> typename std::enable_if<std::is_same<type_t, obj_t>::value, type_t>::type asOrDef(const type_t& val)  const {
				if (!isObj()) return val;
				return m_value.oVal;
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, arr_t>::value, type_t>::type asOrDef(const type_t& val)  const {
				if (!isArr()) return val;
				return m_value.aVal;
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, bin_t>::value, type_t>::type asOrDef(const type_t& val)  const {
				if (!isBin()) return val;
				return m_value.binVal;
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, str_t>::value || std::is_same<typename std::decay<type_t>::type, char*>::value, std::string>::type asOrDef(const type_t& val)  const {
				if (!isStr()) return std::string(val);
				return m_value.sVal;
			}

			template<typename type_t> typename std::enable_if<std::is_integral<type_t>::value && std::is_signed<type_t>::value && !std::is_same<type_t, bool>::value, type_t>::type asOrDef(const type_t& val) const {
				if (isSInt()) return static_cast<type_t>(m_value.iVal);
				if (isUInt()) return static_cast<type_t>(m_value.uVal);
				if (isDec())  return static_cast<type_t>(m_value.dVal);
				if (isBool()) return static_cast<type_t>(m_value.bVal);
				return val;
			}

			template<typename type_t> typename std::enable_if<std::is_integral<type_t>::value && std::is_unsigned<type_t>::value && !std::is_same<type_t, bool>::value, type_t>::type asOrDef(const type_t& val) const {
				if (isSInt()) return static_cast<type_t>(m_value.iVal);
				if (isUInt()) return static_cast<type_t>(m_value.uVal);
				if (isDec())  return static_cast<type_t>(m_value.dVal);
				if (isBool()) return static_cast<type_t>(m_value.bVal);
				return val;
			}

			template<typename type_t> typename std::enable_if<std::is_floating_point<type_t>::value && !std::is_same<type_t, bool>::value, type_t>::type asOrDef(const type_t& val) const {
				if (isSInt()) return static_cast<type_t>(m_value.iVal);
				if (isUInt()) return static_cast<type_t>(m_value.uVal);
				if (isDec())  return static_cast<type_t>(m_value.dVal);
				if (isBool()) return static_cast<type_t>(m_value.bVal);
				return val;
			}

			template<typename type_t> typename std::enable_if<std::is_same<type_t, bool>::value, type_t>::type asOrDef(const type_t& val) const {
				if (isSInt()) return static_cast<type_t>(m_value.iVal);
				if (isUInt()) return static_cast<type_t>(m_value.uVal);
				if (isDec())  return static_cast<type_t>(m_value.dVal);
				if (isBool()) return static_cast<type_t>(m_value.bVal);
				return val;
			}

			template<typename type_t> std::optional<type_t> tryAs() const {
				if (isNull()) return std::optional<type_t>();
				return std::optional<type_t>(as<type_t>());
			}
	};
}

#endif
