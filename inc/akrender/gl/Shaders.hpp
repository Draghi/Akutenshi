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

#ifndef AK_RENDER_GL_SHADERS_HPP_
#define AK_RENDER_GL_SHADERS_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akmath/Matrix.hpp>
#include <akmath/Vector.hpp>
#include <akrender/gl/Types.hpp>
#include <array>
#include <string>

namespace akr {
	namespace gl {

		class ShaderStage final {
			ShaderStage(const ShaderStage&) = delete;
			ShaderStage& operator=(const ShaderStage&) = delete;

			private:
				uint32 m_id;
				StageType m_type;
				std::string m_compileLog;

			public:
				ShaderStage();
				ShaderStage(StageType stageType);
				ShaderStage(ShaderStage&& other);
				~ShaderStage();

				bool attach(const std::string& src);
				bool compile();

				bool isCompiled() const;
				const std::string& compileLog() const;
				void clearCompileLog();

				bool isValid() const;

				uint32 id() const;
				StageType type() const;

				ShaderStage& operator=(ShaderStage&& other);
		};

		class ShaderProgram final {
			ShaderProgram(const ShaderProgram&) = delete;
			ShaderProgram& operator=(const ShaderProgram&) = delete;

			private:
				uint32 m_id;
				std::array<uint32, 3> m_attachedShaders;
				std::string m_linkLog;

			public:
				ShaderProgram();
				ShaderProgram(ShaderProgram&& other);
				~ShaderProgram();

				bool attach(const ShaderStage& stage);

				bool detach(StageType stageType);
				void detachAll();

				bool link();

				bool isLinked() const;
				const std::string& linkLog() const;
				void clearLinkLog();

				uint32 id() const;
				bool isValid() const;

				ShaderProgram& operator=(ShaderProgram&& other);

				// ////////////// //
				// // Uniforms // //
				// ////////////// //

				void setUniform(uint32 index, uint32 x);
				void setUniform(uint32 index, uint32 x, uint32 y);
				void setUniform(uint32 index, uint32 x, uint32 y, uint32 z);
				void setUniform(uint32 index, uint32 x, uint32 y, uint32 z, uint32 w);

				void setUniform(uint32 index, int32 x);
				void setUniform(uint32 index, int32 x, int32 y);
				void setUniform(uint32 index, int32 x, int32 y, int32 z);
				void setUniform(uint32 index, int32 x, int32 y, int32 z, int32 w);

				void setUniform(uint32 index, fpSingle x);
				void setUniform(uint32 index, fpSingle x, fpSingle y);
				void setUniform(uint32 index, fpSingle x, fpSingle y, fpSingle z);
				void setUniform(uint32 index, fpSingle x, fpSingle y, fpSingle z, fpSingle w);

				void setUniform(uint32 index, akm::Vec2 vec);
				void setUniform(uint32 index, akm::Vec3 vec);
				void setUniform(uint32 index, akm::Vec4 vec);

				void setUniform(uint32 index, akm::Mat2 matrix);
				void setUniform(uint32 index, akm::Mat3 matrix);
				void setUniform(uint32 index, akm::Mat4 matrix);
		};

		void bindShaderProgram(const ShaderProgram& pipeline);
		void unbindShaderProgram();
	}
}

#endif
