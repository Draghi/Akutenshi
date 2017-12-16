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

#ifndef AK_RENDER_PRIMITIVES_HPP_
#define AK_RENDER_PRIMITIVES_HPP_

#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/math/Vector.hpp>
#include <vector>

#include "stddef.h"

namespace akr{

	/**
	 * Generates cube mesh data
	 * @param p1 - Bottom-Left-Front vertex (Default: (-0.5, -0.5, -0.5))
	 * @param p2 - Top-Right-Back vertex    (Default: ( 0.5,  0.5,  0.5))
	 * @param genNorms - Generate normals?
	 * @param genTexcoord - Generate texture coords?
	 * @remarks Format: Vx, Vy, Vz, <Nx, Ny, Nz,> <Ts, Tv,>
	 * @return The vertex data for the cube
	 */
	std::vector<fpSingle> genCubeMesh(akm::Vec3 p1 = akm::Vec3(-0.5, -0.5, -0.5), akm::Vec3 p2 = akm::Vec3(0.5, 0.5, 0.5), bool genNorms = true, bool genTexcoord = true);

}

#endif
