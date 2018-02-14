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

#ifndef AK_ANIMATION_MESH_HPP_
#define AK_ANIMATION_MESH_HPP_

#include <ak/animation/Fwd.hpp>
#include <ak/animation/Type.hpp>
#include <ak/data/PValue.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <cstring>
#include <deque>
#include <utility>
#include <vector>

namespace aka {
	inline Mesh::Mesh() = default;
	inline Mesh::Mesh(std::vector<VertexData>&& vertexData, std::vector<BoneWeights>&& boneWeights, std::vector<IndexData>&& indexData) : m_vertexData(vertexData), m_boneWeights(boneWeights), m_indexData(indexData) {}
	inline Mesh::Mesh(const std::vector<VertexData>& vertexData, const std::vector<BoneWeights>& boneWeights, const std::vector<IndexData>& indexData) : m_vertexData(vertexData), m_boneWeights(boneWeights), m_indexData(indexData) {}

	inline std::vector<VertexData>&  Mesh::vertexData()  { return m_vertexData; }
	inline std::vector<BoneWeights>& Mesh::boneWeights() { return m_boneWeights; }
	inline std::vector<IndexData>&   Mesh::indexData()   { return m_indexData; }

	inline const std::vector<VertexData>&  Mesh::vertexData()  const { return m_vertexData; }
	inline const std::vector<BoneWeights>& Mesh::boneWeights() const { return m_boneWeights; }
	inline const std::vector<IndexData>&   Mesh::indexData()   const { return m_indexData; }
}

#endif
