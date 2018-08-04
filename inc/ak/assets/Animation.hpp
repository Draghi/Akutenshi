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
#ifndef AK_ASSETS_ANIMATION_HPP_
#define AK_ASSETS_ANIMATION_HPP_

#include <map>
#include <string>
#include <vector>

#include <ak/math/Serialize.hpp>
#include <ak/data/SmartClass.hpp>
#include <ak/data/SmartEnum.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>

namespace akas {

	AK_SMART_TENUM(Interpolation, uint8,
		Step,
		Linear,
		Cubic
	);

	struct AnimPosFrame final {
		uint64 boneID;
		Interpolation interpolation;
		akm::Vec3 position;
	};

	struct AnimRotFrame final {
		uint64 boneID;
		Interpolation interpolation;
		akm::Quat rotation;
	};

	struct AnimSclFrame final {
		uint64 boneID;
		Interpolation interpolation;
		akm::Vec3 scale;
	};

	struct Animation final {
		std::vector<std::string> boneNames;
		std::map<fpSingle, std::vector<AnimPosFrame>> positionChannel;
		std::map<fpSingle, std::vector<AnimRotFrame>> rotationChannel;
		std::map<fpSingle, std::vector<AnimSclFrame>> scaleChannel;
	};

}

AK_SMART_ENUM_SERIALIZE(akas, Interpolation)

AK_SMART_CLASS(akas::AnimPosFrame,
	FIELD, boneID,
	FIELD, interpolation,
	FIELD, position
)

AK_SMART_CLASS(akas::AnimRotFrame,
	FIELD, boneID,
	FIELD, interpolation,
	FIELD, rotation
)

AK_SMART_CLASS(akas::AnimSclFrame,
	FIELD, boneID,
	FIELD, interpolation,
	FIELD, scale
)

AK_SMART_CLASS(akas::Animation,
	FIELD, boneNames,
	FIELD, positionChannel,
	FIELD, rotationChannel,
	FIELD, scaleChannel
)

#endif /* AK_ASSETS_ANIMATION_HPP_ */
