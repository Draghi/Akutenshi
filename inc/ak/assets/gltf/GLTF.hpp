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

#ifndef AK_ASSETS_GLTF_GLTF_HPP_
#define AK_ASSETS_GLTF_GLTF_HPP_

#include <ak/assets/Convert.hpp>
#include <ak/assets/Material.hpp>
#include <ak/assets/Mesh.hpp>
#include <ak/data/SUID.hpp>
#include <ak/filesystem/Path.hpp>
#include <utility>
#include <vector>

namespace akas {
	namespace gltf {

		bool convertGLTFFile(akas::ConversionHelper& convertHelper, const akfs::Path& root, akd::PValue& cfg);
	}
}

#endif /* AKRES_GLTF_HPP_ */
