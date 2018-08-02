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

#include <string>
#include <vector>

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

#endif /* AK_ASSETS_SKIN_HPP_ */
