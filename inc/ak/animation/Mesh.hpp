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
		size_t vertIDs[3];
		size_t normIDs[3];
		size_t uvIDs[3];
	};

	using BoneWeights = std::unordered_map<std::string, fpSingle>;

	class Mesh final {
		private:
			std::vector<akm::Vec3> m_verts;
			std::vector<akm::Vec3> m_norms;
			std::vector<akm::Vec2> m_uvs;
			std::vector<BoneWeights> m_boneWeights;
			std::vector<Face> m_faces;

		public:
			Mesh() {}
			Mesh(const std::vector<akm::Vec3>& verts, const std::vector<akm::Vec3>& norms, const std::vector<akm::Vec2>& uvs, const std::vector<BoneWeights>& boneWeights, const std::vector<Face>& faces) : m_verts(verts), m_norms(norms), m_uvs(uvs), m_boneWeights(boneWeights), m_faces(faces) {}
			Mesh(std::vector<akm::Vec3>&& verts, std::vector<akm::Vec3>&& norms, std::vector<akm::Vec2>&& uvs, std::vector<BoneWeights>&& boneWeights, std::vector<Face>&& faces) : m_verts(verts), m_norms(norms), m_uvs(uvs), m_boneWeights(boneWeights), m_faces(faces) {}

			Mesh(const Mesh& other) : m_verts(other.m_verts), m_norms(other.m_norms), m_uvs(other.m_uvs), m_boneWeights(other.m_boneWeights), m_faces(other.m_faces) {}
			Mesh(Mesh&& other) : m_verts(std::move(other.m_verts)), m_norms(std::move(other.m_norms)), m_uvs(std::move(other.m_uvs)), m_boneWeights(other.m_boneWeights), m_faces(std::move(other.m_faces)) {}

			std::vector<akm::Vec3>& verts() { return m_verts; }
			std::vector<akm::Vec3>& norms() { return m_norms; }
			std::vector<akm::Vec2>& uvs() { return m_uvs; }
			std::vector<BoneWeights>& boneWeights() { return m_boneWeights; }
			std::vector<Face>& faces() { return m_faces; }

			std::vector<fpSingle> buildBuffer(bool verts, bool norms, bool UVs) {
				std::vector<fpSingle> buffer;

				for(auto iter = m_faces.begin(); iter != m_faces.end(); iter++) {
					for(auto id = 0u; id < 3; id++) {
						if (verts) {
							buffer.push_back(m_verts[iter->vertIDs[id]].x);
							buffer.push_back(m_verts[iter->vertIDs[id]].y);
							buffer.push_back(m_verts[iter->vertIDs[id]].z);
						}
						if (norms) {
							buffer.push_back(m_norms[iter->normIDs[id]].x);
							buffer.push_back(m_norms[iter->normIDs[id]].y);
							buffer.push_back(m_norms[iter->normIDs[id]].z);
						}
						if (UVs) {
							buffer.push_back(m_uvs[iter->uvIDs[id]].x);
							buffer.push_back(m_uvs[iter->uvIDs[id]].y);
						}
					}
				}

				return buffer;
			}

			const std::vector<akm::Vec3>& verts() const { return m_verts; }
			const std::vector<akm::Vec3>& norms() const { return m_norms; }
			const std::vector<akm::Vec2>& uvs() const { return m_uvs; }
			const std::vector<BoneWeights>& boneWeights() const { return m_boneWeights; }
			const std::vector<Face>& faces() const { return m_faces; }

			Mesh& operator=(const Mesh& other) { m_verts = other.m_verts; m_norms = other.m_norms; m_uvs = other.m_uvs; m_boneWeights = other.m_boneWeights; m_faces = other.m_faces; return *this; }
			Mesh& operator=(Mesh&& other) { m_verts = std::move(other.m_verts); m_norms = std::move(other.m_norms); m_uvs = std::move(other.m_uvs); m_boneWeights = std::move(other.m_boneWeights); m_faces = std::move(other.m_faces); return *this; }
	};

}

namespace akd {
	inline bool deserialize(aka::Mesh& dest, akd::PValue& src) {

		std::vector<akm::Vec3> verts;
		auto& vertSrc = src["verts"].asArrOrDef();
		verts.reserve(vertSrc.size());
		for(auto iter = vertSrc.begin(); iter != vertSrc.end(); iter++) {
			akm::Vec3 vert;
			akd::deserialize(vert, *iter);
			verts.push_back(vert);
		}

		std::vector<akm::Vec3> norms;
		auto& normSrc = src["norms"].asArrOrDef();
		norms.reserve(normSrc.size());
		for(auto iter = normSrc.begin(); iter != normSrc.end(); iter++) {
			akm::Vec3 norm;
			akd::deserialize(norm, *iter);
			norms.push_back(norm);
		}

		std::vector<akm::Vec2> uvs;
		auto& uvSrc = src["uv"].asArrOrDef();
		uvs.reserve(uvSrc.size());
		for(auto iter = uvSrc.begin(); iter != uvSrc.end(); iter++) {
			akm::Vec2 uv;
			akd::deserialize(uv, *iter);
			uvs.push_back(uv);
		}

		std::vector<aka::BoneWeights> vWeights;
		auto& vWeightSrc = src["bones"].asArrOrDef();
		vWeights.reserve(vWeightSrc.size());
		for(auto vWeightIter = vWeightSrc.begin(); vWeightIter != vWeightSrc.end(); vWeightIter++) {
			aka::BoneWeights weights;
			auto boneWeightSrc = vWeightIter->asObjOrDef();
			for(auto boneIter = boneWeightSrc.begin(); boneIter != boneWeightSrc.end(); boneIter++) {
				weights.insert(std::make_pair(boneIter->first, boneIter->second.as<fpSingle>()));
			}
			vWeights.push_back(weights);
		}

		std::vector<aka::Face> faces;
		auto& facesSrc = src["faces"].asArrOrDef();
		faces.reserve(facesSrc.size());
		for(auto faceIter = facesSrc.begin(); faceIter != facesSrc.end(); faceIter++) {
			aka::Face face;
			face.vertIDs[0] = faceIter->at(0)[0].as<uint32>();
			face.vertIDs[1] = faceIter->at(0)[1].as<uint32>();
			face.vertIDs[2] = faceIter->at(0)[2].as<uint32>();

			face.normIDs[0] = faceIter->at(1)[0].as<uint32>();
			face.normIDs[1] = faceIter->at(1)[1].as<uint32>();
			face.normIDs[2] = faceIter->at(1)[2].as<uint32>();

			face.uvIDs[0] = faceIter->at(2)[0].as<uint32>();
			face.uvIDs[1] = faceIter->at(2)[1].as<uint32>();
			face.uvIDs[2] = faceIter->at(2)[2].as<uint32>();

			faces.push_back(face);
		}

		dest = aka::Mesh(verts, norms, uvs, vWeights, faces);
		return true;
	}
}

#endif
