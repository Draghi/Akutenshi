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

#ifndef AK_ASSETS_GLTF_BUFFERVIEW_HPP_
#define AK_ASSETS_GLTF_BUFFERVIEW_HPP_

#include <string>

#include <ak/assets/gltf/Types.hpp>
#include <ak/data/PValue.hpp>
#include <ak/PrimitiveTypes.hpp>

namespace akas {
	namespace gltf {

		enum class BufferTarget : int32 {
			None,
			Array   = 34962,
			Element = 34963
		};

		struct BufferView final {
			std::string name;
			gltfID bufferID;
			int32 byteOffset;
			int32 byteSize;
			int32 byteStride;
			BufferTarget target;
		};

		inline BufferView extractBufferView(const akd::PValue& bufferViewData) {
			static constexpr auto extractBufferTarget = [](const akd::PValue& bufferTargetData) {
				std::string target = bufferTargetData.getStrOrDef("");
				if (target == "ARRAY_BUFFER") return BufferTarget::Array;
				if (target == "ELEMENT_ARRAY_BUFFER") return BufferTarget::Element;
				return BufferTarget::None;
			};

			return BufferView{
				bufferViewData.atOrDef("name").getStrOrDef(""),
				bufferViewData["buffer"].as<gltfID>(),
				bufferViewData.atOrDef("byteOffset").asOrDef<int32>(0),
				bufferViewData["byteLength"].as<int32>(),
				bufferViewData.atOrDef("byteStride").asOrDef<int32>(0),
				extractBufferTarget(bufferViewData.atOrDef("target"))
			};
		}

	}
}

#endif
