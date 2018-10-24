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

#ifndef AK_COMMON_SPARSEGRID_HPP_
#define AK_COMMON_SPARSEGRID_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/SlotMap.hpp>
#include <akcommon/UnorderedVector.hpp>
#include <akmath/Scalar.hpp>
#include <akmath/Types.hpp>
#include <akmath/Vector.hpp>
#include <algorithm>
#include <climits>
#include <iterator>
#include <type_traits>
#include <limits>
#include <optional>
#include <unordered_map>
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

			ILoc(value_type xVal, value_type yVal, value_type zVal) : x(xVal), y(yVal), z(zVal) {}
			ILoc(akm::Vec3& pos) : x(static_cast<value_type>(pos.x)), y(static_cast<value_type>(pos.y)), z(static_cast<value_type>(pos.z)) {}

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

				std::optional<std::pair<ILoc, ILoc>> getNodeRange(const akm::Vec3& position, akm::Vec3 halfSize) {
					halfSize = akm::abs(halfSize);
					auto bboxMin = worldToLocal(position - halfSize);
					auto bboxMax = worldToLocal(position + halfSize);

					// Fail if bbox is totally outside the grid
					if ((bboxMax.x <= 0) || (bboxMax.y <= 0) || (bboxMax.z <= 0)) return {};
					if ((bboxMin.x > ILoc::val_max) || (bboxMin.y >  ILoc::val_max) || (bboxMin.z > ILoc::val_max)) return {};

					bboxMin = akm::max(akm::floor(bboxMin), akm::Vec3{0,0,0}); // No point indexing outside the box

					// @bug Might cause a directional bias bug?
					for(auto i = 0; i < 3; i++) if (bboxMax[i] == akm::floor(bboxMax[i])) bboxMax[i] -= 1; // Bump it back if it matches the far edge (ie. a unit cube in the center of a cell should only take up 1 cube)
					bboxMax = akm::clamp(akm::floor(bboxMax), bboxMin, akm::Vec3{ILoc::val_max,ILoc::val_max,ILoc::val_max}); // Clamp to ensure it doesn't cover a negative range because of the adjustment above.

					return {{
						ILoc{static_cast<ILoc::value_type>(bboxMin.x), static_cast<ILoc::value_type>(bboxMin.y), static_cast<ILoc::value_type>(bboxMin.z)},
						ILoc{static_cast<ILoc::value_type>(bboxMax.x), static_cast<ILoc::value_type>(bboxMax.y), static_cast<ILoc::value_type>(bboxMax.z)}
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

				template<typename func_t> bool raycastInternal(const akm::Vec3& rayStart, const akm::Vec3& rayEnd, const func_t& visitFunc) {
					fpSingle  rayProgress = 0;
					akm::Vec3 rayDelta    = rayEnd - rayStart;
					akm::Vec3 rayInvDelta = 1.f/rayDelta;
					akm::Vec3 rayStepSize = 1.f/akm::abs(rayDelta);

					akm::Vec3 tilePos = akm::floor(rayStart);
					akm::Vec3 tileEnd = akm::floor(rayEnd);
					akm::Vec3 tileIncrement = akm::forEachV(rayEnd, rayStart, [](auto start, auto end){ return start == end ? 0 : (start < end ? -1 : 1); });
					akm::Vec3 tileDirOffset = akm::max(akm::Vec3{0,0,0}, tileIncrement);
					akm::Vec3 tileOffset = (((tilePos+tileDirOffset) - rayStart)*rayInvDelta) * akm::abs(tileIncrement);

					akSize cellCount = 1 + static_cast<akSize>(akm::sum(akm::abs(tileEnd - tilePos)));
					for (akSize i = 0; i < cellCount; i++) {
						auto advIndex = 0; // @todo Double check this... In side project there's a degenerate case in lines that only move along one axis...
						if (tileOffset[1] < tileOffset[advIndex]) advIndex = 1;
						if (tileOffset[2] < tileOffset[advIndex]) advIndex = 2;

						ILoc cLoc(tilePos);
						auto iter = m_grid.find(cLoc.toMortonIndex());
						if (iter != m_grid.end()) {
							auto curPos = rayStart + rayDelta*rayProgress;
							auto nextPos =  (tileOffset[advIndex] > 1) ? rayEnd : rayStart + rayDelta*tileOffset[advIndex];
							if (!visitFunc(localToWorld(cLoc.toVec()), localToWorld(curPos), localToWorld(nextPos))) return false;
						}

						tilePos[advIndex] += tileIncrement[advIndex];
						rayProgress = tileOffset[advIndex];
						tileOffset[advIndex] += rayStepSize[advIndex];
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

					// Early-outs
					if (!newRange) { remove(entryID); return MoveResult::Removed; }
					if ((oldRange.first == newRange->first) && (oldRange.second == newRange->second)) return MoveResult::Success;

					entry->bounds = *newRange;

					auto isInRange = [](auto x, auto y, auto z, const auto range){
						return (x >= range.first.x) && (y >=range.first.y) && (z >= range.first.z) && (x <= range.second.x) && (y <=range.second.y) && (z <= range.second.z);
					};

					// Add new nodes
					for(uint64 x = newRange->first.x; x <= newRange->second.x; x++) {
						for(uint64 y = newRange->first.y; y <= newRange->second.y; y++) {
							for(uint64 z = newRange->first.z; z <= newRange->second.z; z++) {
								if (!isInRange(x,y,z, oldRange)) insertEntryAt(entryID, ILoc{x, y, z});
							}
						}
					}

					// Remove unneeded nodes
					for(uint64 x = oldRange.first.x; x <= oldRange.second.x; x++) {
						for(uint64 y = oldRange.first.y; y <= oldRange.second.y; y++) {
							for(uint64 z = oldRange.first.z; z <= oldRange.second.z; z++) {
								if (!isInRange(x,y,z, *newRange)) removeEntryFrom(entryID, ILoc{x, y, z});
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
						visitFunc(localToWorld({loc.x, loc.y, loc.z}), 1.f/m_unitScale);
					}
				}

				template<typename func_t> void castLine(akm::Vec3 p0, akm::Vec3 p1, const func_t& visitFunc) {
					p0 = worldToLocal(p0);
					p1 = worldToLocal(p1);

					auto pDelta = p1 - p0;
					auto pInvDir = 1.f/pDelta;

					akm::Vec3 bbMin{0,0,0}, bbMax{ILoc::val_max + 1, ILoc::val_max + 1, ILoc::val_max + 1}; // +1 to bbox max due to it's volume
					akm::Vec3 t1 = (bbMin - p0)*pInvDir, t2 = (bbMax - p0)*pInvDir;
					fpSingle tmin = akm::max(akm::min(t1, t2)), tmax = akm::min(akm::max(t1, t2));

				    if (tmax < tmin) return; // Outside bounds
				    if ((tmax < 0) || (tmin > 1)) return;

					auto startPos = (tmin <= 0) ? p0 : p0 + pDelta*tmin;
					auto   endPos = (tmax >= 1) ? p1 : p0 + pDelta*tmax;

					raycastInternal(startPos, endPos, visitFunc);
				}

				template<typename func_t> void castLine(const akm::Vec3& pos, const akm::Vec3& dir, fpSingle distance, const func_t& visitFunc) {
					castLine(pos, pos + distance*dir, visitFunc);
				}

				akm::Vec3 cellBounds() const { return 1.f/m_unitScale; }
				akm::Vec3 gridBounds() const { return (static_cast<fpSingle>(ILoc::val_max) + 1)/m_unitScale; }
		};

	}

}

#endif
