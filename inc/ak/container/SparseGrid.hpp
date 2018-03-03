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

#ifndef AK_CONTAINER_SPARSEGRID_HPP_
#define AK_CONTAINER_SPARSEGRID_HPP_

#include <ak/container/SlotMap.hpp>
#include <ak/container/UnorderedVector.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/math/Types.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/ScopeGuard.hpp>
#include <glm/detail/type_vec3.hpp>
#include <algorithm>
#include <climits>
#include <iterator>
#include <limits>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>


namespace akc {
	namespace sparsegrid {

		struct ILoc final {
			using value_type = uint8;
			using index_type = uint32;
			static constexpr value_type val_min = std::numeric_limits<ILoc::value_type>::min();
			static constexpr value_type val_max = std::numeric_limits<ILoc::value_type>::max();
			static constexpr uint64 val_range = (val_max - val_min) + 1;

			value_type x;
			value_type y;
			value_type z;

			index_type toMortonIndex() {
				static index_type* lookup = []{
					constexpr uint64 bitCount = sizeof(value_type)*CHAR_BIT;
					static index_type result[val_range] = {0};
					for(auto i = 0u; i < val_range; i++) {
						for(auto j = 0u; j < bitCount; j++) result[i] = (result[i] << 3) | ((i >> (bitCount - (1+j))) & 0x01);
					}
					return result;
				}();
				return (lookup[x] << 2) | (lookup[y] << 1) | (lookup[z] << 0);
			}

			static ILoc fromMortonIndex(index_type id) {
				static auto convert = [](index_type v) {
					constexpr uint64 bitCount = sizeof(value_type)*CHAR_BIT;
					value_type result = 0;
					for(uint64 i = 0; i < bitCount; i++) result |= ((v >> (i*3)) & 0x01) << i;
					return result;
				};
				return ILoc{convert((id >> 2) & 0x249249), convert((id >> 1) & 0x249249), convert((id >> 0) & 0x249249)};
			}

			akm::Vec3 toVec() { return akm::Vec3(x,y,z); }

			bool operator==(const ILoc& other) const { return (x == other.x) && (y == other.y) && (z == other.z); }
		};

		enum class MoveResult {
			Failed,
			Removed,
			Success,
		};

		template<typename type_t> class SparseGrid final {
			public:
				using value_type = type_t;

			private:
				struct DataRecord final {
					std::pair<ILoc, ILoc> bounds;
					type_t value;
				};

				using node_type = akc::UnorderedVector<akc::SlotID>;
				akc::SlotMap<DataRecord> m_data;
				std::unordered_map<ILoc::index_type, node_type> m_grid;

				akm::Vec3 m_offset;
				akm::Vec3 m_unitScale;

				akm::Vec3 worldToLocal(const akm::Vec3& worldPos) { return m_unitScale*(worldPos+m_offset); }
				akm::Vec3 localToWorld(const akm::Vec3& localPos) { return localPos/m_unitScale-m_offset; }

				std::optional<std::pair<ILoc, ILoc>> getNodeRange(const akm::Vec3& position, const akm::Vec3& halfSize) {
					auto bboxMin = worldToLocal(position - akm::abs(halfSize));
					auto bboxMax = worldToLocal(position + akm::abs(halfSize));

					if ((bboxMax.x <= 0) || (bboxMax.y <= 0) || (bboxMax.z <= 0)) return {};
					if ((bboxMin.x > ILoc::val_max) || (bboxMin.y >  ILoc::val_max) || (bboxMin.z > ILoc::val_max)) return {};
					for(auto i = 0; i < 3; i++) if (bboxMax[i] == akm::floor(bboxMax[i])) bboxMax[i] -= 1; // Bump it back if it matches the far edge

					bboxMin = akm::max(akm::floor(bboxMin), akm::Vec3{0,0,0});
					bboxMax = akm::clamp(akm::floor(bboxMax), bboxMin, akm::Vec3{ILoc::val_max,ILoc::val_max,ILoc::val_max});

					return {{
						ILoc{
							static_cast<ILoc::value_type>(bboxMin.x),
							static_cast<ILoc::value_type>(bboxMin.y),
							static_cast<ILoc::value_type>(bboxMin.z)},
						ILoc{
							static_cast<ILoc::value_type>(bboxMax.x),
							static_cast<ILoc::value_type>(bboxMax.y),
							static_cast<ILoc::value_type>(bboxMax.z)}
					}};
				}

				void insertEntryAt(SlotID id, ILoc::index_type pos) { m_grid[pos].insert(id); }
				void insertEntryAt(SlotID id, ILoc pos) { insertEntryAt(id, pos.toMortonIndex()); }


				bool removeEntryFrom(SlotID id, ILoc::index_type pos) {
					auto iter = m_grid.find(pos);
					if (iter == m_grid.end()) return false;
					iter->second.erase(std::find(iter->second.begin(), iter->second.end(), id));
					if (iter->second.size() == 0) m_grid.erase(iter);
					return true;
				}
				bool removeEntryFrom(SlotID id, ILoc pos) { return removeEntryFrom(id, pos.toMortonIndex()); }

				template<typename func_t> bool raycastInternal(const akm::Vec3& pStart, const akm::Vec3& pEnd, const func_t& visitFunc) {
					akSize   cellCount = 1;
					fpSingle cellSize  = 1;

					fpSingle  rayProgress = 0;
					akm::Vec3 rayStart    = pStart;
					akm::Vec3 rayEnd      = pEnd;
					akm::Vec3 rayDelta    = rayEnd - rayStart;
					akm::Vec3 rayInvDelta = 1.f/rayDelta;

					akm::Vec3 tilePos    = akm::floor(rayStart/cellSize);
					akm::Vec3 tileEndPos = akm::floor(rayEnd/cellSize);
					akm::Vec3 tileOffsetIncrement = cellSize/akm::abs(rayDelta);
					akm::Vec3 tileOffset;
					akm::Vec3 tileIncrement;

					for(auto i = 0; i < 3; i++) {
						if (rayDelta[i] == 0) {
							tileIncrement[i] = 0;
							tileOffset[i] = std::numeric_limits<fpSingle>::max();
						} else {
							int cOff = rayEnd[i] > rayStart[i]; // false - 0, true - 1
							tileIncrement[i] = cOff*2 - 1;
							tileOffset[i] = ((tilePos[i]+cOff)*cellSize - rayStart[i])*rayInvDelta[i];
							cellCount += static_cast<akSize>(akm::abs(tileEndPos[i] - tilePos[i]));
						}
					}

					for (akSize i = 0; i < cellCount; i++) {
						auto advIndex = 0;
						if (tileOffset[1] < tileOffset[advIndex]) advIndex = 1;
						if (tileOffset[2] < tileOffset[advIndex]) advIndex = 2;

						ILoc cLoc{static_cast<ILoc::value_type>(akm::floor(tilePos.x)), static_cast<ILoc::value_type>(akm::floor(tilePos.y)), static_cast<ILoc::value_type>(akm::floor(tilePos.z))};
						auto iter = m_grid.find(cLoc.toMortonIndex());
						if (iter != m_grid.end()) {
							auto curPos = rayStart + rayDelta*rayProgress;
							auto nextPos =  (tileOffset[advIndex] > 1) ? rayEnd : rayStart + rayDelta*tileOffset[advIndex];
							if (!visitFunc(cLoc, localToWorld(tilePos), 1.f/m_unitScale, localToWorld(curPos), localToWorld(nextPos))) return false;
						}

						tilePos[advIndex] += tileIncrement[advIndex];
						rayProgress = tileOffset[advIndex];
						tileOffset[advIndex] += tileOffsetIncrement[advIndex];
					}

					return true;
				}

			public:
				SparseGrid() : m_offset(0,0,0), m_unitScale(1,1,1) {}
				SparseGrid(const akm::Vec3& position, const akm::Vec3& gridSize) : m_offset(-position), m_unitScale(1.f/gridSize) {}

				SlotID insert(const type_t& val, const akm::Vec3& position, const akm::Vec3& halfSize) {
					auto range = getNodeRange(position, halfSize);
					if (!range) return SlotID();

					auto entryID = m_data.insert(DataRecord{*range, val}).first;

					for(uint64 x = range->first.x; x <= range->second.x; x++) {
						for(uint64 y = range->first.y; y <= range->second.y; y++) {
							for(uint64 z = range->first.z; z <= range->second.z; z++) {
								insertEntryAt(entryID, ILoc{x, y, z});
							}
						}
					}

					return entryID;
				}

				MoveResult move(SlotID entryID, const akm::Vec3& position, const akm::Vec3& halfsize) {
					auto entry = m_data.find(entryID);
					if (entry == m_data.end()) return MoveResult::Failed;

					auto oldRange = entry->bounds;
					auto newRange = getNodeRange(position, halfsize);

					if (!newRange) {
						remove(entryID);
						return MoveResult::Removed;
					}

					if ((oldRange.first == newRange->first) && (oldRange.second == newRange->second)) return MoveResult::Success;
					entry->bounds = *newRange;

					// Add new nodes
					for(uint64 x = newRange->first.x; x <= newRange->second.x; x++) {
						for(uint64 y = newRange->first.y; y <= newRange->second.y; y++) {
							for(uint64 z = newRange->first.z; z <= newRange->second.z; z++) {
								if ((x >= oldRange.first.x) && (x <= oldRange.second.x) && (y >= oldRange.first.y) && (y <= oldRange.second.y) && (z >= oldRange.first.z) && (z <= oldRange.second.z)) continue;
								insertEntryAt(entryID, ILoc{x, y, z});
							}
						}
					}

					// Remove unneeded nodes
					for(uint64 x = oldRange.first.x; x <= oldRange.second.x; x++) {
						for(uint64 y = oldRange.first.y; y <= oldRange.second.y; y++) {
							for(uint64 z = oldRange.first.z; z <= oldRange.second.z; z++) {
								if ((x >= newRange->first.x) && (x <= newRange->second.x) && (y >= newRange->first.y) && (y <= newRange->second.y) && (z >= newRange->first.z) && (z <= newRange->second.z)) continue;
								removeEntryFrom(entryID, ILoc{x, y, z});
							}
						}
					}

					return MoveResult::Success;
				}

				bool remove(SlotID entryID) {
					auto entry = m_data.find(entryID);
					if (entry == m_data.end()) return false;

					auto range = entry->bounds;
					m_data.erase(entry);

					for(akSize x = range.first.x; x <= range.second.x; x++) {
						for(akSize y = range.first.y; y <= range.second.y; y++) {
							for(akSize z = range.first.z; z <= range.second.z; z++) {
								removeEntryFrom(entryID, ILoc{x, y, z});
							}
						}
					}

					return true;
				}

				template<typename func_t> void iterate(const func_t& visitFunc) {
					for(auto iter = m_grid.begin(); iter != m_grid.end(); iter++) {
						ILoc loc = ILoc::fromMortonIndex(iter->first);
						visitFunc(localToWorld(akm::Vec3{loc.x, loc.y, loc.z}), 1.f/m_unitScale);
					}
				}

				template<typename func_t> void castRay(akm::Vec3 pos, const akm::Vec3& dir, const func_t& visitFunc) {
					pos = worldToLocal(pos);

					auto pInvDir = 1.f/dir;
					fpSingle bMin = 0, bMax = ILoc::val_max;

				    fpSingle tx1 = (bMin - pos.x)*pInvDir.x, tx2 = (bMax - pos.x)*pInvDir.x;
				    fpSingle ty1 = (bMin - pos.y)*pInvDir.y, ty2 = (bMax - pos.y)*pInvDir.y;
				    fpSingle tz1 = (bMin - pos.z)*pInvDir.z, tz2 = (bMax - pos.z)*pInvDir.z;

				    fpSingle tmin = akm::max(akm::Vec3{akm::min(tx1, tx2), akm::min(ty1, ty2), akm::min(tz1, tz2)});
				    fpSingle tmax = akm::min(akm::Vec3{akm::max(tx1, tx2), akm::max(ty1, ty2), akm::max(tz1, tz2)});

				    if (tmax < tmin) return; // Outside bounds

					auto startPos = (tmin <= 0) ? pos : pos + pInvDir*tmin;
					auto   endPos = pos + pInvDir*tmax;

					visitFunc(0,0,0,0, -m_offset, localToWorld(startPos), localToWorld(endPos));

					raycastInternal(startPos, endPos, visitFunc);
				}

				template<typename func_t> void traceLine(akm::Vec3 p0, akm::Vec3 p1, const func_t& visitFunc) {
					p0 = worldToLocal(p0);
					p1 = worldToLocal(p1);

					auto pDelta = p1 - p0;
					auto pInvDir = 1.f/pDelta;

					fpSingle bMin = 0, bMax = ILoc::val_max;

				    fpSingle tx1 = (bMin - p0.x)*pInvDir.x, tx2 = (bMax - p0.x)*pInvDir.x;
				    fpSingle ty1 = (bMin - p0.y)*pInvDir.y, ty2 = (bMax - p0.y)*pInvDir.y;
				    fpSingle tz1 = (bMin - p0.z)*pInvDir.z, tz2 = (bMax - p0.z)*pInvDir.z;

				    fpSingle tmin = akm::max(akm::Vec3{akm::min(tx1, tx2), akm::min(ty1, ty2), akm::min(tz1, tz2)});
				    fpSingle tmax = akm::min(akm::Vec3{akm::max(tx1, tx2), akm::max(ty1, ty2), akm::max(tz1, tz2)});

				    if (tmax < tmin) return; // Outside bounds
				    if ((tmax < 0) || (tmin > 1)) return;

					auto startPos = (tmin <= 0) ? p0 : p0 + pDelta*tmin;
					auto   endPos = (tmax >= 1) ? p1 : p0 + pDelta*tmax;

					visitFunc(ILoc{0,0,0}, -m_offset, 1.f/m_unitScale, localToWorld(startPos), localToWorld(endPos));

					raycastInternal(startPos, endPos, visitFunc);
				}

				template<typename func_t> void traceLine(const akm::Vec3& pos, const akm::Vec3& dir, fpSingle distance, const func_t& visitFunc) {
					traceLine(pos, pos + distance*dir, visitFunc);
				}

				akm::Vec3 gridUnit() const { return 1.f/m_unitScale; }
				akm::Vec3 gridSize() const { return static_cast<fpSingle>(ILoc::val_max)*gridUnit(); }
		};

	}

}

#endif
