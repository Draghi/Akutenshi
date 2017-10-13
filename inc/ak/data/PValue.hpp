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
#include <string>

namespace ak {
	namespace data {

		enum class PType : uint8 {
			Null,
			Object,
			Array,
			String,
			Integer,
			Decimal,
			Boolean,
		};

		class PValue final {
			public:
				using null_t = std::nullptr_t;
				using obj_t = std::map<std::string, PValue>;
				using arr_t = std::deque<PValue>;
				using str_t = std::string;
				using int_t = int64;
				using dec_t = fpDouble;
				using bool_t = bool;

				using traverse_f = std::function<bool(const Path&, const PValue&)>;

			private:
				union ValueContainer {
					obj_t oVal;
					arr_t aVal;
					str_t sVal;

					int_t iVal;
					dec_t dVal;
					bool_t bVal;

					ValueContainer() : iVal(0) {}
					~ValueContainer() {}
				};

				PType m_type;
				ValueContainer m_value;

				static PValue& navigate_internal(PValue& cNode, const Path& path);
				static const PValue* navigate_internal(const PValue* currentNode, const Path& path);

				void setPValue(const PValue& val);
				void setPValue(PValue&& val);

			public:
				PValue();

				PValue(const PValue& val);
				PValue(PValue&& val);

				PValue(const null_t&);
				PValue(const obj_t& val);
				PValue(const arr_t& val);
				PValue(const str_t& val);
				PValue(const int_t& val);
				PValue(const dec_t& val);
				PValue(const bool_t& val);

				PValue(null_t&&);
				PValue(obj_t&& val);
				PValue(arr_t&& val);
				PValue(str_t&& val);
				PValue(int_t&& val);
				PValue(dec_t&& val);
				PValue(bool_t&& val);

				~PValue();

				PValue& get(const Path& path);

				PValue* tryGet(const Path& path);
				const PValue* tryGet(const Path& path) const;

				const PValue& getOrNull(const Path& path) const;

				bool exists(const Path& path) const;

				void merge(const PValue& other, bool override = true);

				void setNull();
				void setObj();
				void setArr();
				void setObj(const obj_t& val);
				void setArr(const arr_t& val);
				void setStr(const str_t& val);
				void setInt(const int_t& val);
				void setDec(const dec_t& val);
				void setBool(const bool_t& val);

				obj_t& asObj();
				arr_t& asArr();
				str_t& asStr();
				int_t& asInt();
				dec_t& asDec();
				bool_t& asBool();

				obj_t& asObj(const obj_t& def);
				arr_t& asArr(const arr_t& def);
				str_t& asStr(const str_t& def);
				int_t& asInt(int_t def);
				dec_t& asDec(dec_t def);
				bool_t& asBool(bool_t def);

				const obj_t& asObj() const;
				const arr_t& asArr() const;
				const str_t& asStr() const;
				int_t asInt() const;
				dec_t asDec() const;
				bool_t asBool() const;

				const obj_t& asObjOr(const obj_t& def) const;
				const arr_t& asArrOr(const arr_t& def) const;
				const str_t& asStrOr(const str_t& def) const;
				int_t asIntOr(int_t def) const;
				dec_t asDecOr(dec_t def) const;
				bool_t asBoolOr(bool_t def) const;

				obj_t*  asObjPtr();
				arr_t*  asArrPtr();
				str_t*  asStrPtr();
				int_t*  asIntPtr();
				dec_t*  asDecPtr();
				bool_t* asBoolPtr();

				const obj_t*  asObjPtr() const;
				const arr_t*  asArrPtr() const;
				const str_t*  asStrPtr() const;
				const int_t*  asIntPtr() const;
				const dec_t*  asDecPtr() const;
				const bool_t* asBoolPtr() const;

				PType type() const;

				bool isNull() const;
				bool isObj() const;
				bool isArr() const;
				bool isStr() const;
				bool isInt() const;
				bool isDec() const;
				bool isBool() const;

				PValue& operator[](const Path& path);
				const PValue& operator[](const Path& path) const;

				PValue& operator=(const PValue& val);
				PValue& operator=(PValue&& val);

				// ////////////// //
				// // Shortcut // //
				// ////////////// //

				template<typename type_t> void setObj( const type_t& val) { setObj( static_cast<obj_t> (val)); }
				template<typename type_t> void setArr( const type_t& val) { setArr( static_cast<arr_t> (val)); }
				template<typename type_t> void setStr( const type_t& val) { setStr( static_cast<str_t> (val)); }
				template<typename type_t> void setInt( const type_t& val) { setInt( static_cast<int_t> (val)); }
				template<typename type_t> void setDec( const type_t& val) { setDec( static_cast<dec_t> (val)); }
				template<typename type_t> void setBool(const type_t& val) { setBool(static_cast<bool_t>(val)); }

				template<typename type_t> bool assignAsObj(type_t& other)  const { if (!isNull()) { other = static_cast<type_t>(asObj() ); return true; } return false; }
				template<typename type_t> bool assignAsArr(type_t& other)  const { if (!isNull()) { other = static_cast<type_t>(asArr() ); return true; } return false; }
				template<typename type_t> bool assignAsStr(type_t& other)  const { if (!isNull()) { other = static_cast<type_t>(asStr() ); return true; } return false; }
				template<typename type_t> bool assignAsInt(type_t& other)  const { if (!isNull()) { other = static_cast<type_t>(asInt() ); return true; } return false; }
				template<typename type_t> bool assignAsDec(type_t& other)  const { if (!isNull()) { other = static_cast<type_t>(asDec() ); return true; } return false; }
				template<typename type_t> bool assignAsBool(type_t& other) const { if (!isNull()) { other = static_cast<type_t>(asBool()); return true; } return false; }

				template<typename type_t> void setOptObj( const std::optional<type_t>& val) { if (val) setObj( static_cast<obj_t> (*val)); else setNull(); }
				template<typename type_t> void setOptArr( const std::optional<type_t>& val) { if (val) setArr( static_cast<arr_t> (*val)); else setNull(); }
				template<typename type_t> void setOptStr( const std::optional<type_t>& val) { if (val) setStr( static_cast<str_t> (*val)); else setNull(); }
				template<typename type_t> void setOptInt( const std::optional<type_t>& val) { if (val) setInt( static_cast<int_t> (*val)); else setNull(); }
				template<typename type_t> void setOptDec( const std::optional<type_t>& val) { if (val) setDec( static_cast<dec_t> (*val)); else setNull(); }
				template<typename type_t> void setOptBool(const std::optional<type_t>& val) { if (val) setBool(static_cast<bool_t>(*val)); else setNull(); }

				template<typename type_t> void trySetObj( const std::optional<type_t>& val) { if (val) setObj( static_cast<obj_t> (*val)); }
				template<typename type_t> void trySetArr( const std::optional<type_t>& val) { if (val) setArr( static_cast<arr_t> (*val)); }
				template<typename type_t> void trySetStr( const std::optional<type_t>& val) { if (val) setStr( static_cast<str_t> (*val)); }
				template<typename type_t> void trySetInt( const std::optional<type_t>& val) { if (val) setInt( static_cast<int_t> (*val)); }
				template<typename type_t> void trySetDec( const std::optional<type_t>& val) { if (val) setDec( static_cast<dec_t> (*val)); }
				template<typename type_t> void trySetBool(const std::optional<type_t>& val) { if (val) setBool(static_cast<bool_t>(*val)); }

				PValue& get(const std::string& path) { return get(Path() << path); }
				PValue& get(size_t path) { return get(Path() << path); }

				PValue* tryGet(const std::string& path) { return tryGet(Path() << path); }
				PValue* tryGet(size_t path) { return tryGet(Path() << path); }

				const PValue* tryGet(const std::string& path) const { return tryGet(Path() << path); }
				const PValue* tryGet(size_t path) const { return tryGet(Path() << path); }

				const PValue& getOrNull(const std::string& path) const { return getOrNull(Path() << path); }
				const PValue& getOrNull(size_t path) const { return getOrNull(Path() << path); }

				bool exists(const std::string& path) const { return exists(Path() << path); }
				bool exists(size_t path) const { return exists(Path() << path); }

				PValue& operator[](const std::string& path) { return operator[](Path() << path); }
				PValue& operator[](size_t path) { return operator[](Path() << path); }

				const PValue& operator[](const std::string& path) const { return operator[](Path() << path); }
				const PValue& operator[](size_t path) const { return operator[](Path() << path); }
		};

		enum class TraverseAction {
			ObjectStart,
			ObjectEnd,
			ArrayStart,
			ArrayEnd,
			Value,
		};

		void traversePValue(const PValue& cNode, const std::function<void(const Path& path, TraverseAction action, const PValue& value)>& callback);
	}
}

#endif
