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

#include <ak/render/Primitives.hpp>

using namespace akr;

/*std::vector<fpSingle> akr::genCubeMesh(akm::Vec3 p1, akm::Vec3 p2, bool genNorms, bool genTexcoord) {

	const uint8 indicies[] = {
		0, 1, 2, //  Front - Bottom
		2, 3, 0, //  Front - Top
		1, 5, 6, //  Right - Bottom
		6, 2, 1, //  Right - Top
		5, 4, 7, //   Back - Bottom
		7, 6, 5, //   Back - Top
		4, 0, 3, //   Left - Bottom
		3, 7, 4, //   Left - Top
		3, 2, 6, //    Top - Front
		6, 7, 3, //    Top - Back
		4, 5, 1, // Bottom - Back
		1, 0, 4, // Bottom - Front
	};

	const akm::Vec3 norms[] = {
		akm::Vec3( 0,  0, -1), // Front
		akm::Vec3( 1,  0,  0), // Right
		akm::Vec3( 0,  0,  1), // Back
		akm::Vec3(-1,  0,  0), // Left
		akm::Vec3( 0,  1,  0), // Top
		akm::Vec3( 0, -1,  0), // Back
	};

	const akm::Vec2 texcoord[] = {
		akm::Vec2(0, 0),
		akm::Vec2(1, 0),
		akm::Vec2(1, 1),
		akm::Vec2(1, 1),
		akm::Vec2(0, 1),
		akm::Vec2(0, 0),
	};

	akm::Vec3 verts[] = {
		akm::Vec3(p1.x, p1.y, p1.z), // 0 - BLF
		akm::Vec3(p2.x, p1.y, p1.z), // 1 - BRF
		akm::Vec3(p2.x, p2.y, p1.z), // 2 - TRF
		akm::Vec3(p1.x, p2.y, p1.z), // 3 - TLF

		akm::Vec3(p1.x, p1.y, p2.z), // 4 - BLB
		akm::Vec3(p2.x, p1.y, p2.z), // 5 - BRB
		akm::Vec3(p2.x, p2.y, p2.z), // 6 - TRB
		akm::Vec3(p1.x, p2.y, p2.z), // 7 - TLB
	};

	std::vector<fpSingle> result;
	result.reserve(36*8);

	for(akSize i = 0; i < 36; i++) {
		result.push_back(verts[indicies[i]].x);
		result.push_back(verts[indicies[i]].y);
		result.push_back(verts[indicies[i]].z);

		if (genNorms) {
			akSize index = i/6;
			result.push_back(norms[index].x);
			result.push_back(norms[index].y);
			result.push_back(norms[index].z);
		}

		if (genTexcoord) {
			akSize index = i%6;
			result.push_back(texcoord[index].x);
			result.push_back(texcoord[index].y);
		}
	}

	result.shrink_to_fit();
	return result;
}*/
