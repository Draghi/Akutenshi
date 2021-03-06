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
#ifndef AK_ASSETS_SHADERPROGRAM_HPP_
#define AK_ASSETS_SHADERPROGRAM_HPP_

#include <akengine/data/PValue.hpp>
#include <akengine/data/SmartClass.hpp>
#include <akengine/data/SUID.hpp>
#include <akrender/gl/Types.hpp>
#include <unordered_map>

namespace aka {
	struct ShaderStageEntry final {
		akd::SUID identifier;
	};

	struct ShaderProgram final {
		std::unordered_map<akr::gl::StageType, ShaderStageEntry> stages;
	};
}

AK_SMART_CLASS(aka::ShaderStageEntry,
	FIELD, identifier
)

AK_SMART_CLASS(aka::ShaderProgram,
	FIELD, stages
)

#endif /* AK_ASSETS_SHADERPROGRAM_HPP_ */
