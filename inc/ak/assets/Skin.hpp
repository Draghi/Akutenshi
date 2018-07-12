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

#ifndef AK_ASSETS_SKIN_HPP_
#define AK_ASSETS_SKIN_HPP_

#include <ak/data/PValue.hpp>
#include <ak/Iterator.hpp>
#include <ak/math/Types.hpp>
#include <ak/math/Serialize.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <string>
#include <vector>

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
			dst = {
				src["name"].asStr(),
				ak::convert_to<std::vector<uint32>>(src["children"].asArr(), [](const auto& val){ return val.template as<uint32>(); }),
				deserialize<akm::Mat4>(src["inverseBindMatrix"]),
				deserialize<akm::Vec3>(src["position"]),
				deserialize<akm::Quat>(src["rotation"]),
				deserialize<akm::Vec3>(src["scale"])
			};
			return true;
		} catch(const std::logic_error&) { return false; }
	}

	inline void serialize(akd::PValue& dst, const akas::Joint& src) {
		dst["name"].setStr(src.name);
		dst["children"].setArr(); for(auto child : src.children) dst["children"].asArr().push_back(akd::PValue::from<uint32>(child));
		serialize(dst["inverseBindMatrix"], src.inverseBindMatrix);
		serialize(dst["position"],          src.position);
		serialize(dst["rotation"],          src.rotation);
		serialize(dst["scale"],             src.scale);
	}

	inline bool deserialize(akas::Skin& dst, const akd::PValue& src) {
		try {
			dst = {
				src["root"].as<int32>(),
				ak::convert_to<std::vector<akas::Joint>>(src["joints"].asArr(), [](const auto& val){ return deserialize<akas::Joint>(val); }),
				ak::convert_to<std::vector<uint32>>(src["mapping"].asArr(), [](const auto& val){ return val.template as<uint32>(); })
			};
			return true;
		} catch(const std::logic_error&) { return false; }
	}

	inline void serialize(akd::PValue& dst, const akas::Skin& src) {
		dst["root"].set<int32>(src.root);
		dst["joints"].setArr(); for(auto joint : src.joints) { dst["joints"].asArr().push_back(akd::PValue()); serialize(dst["joints"].asArr().back(), joint); }
		dst["mapping"].setArr(); for(auto mapping : src.mapping) dst["mapping"].asArr().push_back(akd::PValue::from<uint32>(mapping));
	}

}

#endif /* AK_ASSETS_SKIN_HPP_ */
