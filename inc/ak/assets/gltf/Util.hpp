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

#ifndef AK_ASSETS_GLTF_UTIL_HPP_
#define AK_ASSETS_GLTF_UTIL_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/assets/gltf/Accessor.hpp>
#include <ak/assets/gltf/Asset.hpp>
#include <ak/assets/gltf/Buffer.hpp>
#include <ak/assets/gltf/BufferView.hpp>
#include <ak/assets/gltf/Types.hpp>
#include <limits>
#include <vector>

namespace akas {
	namespace gltf {
		namespace internal {
			template<typename return_t> return_t extractValue(const Buffer& buffer, ComponnentType componentType, int32& byteOffset, bool normalized) {
				switch(componentType) {
					case ComponnentType::Byte : {
						int8 val = *reinterpret_cast<const int8*>(buffer.dataURI.data() + byteOffset);
						byteOffset += 1;
						if (normalized) return static_cast<return_t>(static_cast<fpDouble>(val - std::numeric_limits<int8>::min())/(static_cast<fpDouble>(std::numeric_limits<int8>::max()) - std::numeric_limits<int8>::min()));
						else return static_cast<return_t>(val);
					}

					case ComponnentType::UByte: {
						uint8 val = *reinterpret_cast<const uint8*>(buffer.dataURI.data() + byteOffset);
						byteOffset += 1;
						if (normalized) return static_cast<return_t>(static_cast<fpDouble>(val - std::numeric_limits<uint8>::min())/(static_cast<fpDouble>(std::numeric_limits<uint8>::max()) - std::numeric_limits<uint8>::min()));
						else return static_cast<return_t>(val);
					}

					case ComponnentType::Short: {
						int16 val = *reinterpret_cast<const int16*>(buffer.dataURI.data() + byteOffset);
						byteOffset += 2;
						if (normalized) return static_cast<return_t>(static_cast<fpDouble>(val - std::numeric_limits<int16>::min())/(static_cast<fpDouble>(std::numeric_limits<int16>::max()) - std::numeric_limits<int16>::min()));
						else return static_cast<return_t>(val);
					}

					case ComponnentType::UShort: {
						uint16 val = *reinterpret_cast<const uint16*>(buffer.dataURI.data() + byteOffset);
						byteOffset += 2;
						if (normalized) return static_cast<return_t>(static_cast<fpDouble>(val - std::numeric_limits<uint16>::min())/(static_cast<fpDouble>(std::numeric_limits<uint16>::max()) - std::numeric_limits<uint16>::min()));
						else return static_cast<return_t>(val);
					}

					case ComponnentType::UInt: {
						uint32 val = *reinterpret_cast<const uint32*>(buffer.dataURI.data() + byteOffset);
						byteOffset += 4;
						if (normalized) return static_cast<return_t>(static_cast<fpDouble>(val - std::numeric_limits<uint32>::min())/(static_cast<fpDouble>(std::numeric_limits<uint32>::max()) - std::numeric_limits<uint32>::min()));
						else return static_cast<return_t>(val);
					}

					case ComponnentType::Float: {
						fpSingle val = *reinterpret_cast<const fpSingle*>(buffer.dataURI.data() + byteOffset);
						byteOffset += 4;
						return static_cast<return_t>(val);
					}
				}
			}
		}

		template<typename return_t> std::vector<return_t> extractAccessorData(const Asset& asset, gltfID accessorID) {
			if (accessorID < 0) return std::vector<return_t>();

			const auto& accessor   = asset.accessors[accessorID];
			const auto& bufferView = asset.bufferViews[accessor.bufferViewID];
			const auto& buffer     = asset.buffers[bufferView.bufferID];

			int32 byteOffset = bufferView.byteOffset + accessor.byteOffset;

			std::vector<return_t> result;
			result.reserve(accessor.count);

			switch(accessor.type) {
				case AccessorType::Scalar: {
					if constexpr (!(std::is_same<return_t, akm::Vec2>::value || std::is_same<return_t, akm::Vec3>::value || std::is_same<return_t, akm::Vec4>::value
					             || std::is_same<return_t, akm::Mat2>::value || std::is_same<return_t, akm::Mat3>::value || std::is_same<return_t, akm::Mat4>::value)) {
						for(int32 i = 0; i < accessor.count; i++) {
							result.push_back(internal::extractValue<return_t>(buffer, accessor.componentType, byteOffset, accessor.normalized));
							byteOffset += bufferView.byteStride;
						}
					} else throw std::runtime_error("Accessor for Scalars");
				} break;

				case AccessorType::Vec2: {
					if constexpr (std::is_same<return_t, akm::Vec2>::value) {
						for(int32 i = 0; i < accessor.count; i++) {
							result.push_back({
								internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized),
								internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized)
							});
							byteOffset += bufferView.byteStride;
						}
					} else throw std::runtime_error("Accessor for Vec2s");
				} break;

				case AccessorType::Vec3: {
					if constexpr (std::is_same<return_t, akm::Vec3>::value) {
						for(int32 i = 0; i < accessor.count; i++) {
							result.push_back({
								internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized),
								internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized),
								internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized)
							});
							byteOffset += bufferView.byteStride;
						}
					} else throw std::runtime_error("Accessor for Vec3s");
				} break;

				case AccessorType::Vec4: {
					if constexpr (std::is_same<return_t, akm::Vec4>::value) {
						for(int32 i = 0; i < accessor.count; i++) {
							result.push_back({
								internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized),
								internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized),
								internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized),
								internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized)
							});
							byteOffset += bufferView.byteStride;
						}
					} else throw std::runtime_error("Accessor for Vec4s");
				} break;

				case AccessorType::Mat2: {
					if constexpr (std::is_same<return_t, akm::Mat2>::value) {
						for(int32 i = 0; i < accessor.count; i++) {
							int32 lastOffset, offsetDelta;

							lastOffset = byteOffset;
							fpSingle m00 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m10 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							offsetDelta = byteOffset - lastOffset;
							byteOffset = lastOffset + 4*(offsetDelta/4 + !!(offsetDelta % 4));

							lastOffset = byteOffset;
							fpSingle m01 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m11 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							offsetDelta = byteOffset - lastOffset;
							byteOffset = lastOffset + 4*(offsetDelta/4 + !!(offsetDelta % 4));

							result.push_back({
								m00, m10,
								m01, m11
							});

							byteOffset += bufferView.byteStride;
						}
					} else throw std::runtime_error("Accessor for Mat2s");
				} break;

				case AccessorType::Mat3: {
					if constexpr (std::is_same<return_t, akm::Mat3>::value) {
						for(int32 i = 0; i < accessor.count; i++) {
							int32 lastOffset, offsetDelta;

							lastOffset = byteOffset;
							fpSingle m00 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m10 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m20 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							offsetDelta = byteOffset - lastOffset;
							byteOffset = lastOffset + 4*(offsetDelta/4 + !!(offsetDelta % 4));

							lastOffset = byteOffset;
							fpSingle m01 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m11 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m21 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							offsetDelta = byteOffset - lastOffset;
							byteOffset = lastOffset + 4*(offsetDelta/4 + !!(offsetDelta % 4));

							lastOffset = byteOffset;
							fpSingle m02 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m12 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m22 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							offsetDelta = byteOffset - lastOffset;
							byteOffset = lastOffset + 4*(offsetDelta/4 + !!(offsetDelta % 4));

							result.push_back({
								m00, m10, m20,
								m01, m11, m21,
								m02, m12, m22
							});

							byteOffset += bufferView.byteStride;
						}
					} else throw std::runtime_error("Accessor for Mat3s");
				} break;

				case AccessorType::Mat4: {
					if constexpr (std::is_same<return_t, akm::Mat4>::value) {
						for(int32 i = 0; i < accessor.count; i++) {
							int32 lastOffset, offsetDelta;

							lastOffset = byteOffset;
							fpSingle m00 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m10 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m20 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m30 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							offsetDelta = byteOffset - lastOffset;
							byteOffset = lastOffset + 4*(offsetDelta/4 + !!(offsetDelta % 4));

							lastOffset = byteOffset;
							fpSingle m01 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m11 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m21 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m31 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							offsetDelta = byteOffset - lastOffset;
							byteOffset = lastOffset + 4*(offsetDelta/4 + !!(offsetDelta % 4));

							lastOffset = byteOffset;
							fpSingle m02 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m12 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m22 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m32 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							offsetDelta = byteOffset - lastOffset;
							byteOffset = lastOffset + 4*(offsetDelta/4 + !!(offsetDelta % 4));

							lastOffset = byteOffset;
							fpSingle m03 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m13 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m23 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							fpSingle m33 = internal::extractValue<fpSingle>(buffer, accessor.componentType, byteOffset, accessor.normalized);
							offsetDelta = byteOffset - lastOffset;
							byteOffset = lastOffset + 4*(offsetDelta/4 + !!(offsetDelta % 4));

							result.push_back({
								m00, m10, m20, m30,
								m01, m11, m21, m31,
								m02, m12, m22, m32,
								m03, m13, m23, m33
							});

							byteOffset += bufferView.byteStride;
						}
					} else throw std::runtime_error("Accessor for Mat4s");
				} break;
			}
			return result;
		}

	}
}



#endif /* AKRES_GLTF_UTIL_HPP_ */
