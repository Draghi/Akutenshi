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
#include <cstring>
#include <deque>
#include <stdexcept>
#include <vector>

#include <ak/assets/Skin.hpp>
#include <ak/data/PValue.hpp>
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

namespace akd {

	inline void serialize(akd::PValue& dst, const akas::Primitive& src) {
		dst["drawType"].set<uint8>(static_cast<uint8>(src.drawType));
		dst["indexData"]   .setBin(   src.indexData.data(),    src.indexData.size()*sizeof(uint16));
		dst["surfaceData"] .setBin( src.surfaceData.data(),  src.surfaceData.size()*sizeof(uint16));
		dst["texCoordData"].setBin(src.texCoordData.data(), src.texCoordData.size()*sizeof(uint16));
		dst["colourData"]  .setBin(  src.colourData.data(),   src.colourData.size()*sizeof(uint16));
		dst["skinningData"].setBin(src.skinningData.data(), src.skinningData.size()*sizeof(uint16));
		serialize(dst["materialAssetID"], src.materialAssetID);
	}

	inline bool deserialize(akas::Primitive& dst, const akd::PValue& src) {
		try {
			auto& indexDataBin = src["indexData"].asBin();
			std::vector<uint16> indexData; indexData.resize(indexDataBin.size()/sizeof(uint16));
			std::memcpy(indexData.data(), indexDataBin.data(), indexDataBin.size());

			auto& surfaceDataBin = src["surfaceData"].asBin();
			std::vector<akas::VertexSurfaceData> surfaceData; surfaceData.resize(surfaceDataBin.size()/sizeof(akas::VertexSurfaceData));
			std::memcpy(surfaceData.data(), surfaceDataBin.data(), surfaceDataBin.size());

			auto& texCoordDataBin = src["texCoordData"].asBin();
			std::array<std::vector<akm::Vec2>, 4> texCoordData;
			std::memcpy(texCoordData.data(), texCoordDataBin.data(), texCoordDataBin.size());

			auto& colourDataBin = src["colourData"].asBin();
			std::vector<akm::Vec4> colourData; surfaceData.resize(surfaceDataBin.size()/sizeof(akm::Vec4));
			std::memcpy(colourData.data(), colourDataBin.data(), colourDataBin.size());

			auto& skinningDataBin = src["skinningData"].asBin();
			std::vector<akas::VertexWeightData> skinningData; skinningData.resize(skinningDataBin.size()/sizeof(akas::VertexWeightData));
			std::memcpy(skinningData.data(), skinningDataBin.data(), surfaceDataBin.size());

			dst = akas::Primitive{
				static_cast<akr::gl::DrawType>(src["drawType"].as<uint8>()),
				indexData,
				surfaceData,
				texCoordData,
				colourData,
				skinningData,
				deserialize<akd::SUID>(src["materialAssetID"])
			};
			return true;
		} catch(const std::logic_error&) { return false; }
	}

	inline void serialize(akd::PValue& dst, const akas::Mesh& src) {
		auto& primitives = dst["primitives"].setArr();
		for(const auto& primitive : src.primitives) { primitives.asArr().push_back(akd::PValue()); serialize(primitives.asArr().back(), primitive); };
		serialize(dst["skin"], src.skin);
	}
}



#endif /* AKRES_ANIM_MESH_HPP_ */
