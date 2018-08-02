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

#include <ak/data/Json.hpp>

#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <rapidjson/encodings.h>
#include <rapidjson/error/en.h>
#include <rapidjson/error/error.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/reader.h>
#include <rapidjson/stream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <algorithm>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include <ak/data/Base64.hpp>
#include <ak/data/PValue.hpp>
#include <ak/data/PVPath.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/Log.hpp>
#include <ak/PrimitiveTypes.hpp>

using namespace akd;

namespace rj = rapidjson;

constexpr auto JSON_BASE64_IDENTIFIER = AK_STRING_VIEW("[$$BASE64$$]");

struct JSONParser : public rj::BaseReaderHandler<rj::UTF8<>, JSONParser> {

		//PValue& root;
		std::deque<PValue*> valueStack;
		std::string cKey;


		JSONParser(akd::PValue& a) : valueStack(), cKey("") { a.setNull(); valueStack.push_back(&a);}

		void addPValue(PValue&& value) {

			bool isObjOrArr = value.isObj() || value.isArr();

			akd::PValue& cValue = *valueStack.back();

			if (cValue.isObj()) {
				cValue.getObj().emplace(cKey, std::move(value));
				if (isObjOrArr) valueStack.push_back(&cValue.getObj().at(cKey));
			} else if (cValue.isArr()) {
				cValue.getArr().push_back(std::move(value));
				if (isObjOrArr) valueStack.push_back(&cValue.getArr().back());
			} else {
				cValue = value;
				if (!isObjOrArr) valueStack.pop_back();
			}
		}

	    bool Null() {
	    	addPValue(akd::PValue());
	    	return true;
	    }

	    bool Bool(bool b) {
	    	addPValue(akd::PValue::from<akd::PValue::bool_t>(b));
	    	return true;
	    }

	    bool Int(int i) {
	    	addPValue(akd::PValue::from(static_cast<akd::PValue::sint_t>(i)));
	    	return true;
	    }

	    bool Uint(unsigned u) {
	    	addPValue(akd::PValue::from(static_cast<akd::PValue::uint_t>(u)));
	    	return true;
	    }

	    bool Int64(int64_t i) {
	    	addPValue(akd::PValue::from(static_cast<akd::PValue::sint_t>(i)));
	    	return true;
	    }

	    bool Uint64(uint64_t u) {
	    	addPValue(akd::PValue::from(static_cast<akd::PValue::uint_t>(u)));
	    	return true;
	    }

	    bool Double(double d) {
	    	addPValue(akd::PValue::from(d));
	    	return true;
	    }

	    bool String(const char* str, rj::SizeType length, bool) {
	    	std::string inStr = std::string(str, length);
	    	if (inStr.substr(0, JSON_BASE64_IDENTIFIER.size()) == JSON_BASE64_IDENTIFIER) {
		    	addPValue(akd::PValue::from(akd::base64::decode(inStr.substr(JSON_BASE64_IDENTIFIER.size()))));
	    	} else {
		    	addPValue(akd::PValue::from(inStr));
	    	}

	        return true;
	    }

	    bool Key(const char* str, rj::SizeType length, bool) {
	    	cKey = std::string(str, length);
	        return true;
	    }

	    bool StartObject() {
	    	addPValue(akd::PValue::from(akd::PValue::obj_t()));
	    	return true;
	    }

	    bool EndObject(rj::SizeType) {
			valueStack.pop_back();
	    	return true;
	    }

	    bool StartArray() {
	    	addPValue(akd::PValue::from(akd::PValue::arr_t()));
	    	return true;
	    }

	    bool EndArray(rj::SizeType) {
			valueStack.pop_back();
	    	return true;
	    }
};

bool akd::fromJson(akd::PValue& dest, const std::string& jsonStr) {
	dest = akd::PValue();
	JSONParser handler(dest);
	rj::Reader reader;
	rj::StringStream stream(jsonStr.c_str());
	auto result = reader.Parse<rj::kParseTrailingCommasFlag | rj::kParseFullPrecisionFlag>(stream, handler);
	if (result.IsError()) {
		akl::Logger("Json").warn("JSON Parse error (Offset ", result.Offset(), "): ", rj::GetParseError_En(result.Code()));
		return false;
	}
	return true;
}

std::string akd::toJson(const akd::PValue& src, bool pretty) {
    rj::StringBuffer s;

    rj::Writer<rj::StringBuffer> nWriter(s);
    rj::PrettyWriter<rj::StringBuffer> pWriter(s);
    pWriter.SetFormatOptions(rj::PrettyFormatOptions::kFormatSingleLineArray);

	akd::traversePValue(src, [pretty, &nWriter, &pWriter](const akd::PVPath& path, const akd::TraverseAction traverseAction, const akd::PValue& value) {


		if ((path.size() > 0) && (!path[path.size() - 1].isIndex) && (traverseAction != akd::TraverseAction::ObjectEnd) && (traverseAction != akd::TraverseAction::ArrayEnd)) {
			if (pretty) pWriter.Key(path[path.size()-1].path.c_str(), path[path.size()-1].path.size());
			else nWriter.Key(path[path.size()-1].path.c_str(), path[path.size()-1].path.size());
		}

		switch(traverseAction) {
			case akd::TraverseAction::ArrayStart: {
				if (pretty) pWriter.StartArray();
				else nWriter.StartArray();
				break;
			}

			case akd::TraverseAction::ArrayEnd: {
				if (pretty) pWriter.EndArray(value.as<akd::PValue::arr_t>().size());
				else nWriter.EndArray(value.as<akd::PValue::arr_t>().size());
				break;
			}

			case akd::TraverseAction::ObjectStart: {
				if (pretty) pWriter.StartObject();
				else nWriter.StartObject();
				break;
			}

			case akd::TraverseAction::ObjectEnd: {
				if (pretty) pWriter.EndObject(value.as<akd::PValue::obj_t>().size());
				else nWriter.EndObject(value.as<akd::PValue::obj_t>().size());
				break;
			}

			case akd::TraverseAction::Value: {
				switch(value.type()) {
					case akd::PType::Object: throw std::logic_error("Cannot serialize object directly.");
					case akd::PType::Array:  throw std::logic_error("Cannot serialize array directly.");

					case akd::PType::Null:
						if (pretty) pWriter.Null();
						else nWriter.Null();
						break;

					case akd::PType::Boolean:
						if (pretty) pWriter.Bool(value.as<bool>());
						else nWriter.Bool(value.as<bool>());
						break;

					case akd::PType::Signed:
						if ((value.as<int64>() >= std::numeric_limits<int>::min()) && (value.as<int64>() <= std::numeric_limits<int>::max())) {
							if (pretty) pWriter.Int(value.as<int>());
							else nWriter.Int(value.as<int>());
						} else {
							if (pretty) pWriter.Int64(value.as<int64>());
							else nWriter.Int64(value.as<int64>());
						}
						break;

					case akd::PType::Unsigned:
						if ((value.as<uint64>() <= std::numeric_limits<unsigned>::max())) {
							if (pretty) pWriter.Uint(value.as<uint>());
							else nWriter.Uint(value.as<uint>());
						} else {
							if (pretty) pWriter.Uint64(value.as<uint64>());
							else nWriter.Uint64(value.as<uint64>());
						}
						break;

					case akd::PType::Decimal:
						if (pretty) pWriter.Double(value.as<fpDouble>());
						else nWriter.Double(value.as<fpDouble>());
						break;

					case akd::PType::String:
						if (pretty) pWriter.String(value.as<std::string>().c_str(), value.as<std::string>().size());
						else nWriter.String(value.as<std::string>().c_str(), value.as<std::string>().size());
						break;

					case akd::PType::Binary:
						std::string binStr = std::string(JSON_BASE64_IDENTIFIER) + akd::base64::encode(value.getBin().data(), value.getBin().size());
						if (pretty) pWriter.String(binStr.c_str(), binStr.size());
						else nWriter.String(binStr.c_str(), binStr.size());

				}
				break;
			}
		}
	});

	return std::string(s.GetString(), s.GetSize());
}

 bool akd::toJsonFile(const akd::PValue& src, const akfs::Path& filepath, bool pretty, bool overwrite) {
	akfs::CFile oFile(filepath, akfs::OpenFlags::Out | (overwrite ? akfs::OpenFlags::Truncate : 0x00));
	if (!oFile) return false;
	if (!oFile.writeLine(toJson(src, pretty), "")) return false;
	return true;
}

akd::PValue akd::fromJsonFile(const akfs::Path& filepath) {
	akfs::CFile inFile(filepath);
	std::string fileContents; if (!inFile.readAllLines(fileContents)) return akd::PValue();
	akd::PValue dst;          if (!fromJson(dst, fileContents))       return akd::PValue();
	return dst;
}

