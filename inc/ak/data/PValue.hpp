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
			Unsigned,
			Integer,
			Float,
			Bool,
		};

		class PValue final {
			public:
				using null_t = std::nullptr_t;
				using obj_t = std::map<std::string, PValue>;
				using arr_t = std::deque<PValue>;
				using str_t = std::string;
				using uint_t = uint64;
				using int_t = int64;
				using float_t = fpDouble;
				using bool_t = bool;

				using traverse_f = std::function<bool(const Path&, const PValue&)>;

			private:
				union ValueContainer {
					obj_t oVal;
					arr_t aVal;
					str_t sVal;

					uint_t uVal;
					int_t iVal;
					float_t fVal;
					bool_t bVal;

					ValueContainer() : uVal(0) {}
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
				PValue(const uint_t& val);
				PValue(const int_t& val);
				PValue(const float_t& val);
				PValue(const bool_t& val);

				PValue(null_t&&);
				PValue(obj_t&& val);
				PValue(arr_t&& val);
				PValue(str_t&& val);
				PValue(uint_t&& val);
				PValue(int_t&& val);
				PValue(float_t&& val);
				PValue(bool_t&& val);

				~PValue();

				PValue& navigate(const Path& path);
				PValue& navigate(const std::string& path) { return navigate(Path() << path); }
				PValue& navigate(size_t path) { return navigate(Path() << path); }

				const PValue* tryNavigate(const Path& path) const;
				const PValue* tryNavigate(const std::string& path) const { return tryNavigate(Path() << path); }
				const PValue* tryNavigate(size_t path) const { return tryNavigate(Path() << path); }

				bool exists(const Path& path) const;
				bool exists(const std::string& path) const { return exists(Path() << path); }
				bool exists(size_t path) const { return exists(Path() << path); }

				void setNull();
				void setObject();
				void setArray();

				void setNull(const null_t&);
				void setObject(const obj_t& val);
				void setArray(const arr_t& val);
				void setString(const str_t& val);
				void setUnsigned(const uint_t& val);
				void setInteger(const int_t& val);
				void setFloat(const float_t& val);
				void setBool(const bool_t& val);

				void setOptionalNull(const std::optional<null_t>&);
				void setOptionalObject(const std::optional<obj_t>& val);
				void setOptionalArray(const std::optional<arr_t>& val);
				void setOptionalString(const std::optional<str_t>& val);
				void setOptionalUnsigned(const std::optional<uint_t>& val);
				void setOptionalInteger(const std::optional<int_t>& val);
				void setOptionalFloat(const std::optional<float_t>& val);
				void setOptionalBool(const std::optional<bool_t>& val);

				void setNull(null_t&&);
				void setObject(obj_t&& val);
				void setArray(arr_t&& val);
				void setString(str_t&& val);
				void setUnsigned(uint_t&& val);
				void setInteger(int_t&& val);
				void setFloat(float_t&& val);
				void setBool(bool_t&& val);

				obj_t* objectPtr();
				arr_t* arrayPtr();
				str_t* stringPtr();
				uint_t* unsignedPtr();
				int_t* integerPtr();
				float_t* floatPtr();
				bool_t* boolPtr();

				const obj_t* objectPtr() const;
				const arr_t* arrayPtr() const;
				const str_t* stringPtr() const;
				const uint_t* unsignedPtr() const;
				const int_t* integerPtr() const;
				const float_t* floatPtr() const;
				const bool_t* boolPtr() const;

				obj_t& objectValue();
				arr_t& arrayValue();
				str_t& stringValue();
				uint_t& unsignedValue();
				int_t& integerValue();
				float_t& floatValue();
				bool_t& boolValue();

				const obj_t& objectValue() const;
				const arr_t& arrayValue() const;
				const str_t& stringValue() const;
				const uint_t& unsignedValue() const;
				const int_t& integerValue() const;
				const float_t& floatValue() const;
				const bool_t& boolValue() const;

				std::optional<uint_t> tryAsUnsigned() const;
				std::optional<int_t> tryAsInteger() const;
				std::optional<float_t> tryAsFloat() const;
				std::optional<str_t> tryAsString() const;
				std::optional<bool_t> tryAsBool() const;

				uint_t asUnsigned() const;
				int_t asInteger() const;
				float_t asFloat() const;
				str_t asString() const;
				bool_t asBool() const;

				PType type() const;

				bool isObject() const;
				bool isArray() const;
				bool isUnsigned() const;
				bool isInteger() const;
				bool isString() const;
				bool isBool() const;

				PValue& operator[](const Path& path);
				PValue& operator[](size_t id);
				PValue& operator[](const std::string& name);

				const PValue& operator[](const Path& path) const;
				const PValue& operator[](size_t id) const { return (*this)[Path() << id]; }
				const PValue& operator[](const std::string& name) const { return (*this)[Path() << name]; }

				PValue& operator=(const PValue& val);
				PValue& operator=(PValue&& val);
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
