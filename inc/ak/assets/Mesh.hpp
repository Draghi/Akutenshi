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

#ifndef AK_ASSETS_MESH_HPP_
#define AK_ASSETS_MESH_HPP_

#include <array>
#include <vector>

#include <ak/assets/Skin.hpp>
#include <ak/data/SUID.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/gl/Types.hpp>

namespace akas {
	struct VertexSurfaceData final {
		akm::Vec3 position;
		akm::Vec3 tangent;
		akm::Vec3 bitangent;
		akm::Vec3 normal;
	};

	struct VertexWeightData final {
		std::array<uint32, 4> bones;
		akm::Vec4 weights;
	};

	struct Primitive final {
		akr::gl::DrawType                     drawType;
		std::vector<uint16>                   indexData;
		std::vector<VertexSurfaceData>        surfaceData;
		std::array<std::vector<akm::Vec2>, 4> texCoordData;
		std::vector<akm::Vec4>                colourData;
		std::vector<VertexWeightData>         skinningData;
		akd::SUID                             materialAssetID;
	};

	struct Mesh final {
		std::vector<Primitive> primitives;
		Skin skin;
	};
}

#endif /* AKRES_ANIM_MESH_HPP_ */
