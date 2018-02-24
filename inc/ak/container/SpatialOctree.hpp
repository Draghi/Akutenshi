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

#ifndef AK_CONTAINER_SPACIALOCTREE_HPP_
#define AK_CONTAINER_SPACIALOCTREE_HPP_

#include <ak/Bits.hpp>
#include <ak/container/SlotMap.hpp>
#include <ak/Log.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <glm/common.hpp>
#include <glm/detail/type_vec3.hpp>
#include <sparsepp/spp.h>
#include <bitset>
#include <climits>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace akc {
	namespace internal {

		enum NodeIndex : uint8 {
			//      XYZ
			lbn = 0b000,
			lbf = 0b001,
			ltn = 0b010,
			ltf = 0b011,
			rbn = 0b100,
			rbf = 0b101,
			rtn = 0b110,
			rtf = 0b111,

			ni_last = 0b111,
			ni_size = ni_last + 1,
		};

		class Location {
			public:
				using data_t = uint8;
				static constexpr akSize MAX_POS = std::numeric_limits<data_t>::max();
				static constexpr akSize MAX_DEPTH = sizeof(data_t)*CHAR_BIT;

			private:
				data_t m_x;
				data_t m_y;
				data_t m_z;
				data_t m_depth;

			public:
				Location() : m_x(0), m_y(0), m_z(0), m_depth(0) {}
				Location(data_t xVal, data_t yVal, data_t zVal, data_t dVal) : m_x(xVal), m_y(yVal), m_z(zVal), m_depth(dVal) { setDepth(dVal); }
				Location(const Location&) = default;
				Location(Location&&) = default;

				Location atDepth(uint8 depth) const { return Location(*this).setDepth(depth); }
				Location& setDepth(uint8 depth) {
					auto mask = ~ak::bitmask<data_t>(MAX_DEPTH - std::min<data_t>(m_depth, depth));
					m_x = m_x & mask; m_y = m_y & mask; m_z = m_z & mask; m_depth = depth;
					return *this;
				}

				Location above() const { return Location(*this).goUp(); }
				Location& goUp() {
					if (m_depth == 0) throw std::out_of_range("Location::goToNode: Attempt to index out of range");
					return setDepth(m_depth - 1);
				}

				Location atNode(NodeIndex node) const { return Location(*this).goToNode(node); }
				Location& goToNode(NodeIndex node) {
					if (m_depth == MAX_DEPTH) throw std::out_of_range("Location::goToNode: Attempt to index out of range");
					m_depth++;
					m_x |= ((node >> 2u) & 0x01u) << (MAX_DEPTH - m_depth);
					m_y |= ((node >> 1u) & 0x01u) << (MAX_DEPTH - m_depth);
					m_z |= ((node >> 0u) & 0x01u) << (MAX_DEPTH - m_depth);
					return *this;
				}

				NodeIndex indexAt(uint8 dVal) const {
					if (dVal == 0) return lbn;
					if (dVal > m_depth) throw std::out_of_range("Location::indexAt: Attempt to index out of range");
					return static_cast<NodeIndex>((zOrder() >> ((MAX_DEPTH - dVal)*3)) & 0x07);
				}

				uint32 zOrder() const {
					static uint32* lookup = []{
						static uint32 result[MAX_POS + 1] = {0};
						for(uint32 i = 0u; i <= MAX_POS; i++) {
							for(uint8 j = 0u; j < MAX_DEPTH; j++) {
								result[i] = (result[i] << 3) | ((i >> (MAX_DEPTH - (j + 1))) & 0x01);
							}
						}
						return result;
					}();
					return ((static_cast<uint32>(m_depth) << (MAX_DEPTH*3)) | (lookup[m_x] << 2ull) | (lookup[m_y] << 1ull) | (lookup[m_z] << 0ull));
				}

				uint8 x() const { return m_x; }
				uint8 y() const { return m_y; }
				uint8 z() const { return m_z; }
				uint8 depth() const { return m_depth; }

				Location& operator=(const Location&) = default;
				Location& operator=(Location&&) = default;

				bool operator==(const Location& v) const { return zOrder() == v.zOrder(); }
				bool operator!=(const Location& v) const { return zOrder() != v.zOrder(); }
				bool operator< (const Location& v) const { return zOrder() <  v.zOrder(); }
				bool operator<=(const Location& v) const { return zOrder() <= v.zOrder(); }
				bool operator> (const Location& v) const { return zOrder() >  v.zOrder(); }
				bool operator>=(const Location& v) const { return zOrder() >= v.zOrder(); }
		};

		enum class OctreeMove {
			FAIL    = 0,
			SUCCESS = 1,
			REMOVE  = 2,
		};

		struct ParentNode { uint8 children; };
		struct ValueNode { std::vector<SlotID> values; };

		template<typename type_t, typename alloc_t = std::allocator<type_t>> class SpatialOctree final {
			private:
				struct DataRecord {
					std::pair<Location, Location> range;
					type_t value;
				};

				SlotMap<DataRecord> m_data;
				spp::sparse_hash_map<Location, ParentNode> m_parentGrid;
				spp::sparse_hash_map<Location, ValueNode>  m_sparseGrid;

				akm::Vec3 m_offset;
				akm::Vec3 m_unitSize;

				akm::Vec3 worldToLocalSpace(const akm::Vec3& vec) { return m_unitSize * (m_offset + vec); }
				akm::Vec3 localToWorldSpace(const akm::Vec3& vec) { return vec/m_unitSize - m_offset; }

				std::optional<std::pair<Location, Location>> getNodeRange(const akm::Vec3& position, const akm::Vec3& halfSize) {
					auto bboxMin = worldToLocalSpace(position - akm::abs(halfSize));
					auto bboxMax = worldToLocalSpace(position + akm::abs(halfSize));

					if ((bboxMax.x <= 0) || (bboxMax.y <= 0) || (bboxMax.z <= 0)) return {};
					if ((bboxMin.x > Location::MAX_POS) || (bboxMin.y > Location::MAX_POS) || (bboxMin.z > Location::MAX_POS)) return {};
					for(auto i = 0; i < 3; i++) if (bboxMax[i] == akm::floor(bboxMax[i])) bboxMax[i] -= 1;

					Location::data_t minX = std::max(0u, static_cast<akSize>(akm::floor(bboxMin.x)));
					Location::data_t minY = std::max(0u, static_cast<akSize>(akm::floor(bboxMin.y)));
					Location::data_t minZ = std::max(0u, static_cast<akSize>(akm::floor(bboxMin.z)));

					Location::data_t maxX = std::min(Location::MAX_POS, static_cast<akSize>(akm::floor(bboxMax.x)));
					Location::data_t maxY = std::min(Location::MAX_POS, static_cast<akSize>(akm::floor(bboxMax.y)));
					Location::data_t maxZ = std::min(Location::MAX_POS, static_cast<akSize>(akm::floor(bboxMax.z)));

					return {{
						Location(minX, minY, minZ, Location::MAX_DEPTH),
						Location(maxX, maxY, maxZ, Location::MAX_DEPTH)
					}};
				}

				void insertEntryAt(Location location, SlotID entryID) {
					/* Update sparse grid */ {
						auto result = m_sparseGrid.insert({location, ValueNode()});
						result.first->second.values.push_back(entryID);
						if (!result.second) return;
					}

					/* Update parent grid */ {
						for(akSSize i = Location::MAX_DEPTH - 1; i >= 0; i--) {
							auto cLocation = location.atDepth(i);

							auto result = m_parentGrid.insert({cLocation, ParentNode{0}});
							result.first->second.children = result.first->second.children | (0x0001u << location.indexAt(i + 1));

							if (!result.second) return;
						}
					}
				}

				bool removeEntryFrom(Location location, SlotID entryID) {
					/* Update sparse grid */  {
						auto iter = m_sparseGrid.find(location);
						if (iter == m_sparseGrid.end()) return false;

						auto entry = std::find(iter->second.values.begin(), iter->second.values.end(), entryID);
						if (entry == iter->second.values.end()) throw std::logic_error("Octree::removeEntryFrom: Could not find entry in node, however, it's existence is required.");
						iter->second.values.erase(entry);

						if (!iter->second.values.empty()) return true;

						m_sparseGrid.erase(iter);
					}

					/* Update parent grid */ {
						for(akSSize i = Location::MAX_DEPTH - 1; i >= 0; i--) {
							auto cLocation = location.atDepth(i);

							auto iter = m_parentGrid.find(cLocation);
							if (iter == m_parentGrid.end()) throw std::logic_error("Octree::removeEntryFrom: Could not find parent, however, it's existence is required.");

							iter->second.children = iter->second.children & ~(0x0001 << (location.indexAt(i + 1)));

							if (iter->second.children != 0) return true;

							m_parentGrid.erase(iter);
						}
					}

					return true;
				}

				template<typename func_t> void traverseInternal(const spp::sparse_hash_map<Location, ParentNode>::iterator& node, const func_t& visitFunc) {
					visitFunc(localToWorldSpace(akm::Vec3(node->first.x(),node->first.y(),node->first.z())), node->first.depth(), std::vector<SlotID>());

					if (node->first.depth() == Location::MAX_DEPTH - 1) {
						for(auto i = 0u; i < ni_size; i++) {
							 if ((node->second.children & (0x0001 << i)) != 0) {
								 auto loc = node->first.atNode(static_cast<NodeIndex>(i));
								visitFunc(localToWorldSpace(akm::Vec3(loc.x(),loc.y(),loc.z())), loc.depth(), m_sparseGrid.at(loc).values);
							 }
						}
					} else {
						for(auto i = 0u; i < ni_size; i++) {
							 if ((node->second.children & (0x0001 << i)) != 0) traverseInternal(m_parentGrid.find(node->first.atNode(static_cast<NodeIndex>(i))), visitFunc);
						}
					}
				}

			public:
				SpatialOctree(const akm::Vec3& position, const akm::Vec3& scale) : m_offset(-position), m_unitSize(1.f/scale) {}

				SlotID insert(const type_t& val, const akm::Vec3& position, const akm::Vec3& halfSize) {
					auto range = getNodeRange(position, halfSize);
					if (!range) {
						akl::Logger("O").info("No Range");
						return SlotID();
					}

					auto entryID = m_data.insert(DataRecord{*range, val}).first;

					for(akSize x = range->first.x(); x <= range->second.x(); x++) {
						for(akSize y = range->first.y(); y <= range->second.y(); y++) {
							for(akSize z = range->first.z(); z <= range->second.z(); z++) {
								insertEntryAt(Location(x, y, z, Location::MAX_DEPTH), entryID);
							}
						}
					}

					return entryID;
				}

				OctreeMove move(SlotID entryID, const akm::Vec3& position, const akm::Vec3& halfSize) {
					auto entry = m_data.find(entryID);
					if (entry == m_data.end()) return OctreeMove::FAIL;

					auto oldRange = entry->range;
					auto newRange = getNodeRange(position, halfSize);

					if (!newRange) {
						remove(entryID);
						return OctreeMove::REMOVE;
					}

					if ((oldRange.first == newRange->first) && (oldRange.second == newRange->second)) return OctreeMove::SUCCESS;
					entry->range = *newRange;

					// Add new nodes first, to prevent pointless deallocation/reallocation/traversal when removing nodes.

					// Add new nodes
					for(akSize x = newRange->first.x(); x <= newRange->second.x(); x++) {
						for(akSize y = newRange->first.y(); y <= newRange->second.y(); y++) {
							for(akSize z = newRange->first.z(); z <= newRange->second.z(); z++) {
								if ((x >= oldRange.first.x()) && (x <= oldRange.second.x()) && (y >= oldRange.first.y()) && (y <= oldRange.second.y()) && (z >= oldRange.first.z()) && (z <= oldRange.second.z())) continue;
								insertEntryAt(Location(x, y, z, Location::MAX_DEPTH), entryID);
							}
						}
					}

					// Remove unneeded nodes
					for(akSize x = oldRange.first.x(); x <= oldRange.second.x(); x++) {
						for(akSize y = oldRange.first.y(); y <= oldRange.second.y(); y++) {
							for(akSize z = oldRange.first.z(); z <= oldRange.second.z(); z++) {
								if ((x >= newRange->first.x()) && (x <= newRange->second.x()) && (y >= newRange->first.y()) && (y <= newRange->second.y()) && (z >= newRange->first.z()) && (z <= newRange->second.z())) continue;
								removeEntryFrom(Location(x, y, z, Location::MAX_DEPTH), entryID);
							}
						}
					}

					return OctreeMove::SUCCESS;
				}

				bool remove(SlotID entryID) {
					auto entry = m_data.find(entryID);
					if (entry == m_data.end()) return false;
					auto range = entry->range;
					m_data.erase(entry);

					for(akSize x = range.first.x(); x <= range.second.x(); x++) {
						for(akSize y = range.first.y(); y <= range.second.y(); y++) {
							for(akSize z = range.first.z(); z <= range.second.z(); z++) {
								removeEntryFrom(Location(x, y, z, Location::MAX_DEPTH), entryID);
							}
						}
					}

					return true;
				}

				template<typename func_t> void castRay(akm::Vec3 pos, akm::Vec3 dir, fpSingle distance, const func_t& visitFunc) {
					castRay(pos, pos + dir*distance, visitFunc);
				}

				template<typename func_t> void castRay(const akm::Vec3& p0, const akm::Vec3& p1, const func_t& visitFunc) {

					fpSingle t = 0;
					int n = 1;

					akm::Vec3 pos = akm::floor(p0);
					akm::Vec3 tileDelta = p1 - p0;
					akm::Vec3 tileOffsetIncrement = 1.f/akm::abs(tileDelta);
					akm::Vec3 tileOffset;
					akm::Vec3 tileIncrement;

					for(auto i = 0; i < 3; i++) {
						if (tileDelta[i] == 0) {
							tileIncrement[i] = 0;
							tileOffset[i] = std::numeric_limits<fpSingle>::max();
						} else if (p1[i] > p0[i]) {
							tileIncrement[i] = 1;
							n += int(akm::floor(p1[i]) - pos[i]);
							tileOffset[i] = (akm::floor(p0[i]) + 1 - p0[i]) * tileOffsetIncrement[i];
						} else {
							tileIncrement[i] = -1;
							n += int(pos[i] - akm::floor(p1[i]));
							tileOffset[i] = (p0[i] - akm::floor(p0[i])) * tileOffsetIncrement[i];
						}
					}

					auto getAdvanceIndex = [](akm::Vec3 val) {
						akSSize minIndex = 0;
						if (val[1] < val[minIndex]) minIndex = 1;
						if (val[2] < val[minIndex]) minIndex = 2;
						return minIndex;
					};

					for (; n > 0; --n) {
						auto valIter = m_sparseGrid.find(Location(static_cast<uint>(pos.x), static_cast<uint>(pos.y), static_cast<uint>(pos.z), Location::MAX_DEPTH));
						if ((valIter != m_sparseGrid.end()) && (!visitFunc(pos, p0 + (tileDelta * t)))) return;

						auto index = getAdvanceIndex(tileOffset);
						pos[index] += tileIncrement[index];
						t = tileOffset[index];
						tileOffset[index] += tileOffsetIncrement[index];
					}
				}

				template<typename func_t> void traverse(const func_t& visitFunc) {
					auto iter = m_parentGrid.find(Location());
					if (iter == m_parentGrid.end()) return;
					traverseInternal(iter, visitFunc);
				}

				akm::Vec3 nodePosition(akm::Vec3 position, Location::data_t depth = Location::MAX_DEPTH) {
					auto gridPos = worldToLocalSpace(position);
					auto depthFactor = akm::pow(2, Location::MAX_DEPTH - depth);
					auto depthDiv = 1/depthFactor;

					return akm::Vec3{
						akm::floor(gridPos.x * depthDiv) * depthFactor,
						akm::floor(gridPos.y * depthDiv) * depthFactor,
						akm::floor(gridPos.z * depthDiv) * depthFactor
					};
				}

				fpSingle unscaledGridSize(uint8 depth) const {
					return 0x0001ull << (Location::MAX_DEPTH - depth);
				}

				akm::Vec3 gridSize(uint8 depth) const {
					auto unscaledSize = unscaledGridSize(depth);
					return akm::Vec3(unscaledSize, unscaledSize, unscaledSize)/m_unitSize;
				}

				akm::Vec3 center() const { return -m_offset; }
				akm::Vec3 scale() const { return 1./m_unitSize; }
				Location::data_t maxDepth() const { return Location::MAX_DEPTH; }

		};
	}


	template<typename type_t, typename alloc_t = std::allocator<type_t>> using SpatialOctree = internal::SpatialOctree<type_t, alloc_t>;
	using internal::OctreeMove;
}

namespace std {
	template<> struct hash<akc::internal::Location> {
		size_t operator()(const akc::internal::Location& v) { return v.zOrder(); }
	};
}

#endif
