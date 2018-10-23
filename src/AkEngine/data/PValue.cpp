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

#include <AkEngine/data/PValue.hpp>
#include <cstring>

using namespace akd;

// //////////// //
// // Values // //
// //////////// //

PValue::obj_t& PValue::getObj() {
	if (isObj()) return m_value.oVal;
	throw std::logic_error("PValue does not contain an object.");
}

PValue::arr_t& PValue::getArr() {
	if (isArr()) return m_value.aVal;
	throw std::logic_error("PValue does not contain an array.");
}

PValue::str_t& PValue::getStr() {
	if (isStr()) return m_value.sVal;
	throw std::logic_error("PValue does not contain an string.");
}

PValue::sint_t& PValue::getSInt() {
	if (isSInt()) return m_value.iVal;
	throw std::logic_error("PValue does not contain a signed integer.");
}

PValue::uint_t& PValue::getUInt() {
	if (isUInt()) return m_value.uVal;
	throw std::logic_error("PValue does not contain an unsigned integer.");
}

PValue::dec_t& PValue::getDec() {
	if (isDec()) return m_value.dVal;
	throw std::logic_error("PValue does not contain a floating point number.");
}

PValue::bool_t& PValue::getBool() {
	if (isBool()) return m_value.bVal;
	throw std::logic_error("PValue does not contain a boolean.");
}

PValue::bin_t& PValue::getBin() {
	if (isBin()) return m_value.binVal;
	throw std::logic_error("PValue does not contain binary data.");
}

PValue::obj_t& PValue::getObjOrSet(const PValue::obj_t& val) {
	if (!isObj()) setObj(val);
	return m_value.oVal;
}

PValue::arr_t& PValue::getArrOrSet(const PValue::arr_t& val) {
	if (!isArr()) setArr(val);
	return m_value.aVal;
}

PValue::str_t& PValue::getStrOrSet(const PValue::str_t& val) {
	if (!isStr()) setStr(val);
	return m_value.sVal;
}

PValue::sint_t& PValue::getSIntOrSet(const PValue::sint_t& val) {
	if (!isSInt()) setSInt(val);
	return m_value.iVal;
}

PValue::uint_t& PValue::getUIntOrSet(const PValue::uint_t& val) {
	if (!isUInt()) setUInt(val);
	return m_value.uVal;
}

PValue::dec_t& PValue::getDecOrSet(const PValue::dec_t& val) {
	if (!isDec()) setDec(val);
	return m_value.dVal;
}

PValue::bool_t& PValue::getBoolOrSet(const PValue::bool_t& val) {
	if (!isBool()) setBool(val);
	return m_value.bVal;
}

PValue::bin_t& PValue::getBinOrSet(const PValue::bin_t& val) {
	if (!isBin()) setBin(val);
	return m_value.binVal;
}


const PValue::obj_t& PValue::getObj() const {
	if (isObj()) return m_value.oVal;
	throw std::logic_error("PValue does not contain an object.");
}

const PValue::arr_t& PValue::getArr() const {
	if (isArr()) return m_value.aVal;
	throw std::logic_error("PValue does not contain an array.");
}

const PValue::str_t& PValue::getStr() const {
	if (isStr()) return m_value.sVal;
	throw std::logic_error("PValue does not contain an string.");
}

PValue::sint_t PValue::getSInt() const {
	if (isSInt()) return m_value.iVal;
	throw std::logic_error("PValue does not contain a signed integer.");
}

PValue::uint_t PValue::getUInt() const {
	if (isUInt()) return m_value.uVal;
	throw std::logic_error("PValue does not contain an unsigned integer.");
}

PValue::dec_t PValue::getDec() const {
	if (isDec()) return m_value.dVal;
	throw std::logic_error("PValue does not contain a floating point number.");
}

PValue::bool_t PValue::getBool() const {
	if (isBool()) return m_value.bVal;
	throw std::logic_error("PValue does not contain a boolean.");
}

const PValue::bin_t& PValue::getBin() const {
	if (isBin()) return m_value.binVal;
	throw std::logic_error("PValue does not contain binary data.");
}

const PValue::obj_t& PValue::getObjOrDef(const PValue::obj_t& val) const {
	if (isObj()) return m_value.oVal;
	return val;
}

const PValue::arr_t& PValue::getArrOrDef(const PValue::arr_t& val) const {
	if (isArr()) return m_value.aVal;
	return val;
}

const PValue::str_t& PValue::getStrOrDef(const PValue::str_t& val) const {
	if (isStr()) return m_value.sVal;
	return val;
}

const PValue::sint_t& PValue::getSIntOrDef(const PValue::sint_t& val) const {
	if (isSInt()) return m_value.iVal;
	return val;
}

const PValue::uint_t& PValue::getUIntOrDef(const PValue::uint_t& val) const {
	if (isUInt()) return m_value.uVal;
	return val;
}

const PValue::dec_t& PValue::getDecOrDef(const PValue::dec_t& val) const {
	if (isDec()) return m_value.dVal;
	return val;
}

const PValue::bool_t& PValue::getBoolOrDef(const PValue::bool_t& val) const {
	if (isBool()) return m_value.bVal;
	return val;
}

const PValue::bin_t& PValue::getBinOrDef(const PValue::bin_t& val) const {
	if (isBin()) return m_value.binVal;
	return val;
}


/*void PValue::merge(const PValue& other, bool override) {
	if (isObj() && other.isObj()) {
		for(auto iter = other.asObj().begin(); iter != other.asObj().end(); iter++) {
			auto existingVal = asObj().find(iter->first);
			if (existingVal == asObj().end()) asObj().insert(*iter);
			else existingVal->second.merge(iter->second, override);
		}
	} else if (override) {
		*this = other;
	}
}*/

// //////////////// //
// // Assignment // //
// //////////////// //

PValue& PValue::setNull() {
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch"
	switch(m_type) {
		case PType::Object: m_value.oVal.~map(); break;
		case PType::Array: m_value.aVal.~deque(); break;
		case PType::String: m_value.sVal.~basic_string(); break;
	}
	#pragma clang diagnostic pop
	m_type = PType::Null;
	return *this;
}

PValue& PValue::setPValue(const PValue& val) {
	switch(val.m_type) {
		case PType::Null: setNull(); break;

		case PType::Object: setObj(val.m_value.oVal); break;
		case PType::Array: setArr(val.m_value.aVal); break;
		case PType::String: setStr(val.m_value.sVal); break;

		case PType::Signed: setSInt(val.m_value.iVal); break;
		case PType::Unsigned:setUInt(val.m_value.uVal); break;
		case PType::Decimal: setDec(val.m_value.dVal); break;
		case PType::Boolean: setBool(val.m_value.bVal); break;

		case PType::Binary: setBin(val.m_value.binVal); break;
	}
	return *this;
}

PValue& PValue::setObj(const obj_t& val) {
	if (!isObj()) setNull();
	new(&m_value.oVal) obj_t(val);
	m_type = PType::Object;
	return *this;
}

PValue& PValue::setArr(const arr_t& val) {
	if (!isArr()) setNull();
	new(&m_value.aVal) arr_t(val);
	m_type = PType::Array;
	return *this;
}

PValue& PValue::setStr(const str_t& val) {
	if (!isStr()) setNull();
	new(&m_value.sVal) std::string(val);
	m_type = PType::String;
	return *this;
}

PValue& PValue::setSInt(const sint_t& val) {
	if (!isSInt()) setNull();
	m_value.iVal = val;
	m_type = PType::Signed;
	return *this;
}

PValue& PValue::setUInt(const uint_t& val) {
	if (!isUInt()) setNull();
	m_value.uVal = val;
	m_type = PType::Unsigned;
	return *this;
}

PValue& PValue::setDec(const dec_t& val) {
	if (!isUInt()) setNull();
	m_value.dVal = val;
	m_type = PType::Decimal;
	return *this;
}

PValue& PValue::setBool(const bool_t& val) {
	if (!isBool()) setNull();
	m_value.bVal = val;
	m_type = PType::Boolean;
	return *this;
}

PValue& PValue::setBin(const bin_t& val) {
	if (!isBin()) setNull();
	new(&m_value.binVal) bin_t(val);
	m_type = PType::Binary;
	return *this;
}

PValue& PValue::setBin(const void* val, akSize size) {
	bin_t dst; dst.resize(size);
	std::memcpy(dst.data(), val, size);
	return setBin(std::move(dst));
}

static void traversePValue_internal(PVPath& path, const PValue& cNode, const std::function<void(const PVPath& path, TraverseAction action, const PValue& value)>& callback) {
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch-enum"
	switch(cNode.type()) {
		case PType::Object: {

			callback(path, TraverseAction::ObjectStart, cNode);

			const auto& obj = cNode.as<akd::PValue::obj_t>();
			for(auto iter = obj.begin(); iter != obj.end(); iter++) {
				traversePValue_internal(path.append(iter->first), iter->second, callback);
				path.pop();
			}

			callback(path, TraverseAction::ObjectEnd, cNode);

			return;
		}

		case PType::Array: {

			callback(path, TraverseAction::ArrayStart, cNode);

			const auto& arr = cNode.as<akd::PValue::arr_t>();
			for(size_t i = 0; i < arr.size(); i++) {
				traversePValue_internal(path.append(i), arr[i], callback);
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

void akd::traversePValue(const PValue& cNode, const std::function<void(const PVPath& path, TraverseAction action, const PValue& value)>& callback) {
	PVPath path;
	traversePValue_internal(path, cNode, callback);

}

































