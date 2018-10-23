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

#include <AkEngine/data/Brotli.hpp>
#include <AkEngine/data/MsgPack.hpp>
#include <AkEngine/data/PValue.hpp>
#include <AkEngine/data/PVPath.hpp>
#include <AkEngine/filesystem/CFile.hpp>
#include <AkEngine/filesystem/Path.hpp>
#include <AkCommon/PrimitiveTypes.hpp>
#include <msgpack/v1/object.hpp>
#include <msgpack/v1/pack.hpp>
#include <msgpack/v1/sbuffer.hpp>
#include <msgpack/v2/null_visitor.hpp>
#include <msgpack/v2/parse.hpp>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <initializer_list>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


using namespace akd;

#include <msgpack.hpp>

struct MSGPackVistor : public msgpack::v2::null_visitor {
	bool isKeyValue;
	std::pair<std::string, akd::PValue> nextValue;
	std::vector<std::pair<std::string, akd::PValue>> stack;

	MSGPackVistor() : isKeyValue(false), nextValue("", akd::PValue()), stack() {}

    // Value
    bool visit_nil() {
    	nextValue.second = akd::PValue();
        return true;
    }

    bool visit_boolean(bool v) {
    	nextValue.second = akd::PValue::from(v);
        return true;
    }

    bool visit_positive_integer(uint64_t v) {
    	nextValue.second = akd::PValue::from(v);
        return true;
    }

    bool visit_negative_integer(int64_t v) {
    	nextValue.second = akd::PValue::from(v);
        return true;
    }

    bool visit_float32(float v) {
    	nextValue.second = akd::PValue::from(v);
        return true;
    }

    bool visit_float64(double v) {
    	nextValue.second = akd::PValue::from(v);
        return true;
    }

    bool visit_str(const char* v, uint32_t size) {
    	if (isKeyValue) nextValue.first = std::string(v, size);
    	else nextValue.second = akd::PValue::from(std::string(v, size));
        return true;
    }

    bool visit_bin(const char* v, uint32_t size) {
    	std::vector<uint8> data;
    	data.resize(size);
    	std::memcpy(data.data(), v, size);
    	nextValue.second = akd::PValue::from(data);
        return true;
    }

    bool visit_ext(const char* v, uint32_t size) {
    	nextValue.second = akd::PValue::from(std::string(v, size));
        return true;
    }

    // Array

    bool start_array(uint32_t /*count*/) {
    	nextValue.second = akd::PValue().setArr();
    	stack.push_back(nextValue);
        return true;
    }

    bool end_array() {
    	nextValue = stack.back();
    	stack.pop_back();
        return true;
    }

    bool start_array_item() {
    	nextValue = std::make_pair("", akd::PValue());
        return true;
    }

    bool end_array_item() {
    	stack.back().second.getArr().push_back(nextValue.second);
        return true;
    }

    // Object

    bool start_map(uint32_t /*count*/) {
    	nextValue.second = akd::PValue().setObj();
    	stack.push_back(nextValue);
        return true;
    }

    bool end_map() {
    	nextValue = stack.back();
    	stack.pop_back();
        return true;
    }

    bool start_map_key() {
    	nextValue.first = "";
    	isKeyValue = true;
        return true;
    }

    bool end_map_key() {
    	isKeyValue = false;
        return true;
    }

    bool start_map_value() {
    	nextValue.second = akd::PValue();
        return true;
    }

    bool end_map_value() {
    	stack.back().second.getObj().insert(nextValue);
        return true;
    }

    // Error
    void parse_error(size_t /*parsed_offset*/, size_t /*error_offset*/) {}
    void insufficient_bytes(size_t /*parsed_offset*/, size_t /*error_offset*/) {}
};

std::vector<uint8> akd::toMsgPack(const akd::PValue& src) {

    msgpack::sbuffer buffer;
    msgpack::packer<msgpack::sbuffer> pk(&buffer);

	akd::traversePValue(src, [&pk](const akd::PVPath& path, const akd::TraverseAction traverseAction, const akd::PValue& value) {
		if ((path.size() > 0) && (!path[path.size() - 1].isIndex) && (traverseAction != akd::TraverseAction::ObjectEnd) && (traverseAction != akd::TraverseAction::ArrayEnd)) {
			pk.pack(path[path.size()-1].path);
		}

		switch(traverseAction) {
			case akd::TraverseAction::ArrayStart: pk.pack_array(value.getArr().size()); break;
			case akd::TraverseAction::ObjectStart: pk.pack_map(value.getObj().size()); break;

			case akd::TraverseAction::Value: {
				switch(value.type()) {
					case akd::PType::Object: throw std::logic_error("Cannot serialize object directly.");
					case akd::PType::Array:  throw std::logic_error("Cannot serialize array directly.");
					case akd::PType::Null: pk.pack_nil(); break;
					case akd::PType::Boolean: pk.pack(value.getBool()); break;
					case akd::PType::Signed: pk.pack(value.getSInt()); break;
					case akd::PType::Unsigned: pk.pack(value.getUInt()); break;
					case akd::PType::Decimal: pk.pack(value.getDec()); break;
					case akd::PType::String: pk.pack(value.getStr()); break;
					case akd::PType::Binary: pk.pack(value.getBin()); break;
				}
				break;
			}

			case akd::TraverseAction::ArrayEnd: break;
			case akd::TraverseAction::ObjectEnd: break;
		}
	});

	std::vector<uint8> result;
	result.resize(buffer.size());
	std::memcpy(result.data(), buffer.data(), buffer.size());
	return result;
}

bool akd::fromMsgPack(akd::PValue& dest, const std::vector<uint8>& msgPackStream) {
	MSGPackVistor vistor;
	std::size_t off = 0;
	if (!msgpack::v2::parse(reinterpret_cast<const char*>(msgPackStream.data()), msgPackStream.size(), off, vistor)) return false;
	dest = vistor.nextValue.second;
	return true;
}

bool akd::toMsgPackFile(const akd::PValue& src, const akfs::Path& filepath, bool compress, bool overwrite) {
	akfs::CFile oFile(filepath, akfs::OpenFlags::Out | (overwrite ? akfs::OpenFlags::Truncate : 0x00));
	if (!oFile) return false;
	auto fileData = toMsgPack(src);
	if (compress) fileData = akd::compressBrotli(fileData);
	if (!oFile.write(fileData.data(), fileData.size())) return false;
	return true;
}

akd::PValue akd::fromMsgPackFile(const akfs::Path& filepath, bool decompress) {
	akfs::CFile inFile(filepath);
	std::vector<uint8> fileContents; if (!inFile.readAll(fileContents)) return akd::PValue();
	if (decompress) fileContents = akd::decompressBrotli(fileContents);
	akd::PValue dst; if (!fromMsgPack(dst, fileContents)) return akd::PValue();
	return dst;
}
