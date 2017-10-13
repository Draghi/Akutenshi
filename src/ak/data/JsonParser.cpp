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
#include <ak/data/PValue.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <algorithm>
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>

#include "rapidjson/encodings.h"
#include "rapidjson/error/error.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace ak::data;

namespace rj = rapidjson;

struct JSONParser : public rj::BaseReaderHandler<rj::UTF8<>, JSONParser> {

		PValue& root;
		std::deque<PValue*> valueStack;
		std::string cKey;


		JSONParser(ak::data::PValue& a) : root(a), valueStack(), cKey("") {}

		void addPValue(PValue&& value) {

			ak::data::PValue* nValue;
			if (valueStack.size() > 0) {
				auto& tValue = *valueStack.back();

				if (tValue.isObj()) nValue = &tValue[cKey];
				else if (tValue.isArr()) nValue = &tValue[tValue.asArr().size()];
				else throw std::logic_error("JSONParser: Invalid parse state");
			} else {
				nValue = &root;
			}

			*nValue = std::move(value);
			if (nValue->isObj() || nValue->isArr()) valueStack.push_back(nValue);
		}

	    bool Null() {
	    	addPValue(ak::data::PValue());
	    	return true;
	    }

	    bool Bool(bool b) {
	    	addPValue(ak::data::PValue(b));
	    	return true;
	    }

	    bool Int(int i) {
	    	addPValue(ak::data::PValue(static_cast<int64>(i)));
	    	return true;
	    }

	    bool Uint(unsigned u) {
	    	addPValue(ak::data::PValue(static_cast<int64>(u)));
	    	return true;
	    }

	    bool Int64(int64_t i) {
	    	addPValue(ak::data::PValue(static_cast<int64>(i)));
	    	return true;
	    }

	    bool Uint64(uint64_t u) {
	    	addPValue(ak::data::PValue(static_cast<int64>(u)));
	    	return true;
	    }

	    bool Double(double d) {
	    	addPValue(ak::data::PValue(d));
	    	return true;
	    }

	    bool String(const char* str, rj::SizeType length, bool) {
	    	addPValue(ak::data::PValue(std::string(str, length)));
	        return true;
	    }

	    bool Key(const char* str, rj::SizeType length, bool) {
	    	cKey = std::string(str, length);
	        return true;
	    }

	    bool StartObject() {
	    	addPValue(ak::data::PValue(ak::data::PValue::obj_t()));
	    	return true;
	    }

	    bool EndObject(rj::SizeType) {
			valueStack.pop_back();
	    	return true;
	    }

	    bool StartArray() {
	    	addPValue(ak::data::PValue(ak::data::PValue::arr_t()));
	    	return true;
	    }

	    bool EndArray(rj::SizeType) {
			valueStack.pop_back();
	    	return true;
	    }
};

bool ak::data::deserializeJson(ak::data::PValue& dest, std::istream& jsonStream) {
	JSONParser handler(dest);
	rj::Reader reader;
	rj::IStreamWrapper stream(jsonStream);
	reader.Parse(stream, handler);
	return true;
}

std::string ak::data::serializeJson(const ak::data::PValue& src, bool pretty) {
    rj::StringBuffer s;

    rj::Writer<rj::StringBuffer> nWriter(s);
    rj::PrettyWriter<rj::StringBuffer> pWriter(s);

	ak::data::traversePValue(src, [pretty, &nWriter, &pWriter](const ak::data::Path& path, const ak::data::TraverseAction traverseAction, const ak::data::PValue& value) {


		if ((path.size() > 0) && (!path[path.size() - 1].isIndex) && (traverseAction != ak::data::TraverseAction::ObjectEnd) && (traverseAction != ak::data::TraverseAction::ArrayEnd)) {
			if (pretty) pWriter.Key(path[path.size()-1].path.c_str(), static_cast<rj::SizeType>(path[path.size()-1].path.size()));
			else nWriter.Key(path[path.size()-1].path.c_str(), static_cast<rj::SizeType>(path[path.size()-1].path.size()));
		}

		switch(traverseAction) {
			case ak::data::TraverseAction::ArrayStart: {
				if (pretty) pWriter.StartArray();
				else nWriter.StartArray();
				break;
			}

			case ak::data::TraverseAction::ArrayEnd: {
				if (pretty) pWriter.EndArray(static_cast<rj::SizeType>(value.asArr().size()));
				else nWriter.EndArray(static_cast<rj::SizeType>(value.asArr().size()));
				break;
			}

			case ak::data::TraverseAction::ObjectStart: {
				if (pretty) pWriter.StartObject();
				else nWriter.StartObject();
				break;
			}

			case ak::data::TraverseAction::ObjectEnd: {
				if (pretty) pWriter.EndObject(static_cast<rj::SizeType>(value.asObj().size()));
				else nWriter.EndObject(static_cast<rj::SizeType>(value.asObj().size()));
				break;
			}

			case ak::data::TraverseAction::Value: {
				#pragma clang diagnostic push
				#pragma clang diagnostic ignored "-Wswitch"
				switch(value.type()) {
					case ak::data::PType::Null:
						if (pretty) pWriter.Null();
						else nWriter.Null();
						break;

					case ak::data::PType::Boolean:
						if (pretty) pWriter.Bool(value.asBool());
						else nWriter.Bool(value.asBool());
						break;

					case ak::data::PType::Integer:
						if ((value.asInt() >= std::numeric_limits<int>::min()) && (value.asInt() <= std::numeric_limits<int>::max())) {
							if (pretty) pWriter.Int(static_cast<int>(value.asInt()));
							else nWriter.Int(static_cast<int>(value.asInt()));
						} else {
							if (pretty) pWriter.Int64(value.asInt());
							else nWriter.Int64(value.asInt());
						}
						break;

					case ak::data::PType::Decimal:
						if (pretty) pWriter.Double(value.asDec());
						else nWriter.Double(value.asDec());
						break;

					case ak::data::PType::String:
						if (pretty) pWriter.String(value.asStr().c_str(), static_cast<rj::SizeType>(value.asStr().size()));
						else nWriter.String(value.asStr().c_str(), static_cast<rj::SizeType>(value.asStr().size()));
						break;
				}
				#pragma clang diagnostic pop
				break;
			}
		}
	});

	return std::string(s.GetString(), s.GetSize());
}



