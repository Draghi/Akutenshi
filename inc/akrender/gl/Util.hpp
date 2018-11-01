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

#ifndef AK_RENDER_GL_UTIL_HPP_
#define AK_RENDER_GL_UTIL_HPP_

#include <akcommon/String.hpp>
#include <akengine/filesystem/CFile.hpp>
#include <akrender/gl/Shaders.hpp>
#include <akrender/gl/Types.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace akr {
	namespace gl {
		namespace util {
			inline ShaderProgram buildShaderProgram(const std::vector<std::pair<StageType, std::string>>& stages) {
				ShaderProgram program;

				for(auto& stageInfo : stages) {
					auto shaderFile = akfs::CFile(stageInfo.second, akfs::OpenFlags::In);
					if (!shaderFile) throw std::runtime_error(akc::buildString("buildShaderProgram: Could not open file: ", stageInfo.second));

					std::string source;
					if (!shaderFile.readAllLines(source)) throw std::runtime_error(akc::buildString("buildShaderProgram: Could not read data from file: ", stageInfo.second));

					ShaderStage stage(stageInfo.first);
					if (!stage.attach(source)) throw std::runtime_error(akc::buildString("buildShaderProgram: Could not attach source to shader, see log for more information."));
					if (!stage.compile()) throw std::runtime_error(akc::buildString("buildShaderProgram: Could not compile shader. Error log:\n", stage.compileLog()));

					if (!program.attach(stage)) throw std::runtime_error(akc::buildString("buildShaderProgram: Could not attach shader to program."));
				}

				if (!program.link()) throw std::runtime_error(akc::buildString("buildShaderProgram: Failed to link shader. Error log:\n", program.linkLog()));
				return program;
			}
		}
	}
}

#endif /* AK_RENDER_GL_UTIL_HPP_ */
