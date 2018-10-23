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

#ifndef AK_ASSETS_GLTF_ACCESSOR_HPP_
#define AK_ASSETS_GLTF_ACCESSOR_HPP_

#include <AkAsset/gltf/Types.hpp>
#include <AkCommon/PrimitiveTypes.hpp>
#include <AkEngine/data/PValue.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace aka {
	namespace gltf {

		enum class ComponnentType : uint32 {
			Byte   = 5120,
			UByte  = 5121,
			Short  = 5122,
			UShort = 5123,
			UInt   = 5125,
			Float  = 5126,
		};

		enum class AccessorType {
			Scalar,
			Vec2,
			Vec3,
			Vec4,
			Mat2,
			Mat3,
			Mat4
		};

		struct AccessorSparseIndicies final {
			gltfID bufferViewID;
			int32 byteOffset;
			ComponnentType componentType;
		};

		struct AccessorSparseValues final {
			gltfID bufferViewID;
			int32 byteOffset;
		};

		struct AccessorSparse final {
			int32 count;
			AccessorSparseIndicies indidies;
			AccessorSparseValues values;
		};

		struct Accessor final {
			std::string name;
			gltfID bufferViewID;
			int32 byteOffset;
			ComponnentType componentType;
			bool normalized;
			int32 count;
			AccessorType type;
			std::vector<fpSingle> max;
			std::vector<fpSingle> min;
			//AccessorSparse sparseAccessor;
		};

		inline AccessorType extractAccessorType(const akd::PValue& accessorTypeData) {
			std::string val = accessorTypeData.getStr();
			if (val == "SCALAR") return AccessorType::Scalar;
			if (val == "VEC2") return AccessorType::Vec2;
			if (val == "VEC3") return AccessorType::Vec3;
			if (val == "VEC4") return AccessorType::Vec4;
			if (val == "MAT2") return AccessorType::Mat2;
			if (val == "MAT3") return AccessorType::Mat3;
			if (val == "MAT4") return AccessorType::Mat4;
			throw std::runtime_error("Unsupported accessor type: " + val);
		}

		inline AccessorSparse extractSparseAccessor(const akd::PValue& sparseAccessorData) {
			if (sparseAccessorData.isNull()) return AccessorSparse{0, {-1, 0, ComponnentType::Byte}, {-1, 0}};
			return AccessorSparse{
				sparseAccessorData["count"].as<int32>(),
				{
					sparseAccessorData["indices"]["bufferView"].as<gltfID>(),
					sparseAccessorData["indices"]["byteOffset"].as<int32>(),
					static_cast<ComponnentType>(sparseAccessorData["indices"]["componentType"].as<uint32>()),
				},
				{
					sparseAccessorData["indices"]["bufferView"].as<gltfID>(),
					sparseAccessorData["indices"]["byteOffset"].as<int32>(),
				}
			};
		}

		inline Accessor extractAccessor(const akd::PValue& accessorData) {
			static constexpr auto extractArray = [](const akd::PValue& val){
				std::vector<fpSingle> result; result.reserve(val.isArr() ? val.size() : 0);
				for(auto& v : val.getArr()) result.push_back(v.as<fpSingle>());
				return result;
			};

			if (accessorData.exists("sparse")) throw std::runtime_error("Sparse accessor not supported.");

			return Accessor{
				accessorData.atOrDef("name").getStrOrDef(),
				accessorData["bufferView"].as<gltfID>(),
				accessorData.atOrDef("byteOffset").asOrDef<int32>(0),
				static_cast<ComponnentType>(accessorData["componentType"].as<uint32>()),
				accessorData.atOrDef("normalized").asOrDef<bool>(false),
				accessorData.atOrDef("count").asOrDef<int32>(0),
				extractAccessorType(accessorData["type"]),
				extractArray(accessorData.atOrDef("max")),
				extractArray(accessorData.atOrDef("min"))//,
				//extractSparseAccessor(accessorData.atOrDef("sparse"))
			};
		}


	}
}



#endif /* AKRES_GLTF_ACCESSOR_HPP_ */
