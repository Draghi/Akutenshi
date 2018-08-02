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

#ifndef AK_ASSETS_SKIN_HPP_
#define AK_ASSETS_SKIN_HPP_

#include <deque>
#include <stdexcept>
#include <string>
#include <vector>

#include <ak/data/PValue.hpp>
#include <ak/util/Iterator.hpp>
#include <ak/math/Serialize.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>

namespace akas {

	struct Joint final {
		std::string name;
		std::vector<uint32> children;
		akm::Mat4 inverseBindMatrix;
		akm::Vec3 position;
		akm::Quat rotation;
		akm::Vec3 scale;
	};

	// @todo revise
	struct Skin final {
		int32 root;
		std::vector<Joint> joints;
		std::vector<uint32> mapping;
	};

}

namespace akd {
	inline bool deserialize(akas::Joint& dst, const akd::PValue& src) {
		try {
			akas::Joint result;
			if (!deserialize(result.name             , src["name"]             )) return false;
			if (!deserialize(result.children         , src["children"]         )) return false;
			if (!deserialize(result.inverseBindMatrix, src["inverseBindMatrix"])) return false;
			if (!deserialize(result.position         , src["position"]         )) return false;
			if (!deserialize(result.rotation         , src["rotation"]         )) return false;
			if (!deserialize(result.scale            , src["scale"]            )) return false;
			dst = result;
			return true;
		} catch(const std::logic_error&) { return false; }
	}

	inline void serialize(akd::PValue& dst, const akas::Joint& src) {
		serialize(dst["name"],              src.name);
		serialize(dst["children"],          src.children);
		serialize(dst["inverseBindMatrix"], src.inverseBindMatrix);
		serialize(dst["position"],          src.position);
		serialize(dst["rotation"],          src.rotation);
		serialize(dst["scale"],             src.scale);
	}

	inline bool deserialize(akas::Skin& dst, const akd::PValue& src) {
		try {
			dst = {
				src["root"].as<int32>(),
				aku::convert_to<std::vector<akas::Joint>>(src["joints"].getArr(), [](const auto& val){ return deserialize<akas::Joint>(val); }),
				aku::convert_to<std::vector<uint32>>(src["mapping"].getArr(), [](const auto& val){ return val.template as<uint32>(); })
			};
			return true;
		} catch(const std::logic_error&) { return false; }
	}

	inline void serialize(akd::PValue& dst, const akas::Skin& src) {
		dst["root"].set<int32>(src.root);
		dst["joints"].setArr(); for(auto joint : src.joints) { dst["joints"].getArr().push_back(akd::PValue()); serialize(dst["joints"].getArr().back(), joint); }
		dst["mapping"].setArr(); for(auto mapping : src.mapping) dst["mapping"].getArr().push_back(akd::PValue::from<uint32>(mapping));
	}

}

#endif /* AK_ASSETS_SKIN_HPP_ */