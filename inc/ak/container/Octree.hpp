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

#ifndef AK_CONTAINER_OCTREE_HPP_
#define AK_CONTAINER_OCTREE_HPP_

#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <stdexcept>

namespace akc {

	enum OctreeIndex : uint8 {
		// XYZ
		LBN = 0b000, // 0
		LBF = 0b001, // 1
		LTN = 0b010, // 2
		LTF = 0b011, // 3

		RBN = 0b100, // 4
		RBF = 0b101, // 5
		RTN = 0b110, // 6
		RTF = 0b111, // 7
	};

	struct OctreeLocation {
		struct Loc {
			uint16 x;
			uint16 y;
			uint16 z;
		} location;

		uint8 depth;


		OctreeLocation() : location{0,0,0}, depth(0) {}
		OctreeLocation(uint16 x, uint16 y, uint16 z, uint8 depthVal = 15) : location{x,y,z}, depth(depthVal) {}
		OctreeLocation(akm::Vec3 pos, akm::Vec3 unit = akm::Vec3(1, 1, 1)) : location{static_cast<uint16>(pos.x * unit.x), static_cast<uint16>(pos.y * unit.y), static_cast<uint16>(pos.z * unit.z)}, depth(15) {}

		uint8 getIndexForDepth(uint8 depthIndex) {
			if (depth > 15) throw std::out_of_range("OctreeLocation: Attempt to index out of bounds");
			return static_cast<uint8>(
				  (((location.x >> (15 - depthIndex)) & 0x01) << 2)
				| (((location.y >> (15 - depthIndex)) & 0x01) << 1)
				| (((location.z >> (15 - depthIndex)) & 0x01) << 0));
		}
	};

}

#endif
