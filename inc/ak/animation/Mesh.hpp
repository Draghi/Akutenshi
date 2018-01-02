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

#include <ak/data/PValue.hpp>
#include <ak/math/Serialize.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <stddef.h>
#include <algorithm>
#include <deque>
#include <initializer_list>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace aka {

	struct Face {
		akSize vertIDs[3];
		akSize normIDs[3];
		akSize texCoordIDs[3];
	};

	using BoneWeights = std::unordered_map<std::string, fpSingle>;

	class Mesh final {
		private:
			std::vector<akm::Vec3> m_vertices;
			std::vector<akm::Vec3> m_normals;
			std::vector<akm::Vec3> m_tangents;
			std::vector<akm::Vec3> m_bitangents;
			std::vector<akm::Vec2> m_texCoords;
			std::vector<BoneWeights> m_boneWeights;
			std::vector<Face> m_faces;

		public:
			Mesh() {}

			Mesh(
				const std::vector<akm::Vec3>& vertices,
				const std::vector<akm::Vec3>& normals,
				const std::vector<akm::Vec3>& tangents,
				const std::vector<akm::Vec3>& bitangents,
				const std::vector<akm::Vec2>& texCoords,
				const std::vector<BoneWeights>& boneWeights,
				const std::vector<Face>& faces)
			: m_vertices(vertices), m_normals(normals), m_tangents(tangents), m_bitangents(bitangents), m_texCoords(texCoords), m_boneWeights(boneWeights), m_faces(faces) {}

			Mesh(
				std::vector<akm::Vec3>&& vertices,
				std::vector<akm::Vec3>&& normals,
				std::vector<akm::Vec3>&& tangents,
				std::vector<akm::Vec3>&& bitangents,
				std::vector<akm::Vec2>&& texCoords,
				std::vector<BoneWeights>&& boneWeights,
				std::vector<Face>&& faces)
			: m_vertices(vertices), m_normals(normals), m_tangents(tangents), m_bitangents(bitangents), m_texCoords(texCoords), m_boneWeights(boneWeights), m_faces(faces) {}

			Mesh(const Mesh& other) : m_vertices(other.m_vertices), m_normals(other.m_normals), m_tangents(other.m_tangents), m_bitangents(other.m_bitangents), m_texCoords(other.m_texCoords), m_boneWeights(other.m_boneWeights), m_faces(other.m_faces) {}
			Mesh(Mesh&& other) : m_vertices(std::move(other.m_vertices)), m_normals(std::move(other.m_normals)), m_tangents(std::move(other.m_tangents)), m_bitangents(std::move(other.m_bitangents)), m_texCoords(std::move(other.m_texCoords)), m_boneWeights(other.m_boneWeights), m_faces(std::move(other.m_faces)) {}

			std::vector<akm::Vec3>& vertices() { return m_vertices; }
			std::vector<akm::Vec3>& normals() { return m_normals; }
			std::vector<akm::Vec3>& tangents() { return m_tangents; }
			std::vector<akm::Vec3>& bitangents() { return m_bitangents; }
			std::vector<akm::Vec2>& texCoords() { return m_texCoords; }
			std::vector<BoneWeights>& boneWeights() { return m_boneWeights; }
			std::vector<Face>& faces() { return m_faces; }

			std::vector<fpSingle> buildBuffer(bool vertices, bool normals, bool tangents, bool bitangents, bool UVs) {
				std::vector<fpSingle> buffer;

				for(auto iter = m_faces.begin(); iter != m_faces.end(); iter++) {
					for(auto id = 0u; id < 3; id++) {
						if (vertices) {
							buffer.push_back(m_vertices[iter->vertIDs[id]].x);
							buffer.push_back(m_vertices[iter->vertIDs[id]].y);
							buffer.push_back(m_vertices[iter->vertIDs[id]].z);
						}
						if (normals) {
							buffer.push_back(m_normals[iter->normIDs[id]].x);
							buffer.push_back(m_normals[iter->normIDs[id]].y);
							buffer.push_back(m_normals[iter->normIDs[id]].z);
						}
						if (tangents) {
							buffer.push_back(m_tangents[iter->normIDs[id]].x);
							buffer.push_back(m_tangents[iter->normIDs[id]].y);
							buffer.push_back(m_tangents[iter->normIDs[id]].z);
						}
						if (bitangents) {
							buffer.push_back(m_bitangents[iter->normIDs[id]].x);
							buffer.push_back(m_bitangents[iter->normIDs[id]].y);
							buffer.push_back(m_bitangents[iter->normIDs[id]].z);
						}
						if (UVs) {
							buffer.push_back(m_texCoords[iter->texCoordIDs[id]].x);
							buffer.push_back(m_texCoords[iter->texCoordIDs[id]].y);
						}
					}
				}

				return buffer;
			}

			const std::vector<akm::Vec3>& vertices() const { return m_vertices; }
			const std::vector<akm::Vec3>& normals() const { return m_normals; }
			const std::vector<akm::Vec3>& tangents() const { return m_tangents; }
			const std::vector<akm::Vec3>& bitangents() const { return m_bitangents; }
			const std::vector<akm::Vec2>& texCoords() const { return m_texCoords; }
			const std::vector<BoneWeights>& boneWeights() const { return m_boneWeights; }
			const std::vector<Face>& faces() const { return m_faces; }

			Mesh& operator=(const Mesh& other) {
				m_vertices = other.m_vertices;
				m_normals = other.m_normals;
				m_tangents = other.m_tangents;
				m_bitangents = other.m_bitangents;
				m_texCoords = other.m_texCoords;
				m_boneWeights = other.m_boneWeights;
				m_faces = other.m_faces;
				return *this;
			}

			Mesh& operator=(Mesh&& other) {
				m_vertices = std::move(other.m_vertices);
				m_normals = std::move(other.m_normals);
				m_tangents = std::move(other.m_tangents);
				m_bitangents = std::move(other.m_bitangents);
				m_texCoords = std::move(other.m_texCoords);
				m_boneWeights = std::move(other.m_boneWeights);
				m_faces = std::move(other.m_faces);
				return *this;
			}
	};

}

namespace akd {
	inline bool deserialize(aka::Mesh& dest, akd::PValue& src) {
		dest = aka::Mesh();

		{
			std::vector<akm::Vec3> vertices;
			auto& verticesSrc = src["vertices"].asArr();
			vertices.reserve(verticesSrc.size());
			for(auto iter = verticesSrc.begin(); iter != verticesSrc.end(); iter++) {
				akm::Vec3 vertex;
				akd::deserialize(vertex, *iter);
				vertices.push_back(vertex);
			}
			dest.vertices() = vertices;
		}

		{
			std::vector<akm::Vec3> normals;
			auto& normalsSrc = src["normals"].asArr();
			normals.reserve(normalsSrc.size());
			for(auto iter = normalsSrc.begin(); iter != normalsSrc.end(); iter++) {
				akm::Vec3 normal;
				akd::deserialize(normal, *iter);
				normals.push_back(normal);
			}
			dest.normals() = normals;
		}

		{
			std::vector<akm::Vec3> tangents;
			auto& tangentSrc = src["tangents"].asArr();
			tangents.reserve(tangentSrc.size());
			for(auto iter = tangentSrc.begin(); iter != tangentSrc.end(); iter++) {
				akm::Vec3 tangent;
				akd::deserialize(tangent, *iter);
				tangents.push_back(tangent);
			}
			dest.tangents() = tangents;
		}

		{
			std::vector<akm::Vec3> bitangents;
			auto& bitangentSrc = src["bitangents"].asArr();
			bitangents.reserve(bitangentSrc.size());
			for(auto iter = bitangentSrc.begin(); iter != bitangentSrc.end(); iter++) {
				akm::Vec3 bitangent;
				akd::deserialize(bitangent, *iter);
				bitangents.push_back(bitangent);
			}
			dest.bitangents() = bitangents;
		}

		{
			std::vector<akm::Vec2> texCoords;
			auto& texCoordSrc = src["texCoords"].asArr();
			texCoords.reserve(texCoordSrc.size());
			for(auto iter = texCoordSrc.begin(); iter != texCoordSrc.end(); iter++) {
				akm::Vec2 texCoord;
				akd::deserialize(texCoord, *iter);
				texCoords.push_back(texCoord);
			}
			dest.texCoords() = texCoords;
		}

		{
			std::vector<aka::BoneWeights> vWeights;
			auto& vWeightSrc = src["vertexWeights"].asArr();
			vWeights.reserve(vWeightSrc.size());
			for(auto vWeightIter = vWeightSrc.begin(); vWeightIter != vWeightSrc.end(); vWeightIter++) {
				aka::BoneWeights weights;
				auto boneWeightSrc = vWeightIter->asObj();
				for(auto boneIter = boneWeightSrc.begin(); boneIter != boneWeightSrc.end(); boneIter++) {
					weights.insert(std::make_pair(boneIter->first, boneIter->second.as<fpSingle>()));
				}
				vWeights.push_back(weights);
			}
			dest.boneWeights() = vWeights;
		}

		if ((dest.normals().size() != dest.tangents().size()) || (dest.tangents().size() != dest.bitangents().size())) {
			throw std::logic_error("Normal/Tangent/Bitangent size mismatch");
		}

		{
			std::vector<aka::Face> faces;
			auto& facesSrc = src["faces"].asArr();
			faces.reserve(facesSrc.size());
			for(auto faceIter = facesSrc.begin(); faceIter != facesSrc.end(); faceIter++) {
				aka::Face face;
				face.vertIDs[0] = faceIter->at(0)[0].as<uint32>();
				face.vertIDs[1] = faceIter->at(0)[1].as<uint32>();
				face.vertIDs[2] = faceIter->at(0)[2].as<uint32>();

				face.normIDs[0] = faceIter->at(1)[0].as<uint32>();
				face.normIDs[1] = faceIter->at(1)[1].as<uint32>();
				face.normIDs[2] = faceIter->at(1)[2].as<uint32>();

				face.texCoordIDs[0] = faceIter->at(2)[0].as<uint32>();
				face.texCoordIDs[1] = faceIter->at(2)[1].as<uint32>();
				face.texCoordIDs[2] = faceIter->at(2)[2].as<uint32>();

				faces.push_back(face);
			}
			dest.faces() = faces;
		}

		return true;
	}
}

#endif
