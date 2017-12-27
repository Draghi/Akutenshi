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

#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <stddef.h>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace aka {

	struct Face {
		size_t vertIDs[3];
		size_t normIDs[3];
		size_t uvIDs[3];
		std::unordered_map<std::string, fpDouble> boneWeights;
	};

	class Mesh final {
		private:
			std::vector<akm::Vec3> m_verts;
			std::vector<akm::Vec3> m_norms;
			std::vector<akm::Vec2> m_uvs;
			std::vector<Face> m_faces;

		public:
			Mesh() {}
			Mesh(const std::vector<akm::Vec3>& verts, const std::vector<akm::Vec3>& norms, const std::vector<akm::Vec2>& uvs, const std::vector<Face>& faces) : m_verts(verts), m_norms(norms), m_uvs(uvs), m_faces(faces) {}
			Mesh(std::vector<akm::Vec3>&& verts, std::vector<akm::Vec3>&& norms, std::vector<akm::Vec2>&& uvs, std::vector<Face>&& faces) : m_verts(verts), m_norms(norms), m_uvs(uvs), m_faces(faces) {}

			Mesh(const Mesh& other) : m_verts(other.m_verts), m_norms(other.m_norms), m_uvs(other.m_uvs), m_faces(other.m_faces) {}
			Mesh(Mesh&& other) : m_verts(std::move(other.m_verts)), m_norms(std::move(other.m_norms)), m_uvs(std::move(other.m_uvs)), m_faces(std::move(other.m_faces)) {}

			std::vector<akm::Vec3>& verts() { return m_verts; }
			std::vector<akm::Vec3>& norms() { return m_norms; }
			std::vector<akm::Vec2>& uvs() { return m_uvs; }
			std::vector<Face>& faces() { return m_faces; }

			const std::vector<akm::Vec3>& verts() const { return m_verts; }
			const std::vector<akm::Vec3>& norms() const { return m_norms; }
			const std::vector<akm::Vec2>& uvs() const { return m_uvs; }
			const std::vector<Face>& faces() const { return m_faces; }

			Mesh& operator=(const Mesh& other) { m_verts = other.m_verts; m_norms = other.m_norms; m_uvs = other.m_uvs; m_faces = other.m_faces; return *this; }
			Mesh& operator=(Mesh&& other) { m_verts = std::move(other.m_verts); m_norms = std::move(other.m_norms); m_uvs = std::move(other.m_uvs); m_faces = std::move(other.m_faces); return *this; }
	};

}

namespace akd {
	bool deserialize(aka::Mesh& dest);
}

#endif
