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

#include <ak/data/PValue.hpp>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <sstream>

using namespace ak::data;

PValue& PValue::navigate_internal(PValue& cNode, const Path& path) {

	PValue* currentNode = &cNode;
	for(size_t i = 0; i < path.size(); i++) {
		auto entry = path.entry(i);
		if (entry.isIndex) {
			if (currentNode->m_type != PType::Array) currentNode->setArray(arr_t());
			if (entry.index >= currentNode->m_value.aVal.size()) currentNode->m_value.aVal.resize(entry.index + 1);

			currentNode = &currentNode->m_value.aVal[entry.index];
			continue;

		} else {
			if (currentNode->m_type != PType::Object) currentNode->setObject(obj_t());

			auto iter = currentNode->m_value.oVal.find(entry.path);
			if (iter == currentNode->m_value.oVal.end()) iter = currentNode->m_value.oVal.insert(std::make_pair(entry.path, PValue())).first;

			currentNode = &iter->second;
			continue;

		}
	}

	return *currentNode;
}

const PValue* PValue::navigate_internal(const PValue* currentNode, const Path& path) {
	if (currentNode == nullptr) return nullptr;

	for(size_t i = 0; i < path.size(); i++) {

		auto entry = path.entry(i);
		if (entry.isIndex) {

			if (currentNode->m_type != PType::Array) return nullptr;
			if (entry.index >= currentNode->m_value.aVal.size()) return nullptr;

			currentNode = &currentNode->m_value.aVal[entry.index];
			continue;

		} else {

			if (currentNode->m_type != PType::Object) return nullptr;

			auto iter = currentNode->m_value.oVal.find(entry.path);
			if (iter == currentNode->m_value.oVal.end()) return nullptr;

			currentNode = &iter->second;
			continue;

		}
	}

	return currentNode;
}


void PValue::setPValue(const PValue& val) {
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch"
	switch(val.m_type) {
		case PType::Object: setObject(val.m_value.oVal); break;
		case PType::Array: setArray(val.m_value.aVal); break;
		case PType::String: setString(val.m_value.sVal); break;

		case PType::Unsigned: setUnsigned(val.m_value.uVal); break;
		case PType::Integer: setInteger(val.m_value.iVal); break;
		case PType::Float: setFloat(val.m_value.fVal); break;
		case PType::Bool: setBool(val.m_value.bVal); break;
	}
	#pragma clang diagnostic pop
}

void PValue::setPValue(PValue&& val) {
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch"
	switch(val.m_type) {
		case PType::Object: setObject(std::move(val.m_value.oVal)); break;
		case PType::Array: setArray(std::move(val.m_value.aVal)); break;
		case PType::String: setString(std::move(val.m_value.sVal)); break;

		case PType::Unsigned: setUnsigned(std::move(val.m_value.uVal)); break;
		case PType::Integer: setInteger(std::move(val.m_value.iVal)); break;
		case PType::Float: setFloat(std::move(val.m_value.fVal)); break;
		case PType::Bool: setBool(std::move(val.m_value.bVal)); break;
	}
	val.setNull();
	#pragma clang diagnostic pop
}

PValue::PValue() : m_type(PType::Null) {

}

PValue::PValue(const PValue& val) : m_type(PType::Null) {
	setPValue(val);
}

PValue::PValue(PValue&& val) : m_type(PType::Null) {
	setPValue(std::move(val));
}

PValue::PValue(const null_t&) : m_type(PType::Null) {

}

PValue::PValue(const obj_t& val) : m_type(PType::Null) {
	setObject(val);
}

PValue::PValue(const arr_t& val) : m_type(PType::Null) {
	setArray(val);
}

PValue::PValue(const str_t& val) : m_type(PType::Null) {
	setString(val);
}

PValue::PValue(const uint_t& val) : m_type(PType::Null) {
	setUnsigned(val);
}

PValue::PValue(const int_t& val) : m_type(PType::Null) {
	setInteger(val);
}

PValue::PValue(const float_t& val) : m_type(PType::Null) {
	setFloat(val);
}

PValue::PValue(const bool_t& val) : m_type(PType::Null) {
	setBool(val);
}

PValue::PValue(null_t&&) : m_type(PType::Null) {

}

PValue::PValue(obj_t&& val) : m_type(PType::Null) {
	setObject(std::move(val));
}

PValue::PValue(arr_t&& val) : m_type(PType::Null) {
	setArray(std::move(val));
}

PValue::PValue(str_t&& val) : m_type(PType::Null) {
	setString(std::move(val));
}

PValue::PValue(uint_t&& val) : m_type(PType::Null) {
	setUnsigned(std::move(val));
}

PValue::PValue(int_t&& val) : m_type(PType::Null) {
	setInteger(std::move(val));
}

PValue::PValue(float_t&& val) : m_type(PType::Null) {
	setFloat(std::move(val));
}

PValue::PValue(bool_t&& val) : m_type(PType::Null) {
	setBool(std::move(val));
}

PValue::~PValue() {
	setNull();
}

PValue& PValue::navigate(const Path& path){
	return navigate_internal(*this, path);
}

const PValue* PValue::tryNavigate(const Path& path) const {
	return navigate_internal(this, path);
}

bool PValue::exists(const Path& path) const {
	return tryNavigate(path) != nullptr;
}

void PValue::setNull() {
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch"
	switch(m_type) {
		case PType::Object: m_value.oVal.~map(); break;
		case PType::Array: m_value.aVal.~deque(); break;
		case PType::String: m_value.sVal.~basic_string(); break;
	}
	m_type = PType::Null;
	#pragma clang diagnostic pop
}

void PValue::setObject() {
	setObject(obj_t());
}

void PValue::setArray() {
	setArray(arr_t());
}

void PValue::setNull(const null_t&) {
	setNull();
}

void PValue::setObject(const obj_t& val) {
	setNull();
	new(&m_value.oVal) obj_t(val);
	m_type = PType::Object;
}

void PValue::setArray(const arr_t& val) {
	setNull();
	new(&m_value.aVal) arr_t(val);
	m_type = PType::Array;
}

void PValue::setString(const str_t& val) {
	setNull();
	new(&m_value.sVal) std::string(val);
	m_type = PType::String;
}

void PValue::setUnsigned(const uint_t& val) {
	setNull();
	m_value.uVal = val;
	m_type = PType::Unsigned;
}

void PValue::setInteger(const int_t& val) {
	setNull();
	m_value.iVal = val;
	m_type = PType::Integer;
}

void PValue::setFloat(const float_t& val) {
	setNull();
	m_value.fVal = val;
	m_type = PType::Float;
}

void PValue::setBool(const bool_t& val) {
	setNull();
	m_value.bVal = val;
	m_type = PType::Bool;
}

void PValue::setOptionalNull(const std::optional<null_t>&) {
	setNull();
}

void PValue::setOptionalObject(const std::optional<obj_t>& val) {
	if (val) setObject(*val);
	else setNull();
}

void PValue::setOptionalArray(const std::optional<arr_t>& val) {
	if (val) setArray(*val);
	else setNull();
}

void PValue::setOptionalString(const std::optional<str_t>& val) {
	if (val) setString(*val);
	else setNull();
}

void PValue::setOptionalUnsigned(const std::optional<uint_t>& val) {
	if (val) setUnsigned(*val);
	else setNull();
}

void PValue::setOptionalInteger(const std::optional<int_t>& val) {
	if (val) setInteger(*val);
	else setNull();
}

void PValue::setOptionalFloat(const std::optional<float_t>& val) {
	if (val) setFloat(*val);
	else setNull();
}

void PValue::setOptionalBool(const std::optional<bool_t>& val) {
	if (val) setBool(*val);
	else setNull();
}

void PValue::setNull(null_t&&) {
	setNull();
}

void traversePValue(const PValue& rootNode, const std::function<void(const Path& path, TraverseAction action, const PValue& value)>& callback);
void PValue::setObject(obj_t&& val) {
	setNull();
	new(&m_value.oVal) obj_t(std::move(val));
	m_type = PType::Object;
}

void PValue::setArray(arr_t&& val) {
	setNull();
	new(&m_value.aVal) arr_t(std::move(val));
	m_type = PType::Array;
}

void PValue::setString(str_t&& val) {
	setNull();
	new(&m_value.sVal)
	std::string(std::move(val));
	m_type = PType::String;
}

void PValue::setUnsigned(uint_t&& val) {
	setNull();
	m_value.uVal = std::move(val);
	m_type = PType::Unsigned;
}

void PValue::setInteger(int_t&& val) {
	setNull();
	m_value.iVal = std::move(val);
	m_type = PType::Integer;
}

void PValue::setFloat(float_t&& val) {
	setNull();
	m_value.fVal = std::move(val);
	m_type = PType::Float;
}

void PValue::setBool(bool_t&& val) {
	setNull();
	m_value.bVal = std::move(val);
	m_type = PType::Bool;
}

PValue::obj_t* PValue::objectPtr() {
	return (m_type == PType::Object) ? &m_value.oVal : nullptr;
}

PValue::arr_t* PValue::arrayPtr() {
	return (m_type == PType::Array) ? &m_value.aVal : nullptr;
}

PValue::str_t* PValue::stringPtr() {
	return (m_type == PType::String) ? &m_value.sVal : nullptr;
}

PValue::uint_t* PValue::unsignedPtr() {
	return (m_type == PType::Unsigned) ? &m_value.uVal : nullptr;
}

PValue::int_t* PValue::integerPtr() {
	return (m_type == PType::Integer) ? &m_value.iVal : nullptr;
}

PValue::float_t* PValue::floatPtr() {
	return (m_type == PType::Float) ? &m_value.fVal : nullptr;
}

PValue::bool_t* PValue::boolPtr() {
	return (m_type == PType::Bool) ? &m_value.bVal : nullptr;
}

const PValue::obj_t* PValue::objectPtr() const {
	return (m_type == PType::Object) ? &m_value.oVal : nullptr;
}

const PValue::arr_t* PValue::arrayPtr() const {
	return (m_type == PType::Array) ? &m_value.aVal : nullptr;
}

const PValue::str_t* PValue::stringPtr() const {
	return (m_type == PType::String) ? &m_value.sVal : nullptr;
}

const PValue::uint_t* PValue::unsignedPtr() const {
	return (m_type == PType::Unsigned) ? &m_value.uVal : nullptr;
}

const PValue::int_t* PValue::integerPtr() const {
	return (m_type == PType::Integer) ? &m_value.iVal : nullptr;
}

const PValue::float_t* PValue::floatPtr() const {
	return (m_type == PType::Float) ? &m_value.fVal : nullptr;
}

const PValue::bool_t* PValue::boolPtr() const {
	return (m_type == PType::Bool) ? &m_value.bVal : nullptr;
}

PValue::obj_t& PValue::objectValue() {
	auto* ptr = objectPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain object value");
}

PValue::arr_t& PValue::arrayValue() {
	auto* ptr = arrayPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain array value");
}

PValue::str_t& PValue::stringValue() {
	auto* ptr = stringPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain string value");
}

PValue::uint_t& PValue::unsignedValue() {
	auto* ptr = unsignedPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain unsigned value");
}

PValue::int_t& PValue::integerValue() {
	auto* ptr = integerPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain integer value");
}

PValue::float_t& PValue::floatValue() {
	auto* ptr = floatPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain float value");
}

PValue::bool_t& PValue::boolValue() {
	auto* ptr = boolPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain float value");
}

const PValue::obj_t& PValue::objectValue() const {
	auto* ptr = objectPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain object value");
}

const PValue::arr_t& PValue::arrayValue() const {
	auto* ptr = arrayPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain array value");
}

const PValue::str_t& PValue::stringValue() const {
	auto* ptr = stringPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain string value");
}

const PValue::uint_t& PValue::unsignedValue() const {
	auto* ptr = unsignedPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain unsigned value");
}

const PValue::int_t& PValue::integerValue() const {
	auto* ptr = integerPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain integer value");
}

const PValue::float_t& PValue::floatValue() const {
	auto* ptr = floatPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain float value");
}

const PValue::bool_t& PValue::boolValue() const {
	auto* ptr = boolPtr();
	if (ptr) return *ptr;
	throw std::logic_error("PValue::value: Doesn't contain bool value");
}

std::optional<PValue::uint_t> PValue::tryAsUnsigned() const {
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch"
	switch(m_type) {
		case PType::Null: return {0};
		case PType::Unsigned: return {m_value.uVal};
		case PType::Integer: return {static_cast<uint_t>(m_value.iVal)};
		case PType::Float: return {static_cast<uint_t>(m_value.fVal)};
		case PType::Bool: return {static_cast<uint_t>(m_value.bVal)};

		case PType::String: {
			try {
				return {std::stoull(m_value.sVal)};
			} catch(...) {
				return {};
			}
		}
	}
	#pragma clang diagnostic pop
	return {};
}

std::optional<PValue::int_t> PValue::tryAsInteger() const {
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch"
	switch(m_type) {
		case PType::Null: return {0};
		case PType::Unsigned: return {static_cast<int_t>(m_value.uVal)};
		case PType::Integer: return {m_value.iVal};
		case PType::Float: return {static_cast<int_t>(m_value.fVal)};
		case PType::Bool: return {static_cast<int_t>(m_value.bVal)};

		case PType::String: {
			try {
				return {std::stoll(m_value.sVal)};
			} catch(...) {
				return {};
			}
		}
	}
	#pragma clang diagnostic pop
	return {};
}

std::optional<PValue::float_t> PValue::tryAsFloat() const {
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch"
	switch(m_type) {
		case PType::Null: return {0};
		case PType::Unsigned: return {static_cast<float_t>(m_value.uVal)};
		case PType::Integer: return {static_cast<float_t>(m_value.uVal)};
		case PType::Float: return {m_value.fVal};
		case PType::Bool: return {static_cast<float_t>(m_value.bVal)};

		case PType::String: {
			try {
				return {std::stold(m_value.sVal)};
			} catch(...) {
				return {};
			}
		}
	}
	#pragma clang diagnostic pop
	return {};
}

std::optional<PValue::str_t> PValue::tryAsString() const {
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch"
	switch(m_type) {
		case PType::Null: return {""};
		case PType::Unsigned: return {std::to_string(m_value.uVal)};
		case PType::Integer: return {std::to_string(m_value.uVal)};
		case PType::Float: return {std::to_string(m_value.fVal)};
		case PType::Bool: return {m_value.bVal ? "true" : "false"};

		case PType::String: return {m_value.sVal};
	}
	#pragma clang diagnostic pop
	return {};
}

std::optional<PValue::bool_t> PValue::tryAsBool() const {
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch"
	switch(m_type) {
		case PType::Null: return {""};
		case PType::Unsigned: return {static_cast<bool_t>(m_value.uVal)};
		case PType::Integer: return {static_cast<bool_t>(m_value.uVal)};
		case PType::Float: return {static_cast<bool_t>(m_value.fVal)};
		case PType::Bool: return {m_value.bVal};

		case PType::String: {
			try {
				bool result;
				std::stringstream sstream(m_value.sVal);
				sstream >> std::boolalpha >> result;
				return {result};
			} catch(...) {
				return {};
			}
		}
	}
	#pragma clang diagnostic pop
	return {};
}

PValue::uint_t PValue::asUnsigned() const {
	return tryAsUnsigned().value();
}

PValue::int_t PValue::asInteger() const {
	return tryAsInteger().value();
}

PValue::float_t PValue::asFloat() const {
	return tryAsFloat().value();
}

PValue::str_t PValue::asString() const {
	return tryAsString().value();
}

PValue::bool_t PValue::asBool() const {
	return tryAsBool().value();
}

PType PValue::type() const {
	return m_type;
}

bool PValue::isObject() const {
	return m_type == PType::Object;
}

bool PValue::isArray() const {
	return m_type == PType::Array;
}

bool PValue::isUnsigned() const {
	return m_type == PType::Unsigned;
}

bool PValue::isInteger() const {
	return m_type == PType::Integer;
}

bool PValue::isString() const {
	return m_type == PType::String;
}

bool PValue::isBool() const {
	return m_type == PType::Bool;
}

PValue& PValue::operator[](const Path& path) {
	return navigate(path);
}

PValue& PValue::operator[](size_t id) {
	return (*this)[Path() << id];
}

PValue& PValue::operator[](const std::string& name) {
	return (*this)[Path() << name];
}

const PValue& PValue::operator[](const Path& path) const {
	const PValue* result = tryNavigate(path);
	if (result) return *result;
	throw std::out_of_range("PValue::operator[]: Path out of range");
}

PValue& PValue::operator=(const PValue& val) {
	setPValue(val);
	return *this;
}

PValue& PValue::operator=(PValue&& val) {
	setPValue(std::move(val));
	return *this;
}

static void traversePValue_internal(Path& path, const PValue& cNode, const std::function<void(const Path& path, TraverseAction action, const PValue& value)>& callback) {

	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch-enum"
	switch(cNode.type()) {
		case PType::Object: {

			callback(path, TraverseAction::ObjectStart, cNode);

			auto& object = cNode.objectValue();
			for(auto iter = object.begin(); iter != object.end(); iter++) {
				traversePValue_internal(path.append(iter->first), iter->second, callback);
				path.pop();
			}

			callback(path, TraverseAction::ObjectEnd, cNode);

			return;
		}

		case PType::Array: {

			callback(path, TraverseAction::ArrayStart, cNode);

			auto& array = cNode.arrayValue();
			for(size_t i = 0; i < array.size(); i++) {
				traversePValue_internal(path.append(i), array[i], callback);
				path.pop();
			}

			callback(path, TraverseAction::ArrayEnd, cNode);

			return;
		}

		default: {
			callback(path, TraverseAction::Value, cNode);
			return;
		}
	}
	#pragma clang diagnostic pop

}

void ak::data::traversePValue(const PValue& cNode, const std::function<void(const Path& path, TraverseAction action, const PValue& value)>& callback) {
	Path path;
	traversePValue_internal(path, cNode, callback);

}

































