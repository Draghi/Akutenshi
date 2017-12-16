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

#include <ak/data/JsonParser.hpp>
#include <ak/data/Path.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>
#include <algorithm>
#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <utility>

#include "rapidjson/encodings.h"
#include "rapidjson/error/error.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace akd;

namespace rj = rapidjson;

struct JSONParser : public rj::BaseReaderHandler<rj::UTF8<>, JSONParser> {

		//PValue& root;
		std::deque<PValue*> valueStack;
		std::string cKey;


		JSONParser(akd::PValue& a) : valueStack(), cKey("") { a.setNull(); valueStack.push_back(&a);}

		void addPValue(PValue&& value) {

			bool isObjOrArr = value.isObj() || value.isArr();

			akd::PValue& cValue = *valueStack.back();

			if (cValue.isObj()) {
				cValue.asObj().insert(std::make_pair(cKey, std::move(value)));
				if (isObjOrArr) valueStack.push_back(&cValue.asObj().at(cKey));
			} else if (cValue.isArr()) {
				cValue.asArr().push_back(std::move(value));
				if (isObjOrArr) valueStack.push_back(&cValue.asArr().back());
			} else {
				cValue.set<akd::PValue>(value);
				if (!isObjOrArr) valueStack.pop_back();
			}
		}

	    bool Null() {
	    	addPValue(akd::PValue());
	    	return true;
	    }

	    bool Bool(bool b) {
	    	addPValue(akd::PValue(b));
	    	return true;
	    }

	    bool Int(int i) {
	    	addPValue(akd::PValue(static_cast<akd::PValue::int_t>(i)));
	    	return true;
	    }

	    bool Uint(unsigned u) {
	    	addPValue(akd::PValue(static_cast<akd::PValue::uint_t>(u)));
	    	return true;
	    }

	    bool Int64(int64_t i) {
	    	addPValue(akd::PValue(static_cast<akd::PValue::int_t>(i)));
	    	return true;
	    }

	    bool Uint64(uint64_t u) {
	    	addPValue(akd::PValue(static_cast<akd::PValue::uint_t>(u)));
	    	return true;
	    }

	    bool Double(double d) {
	    	addPValue(akd::PValue(d));
	    	return true;
	    }

	    bool String(const char* str, rj::SizeType length, bool) {
	    	addPValue(akd::PValue(std::string(str, length)));
	        return true;
	    }

	    bool Key(const char* str, rj::SizeType length, bool) {
	    	cKey = std::string(str, length);
	        return true;
	    }

	    bool StartObject() {
	    	addPValue(akd::PValue(akd::PValue::obj_t()));
	    	return true;
	    }

	    bool EndObject(rj::SizeType) {
			valueStack.pop_back();
	    	return true;
	    }

	    bool StartArray() {
	    	addPValue(akd::PValue(akd::PValue::arr_t()));
	    	return true;
	    }

	    bool EndArray(rj::SizeType) {
			valueStack.pop_back();
	    	return true;
	    }
};

bool akd::deserializeJson(akd::PValue& dest, std::istream& jsonStream) {
	dest = akd::PValue();
	JSONParser handler(dest);
	rj::Reader reader;
	rj::IStreamWrapper stream(jsonStream);
	reader.Parse(stream, handler);
	return true;
}

std::string akd::serializeJson(const akd::PValue& src, bool pretty) {
    rj::StringBuffer s;

    rj::Writer<rj::StringBuffer> nWriter(s);
    rj::PrettyWriter<rj::StringBuffer> pWriter(s);

	akd::traversePValue(src, [pretty, &nWriter, &pWriter](const akd::Path& path, const akd::TraverseAction traverseAction, const akd::PValue& value) {


		if ((path.size() > 0) && (!path[path.size() - 1].isIndex) && (traverseAction != akd::TraverseAction::ObjectEnd) && (traverseAction != akd::TraverseAction::ArrayEnd)) {
			if (pretty) pWriter.Key(path[path.size()-1].path.c_str(), static_cast<rj::SizeType>(path[path.size()-1].path.size()));
			else nWriter.Key(path[path.size()-1].path.c_str(), static_cast<rj::SizeType>(path[path.size()-1].path.size()));
		}

		switch(traverseAction) {
			case akd::TraverseAction::ArrayStart: {
				if (pretty) pWriter.StartArray();
				else nWriter.StartArray();
				break;
			}

			case akd::TraverseAction::ArrayEnd: {
				if (pretty) pWriter.EndArray(static_cast<rj::SizeType>(value.as<akd::PValue::arr_t>().size()));
				else nWriter.EndArray(static_cast<rj::SizeType>(value.as<akd::PValue::arr_t>().size()));
				break;
			}

			case akd::TraverseAction::ObjectStart: {
				if (pretty) pWriter.StartObject();
				else nWriter.StartObject();
				break;
			}

			case akd::TraverseAction::ObjectEnd: {
				if (pretty) pWriter.EndObject(static_cast<rj::SizeType>(value.as<akd::PValue::obj_t>().size()));
				else nWriter.EndObject(static_cast<rj::SizeType>(value.as<akd::PValue::obj_t>().size()));
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

					case akd::PType::Integer:
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
							if (pretty) pWriter.Uint(static_cast<unsigned>(value.as<uint>()));
							else nWriter.Uint(static_cast<unsigned>(value.as<uint>()));
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
						if (pretty) pWriter.String(value.as<std::string>().c_str(), static_cast<rj::SizeType>(value.as<std::string>().size()));
						else nWriter.String(value.as<std::string>().c_str(), static_cast<rj::SizeType>(value.as<std::string>().size()));
						break;
				}
				break;
			}
		}
	});

	return std::string(s.GetString(), s.GetSize());
}



