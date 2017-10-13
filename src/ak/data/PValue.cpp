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

static const PValue dataPValueNull;

PValue& PValue::navigate_internal(PValue& cNode, const Path& path) {

	PValue* currentNode = &cNode;
	for(size_t i = 0; i < path.size(); i++) {
		auto entry = path.entry(i);
		if (entry.isIndex) {
			if (currentNode->m_type != PType::Array) currentNode->setArr();
			if (entry.index >= currentNode->m_value.aVal.size()) currentNode->m_value.aVal.resize(entry.index + 1);

			currentNode = &currentNode->m_value.aVal[entry.index];
			continue;

		} else {
			if (currentNode->m_type != PType::Object) currentNode->setObj();

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
		case PType::Object: setObj(val.m_value.oVal); break;
		case PType::Array: setArr(val.m_value.aVal); break;
		case PType::String: setStr(val.m_value.sVal); break;

		case PType::Integer: setInt(val.m_value.iVal); break;
		case PType::Decimal: setDec(val.m_value.dVal); break;
		case PType::Boolean: setBool(val.m_value.bVal); break;
	}
	#pragma clang diagnostic pop
}

void PValue::setPValue(PValue&& val) {
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch"
	switch(val.m_type) {
		case PType::Object: setObj(std::move(val.m_value.oVal)); break;
		case PType::Array: setArr(std::move(val.m_value.aVal)); break;
		case PType::String: setStr(std::move(val.m_value.sVal)); break;

		case PType::Integer: setInt(std::move(val.m_value.iVal)); break;
		case PType::Decimal: setDec(std::move(val.m_value.dVal)); break;
		case PType::Boolean: setBool(std::move(val.m_value.bVal)); break;
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
	setObj(val);
}

PValue::PValue(const arr_t& val) : m_type(PType::Null) {
	setArr(val);
}

PValue::PValue(const str_t& val) : m_type(PType::Null) {
	setStr(val);
}

PValue::PValue(const int_t& val) : m_type(PType::Null) {
	setInt(val);
}

PValue::PValue(const dec_t& val) : m_type(PType::Null) {
	setDec(val);
}

PValue::PValue(const bool_t& val) : m_type(PType::Null) {
	setBool(val);
}

PValue::PValue(null_t&&) : m_type(PType::Null) {

}

PValue::PValue(obj_t&& val) : m_type(PType::Null) {
	setObj(std::move(val));
}

PValue::PValue(arr_t&& val) : m_type(PType::Null) {
	setArr(std::move(val));
}

PValue::PValue(str_t&& val) : m_type(PType::Null) {
	setStr(std::move(val));
}

PValue::PValue(int_t&& val) : m_type(PType::Null) {
	setInt(std::move(val));
}

PValue::PValue(dec_t&& val) : m_type(PType::Null) {
	setDec(std::move(val));
}

PValue::PValue(bool_t&& val) : m_type(PType::Null) {
	setBool(std::move(val));
}

PValue::~PValue() {
	setNull();
}

PValue& PValue::get(const Path& path){
	return navigate_internal(*this, path);
}

PValue* PValue::tryGet(const Path& path) {
	return const_cast<PValue*>(navigate_internal(const_cast<const PValue*>(this), path));
}

const PValue* PValue::tryGet(const Path& path) const {
	return navigate_internal(this, path);
}

const PValue& PValue::getOrNull(const Path& path) const {
	auto result = navigate_internal(this, path);
	if (result) return *result;
	return dataPValueNull;
}

bool PValue::exists(const Path& path) const {
	return tryGet(path) != nullptr;
}

void PValue::merge(const PValue& other, bool override) {
	if (isObj() && other.isObj()) {
		for(auto iter = other.asObj().begin(); iter != other.asObj().end(); iter++) {
			auto existingVal = asObj().find(iter->first);
			if (existingVal == asObj().end()) asObj().insert(*iter);
			else existingVal->second.merge(iter->second, override);
		}
	} else if (override) {
		*this = other;
	}
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

void PValue::setObj() {
	setObj(obj_t());
}

void PValue::setArr() {
	setArr(arr_t());
}

void PValue::setObj(const obj_t& val) {
	setNull();
	new(&m_value.oVal) obj_t(val);
	m_type = PType::Object;
}

void PValue::setArr(const arr_t& val) {
	setNull();
	new(&m_value.aVal) arr_t(val);
	m_type = PType::Array;
}

void PValue::setStr(const str_t& val) {
	setNull();
	new(&m_value.sVal) std::string(val);
	m_type = PType::String;
}

void PValue::setInt(const int_t& val) {
	setNull();
	m_value.iVal = val;
	m_type = PType::Integer;
}

void PValue::setDec(const dec_t& val) {
	setNull();
	m_value.dVal = val;
	m_type = PType::Decimal;
}

void PValue::setBool(const bool_t& val) {
	setNull();
	m_value.bVal = val;
	m_type = PType::Boolean;
}

void traversePValue(const PValue& rootNode, const std::function<void(const Path& path, TraverseAction action, const PValue& value)>& callback);

PValue::obj_t& PValue::asObj() {
	if (isObj()) return m_value.oVal;
	throw std::logic_error("PValue::value: Doesn't contain object value");
}

PValue::arr_t& PValue::asArr() {
	if (isArr()) return m_value.aVal;
	throw std::logic_error("PValue::value: Doesn't contain array value");
}

PValue::str_t& PValue::asStr() {
	if (isStr()) return m_value.sVal;
	throw std::logic_error("PValue::value: Doesn't contain string value");
}

PValue::int_t& PValue::asInt() {
	if (isInt()) return m_value.iVal;
	throw std::logic_error("PValue::value: Doesn't contain integer value");
}

PValue::dec_t& PValue::asDec() {
	if (isDec()) return m_value.dVal;
	throw std::logic_error("PValue::value: Doesn't contain decimal value");
}

PValue::bool_t& PValue::asBool() {
	if (isBool()) return m_value.bVal;
	throw std::logic_error("PValue::value: Doesn't contain decimal value");
}

PValue::obj_t& PValue::asObj(const obj_t& def) {
	if (isObj()) return m_value.oVal;
	setObj(def);
	return asObj();
}

PValue::arr_t& PValue::asArr(const arr_t& def) {
	if (isArr()) return m_value.aVal;
	setArr(def);
	return asArr();
}

PValue::str_t& PValue::asStr(const str_t& def) {
	if (isStr()) return m_value.sVal;
	setStr(def);
	return asStr();
}

PValue::int_t& PValue::asInt(int_t def) {
	if (isInt()) return m_value.iVal;
	setInt(def);
	return asInt();
}

PValue::dec_t& PValue::asDec(dec_t def) {
	if (isDec()) return m_value.dVal;
	setDec(def);
	return asDec();
}

PValue::bool_t& PValue::asBool(bool_t def) {
	if (isBool()) return m_value.bVal;
	setBool(def);
	return asBool();
}

const PValue::obj_t& PValue::asObj() const {
	if (isObj()) return m_value.oVal;
	throw std::logic_error("PValue::value: Doesn't contain object value");
}

const PValue::arr_t& PValue::asArr() const {
	if (isArr()) return m_value.aVal;
	throw std::logic_error("PValue::value: Doesn't contain array value");
}

const PValue::str_t& PValue::asStr() const {
	if (isStr()) return m_value.sVal;
	throw std::logic_error("PValue::value: Doesn't contain string value");
}

PValue::int_t PValue::asInt() const {
	if (isInt()) return m_value.iVal;
	throw std::logic_error("PValue::value: Doesn't contain integer value");
}

PValue::dec_t PValue::asDec() const {
	if (isDec()) return m_value.dVal;
	throw std::logic_error("PValue::value: Doesn't contain decimal value");
}

PValue::bool_t PValue::asBool() const {
	if (isBool()) return m_value.bVal;
	throw std::logic_error("PValue::value: Doesn't contain bool value");
}

const PValue::obj_t& PValue::asObjOr(const obj_t& def) const {
	if (isObj()) return m_value.oVal;
	return def;
}

const PValue::arr_t& PValue::asArrOr(const arr_t& def) const {
	if (isArr()) return m_value.aVal;
	return def;
}

const PValue::str_t& PValue::asStrOr(const str_t& def) const {
	if (isStr()) return m_value.sVal;
	return def;
}

PValue::int_t PValue::asIntOr(int_t def) const {
	if (isInt()) return m_value.iVal;
	return def;
}

PValue::dec_t PValue::asDecOr(dec_t def) const {
	if (isDec()) return m_value.dVal;
	return def;
}

PValue::bool_t PValue::asBoolOr(bool_t def) const {
	if (isBool()) return m_value.bVal;
	return def;
}

const PValue::obj_t* PValue::asObjPtr() const {
	if (isObj()) return &m_value.oVal;
	return nullptr;
}

const PValue::arr_t* PValue::asArrPtr() const {
	if (isArr()) return &m_value.aVal;
	return nullptr;
}

const PValue::str_t* PValue::asStrPtr() const {
	if (isStr()) return &m_value.sVal;
	return nullptr;
}

const PValue::int_t* PValue::asIntPtr() const {
	if (isInt()) return &m_value.iVal;
	return nullptr;
}

const PValue::dec_t* PValue::asDecPtr() const {
	if (isDec()) return &m_value.dVal;
	return nullptr;
}

const PValue::bool_t* PValue::asBoolPtr() const {
	if (isBool()) return &m_value.bVal;
	return nullptr;
}

PType PValue::type() const {
	return m_type;
}

bool PValue::isNull() const {
	return m_type == PType::Null;
}

bool PValue::isObj() const {
	return m_type == PType::Object;
}

bool PValue::isArr() const {
	return m_type == PType::Array;
}

bool PValue::isStr() const {
	return m_type == PType::String;
}

bool PValue::isInt() const {
	return m_type == PType::Integer;
}

bool PValue::isDec() const {
	return m_type == PType::Decimal;
}

bool PValue::isBool() const {
	return m_type == PType::Boolean;
}

PValue& PValue::operator[](const Path& path) {
	return get(path);
}

const PValue& PValue::operator[](const Path& path) const {
	auto result = tryGet(path);
	if (result) return *result;
	throw std::out_of_range("PValue::operator[]: Attempt to access path that's out of range");
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

			auto& object = cNode.asObj();
			for(auto iter = object.begin(); iter != object.end(); iter++) {
				traversePValue_internal(path.append(iter->first), iter->second, callback);
				path.pop();
			}

			callback(path, TraverseAction::ObjectEnd, cNode);

			return;
		}

		case PType::Array: {

			callback(path, TraverseAction::ArrayStart, cNode);

			auto& array = cNode.asArr();
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

































