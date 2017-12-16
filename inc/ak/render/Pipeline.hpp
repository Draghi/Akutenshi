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

#ifndef AK_RENDER_PIPELINE_HPP_
#define AK_RENDER_PIPELINE_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/render/Types.hpp>
#include <algorithm>
#include <array>
#include <string>

namespace akr {

	class PipelineStage final {
		PipelineStage(const PipelineStage&) = delete;
		PipelineStage& operator=(const PipelineStage&) = delete;

		private:
			uint32 m_id;
			StageType m_type;
			std::string m_src;

		public:
			PipelineStage(StageType stageType);
			PipelineStage(PipelineStage&& other) : m_id(other.m_id), m_type(other.m_type), m_src(std::move(other.m_src)) { other.m_id = 0; }
			~PipelineStage();

			bool attach(const std::string& src);
			bool compile();

			uint32 id() const { return m_id; }
			StageType type() const { return m_type; }
			std::string source() const { return m_src; }
			bool isCompiled() const;
	};

	class Pipeline final {
		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		private:
			uint32 m_id;
			std::array<uint32, 3> m_attachedShaders;
			bool m_isLinked;

		public:
			Pipeline();
			Pipeline(Pipeline&& other) : m_id(other.m_id), m_attachedShaders(std::move(other.m_attachedShaders)), m_isLinked(other.m_isLinked) { other.m_id = 0; }
			~Pipeline();

			bool attach(const PipelineStage& stage);
			bool link();

			uint32 id() const { return m_id; }
			bool isLinked() const { return m_isLinked; }
	};

	bool resetActivePipeline();
	bool setActivePipeline(const Pipeline& pipeline);

}

#endif
