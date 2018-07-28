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

#ifndef AK_ASSETS_GLTF_BUFFER_HPP_
#define AK_ASSETS_GLTF_BUFFER_HPP_

#include <string>
#include <vector>

#include <ak/data/Base64.hpp>
#include <ak/data/PValue.hpp>
#include <ak/PrimitiveTypes.hpp>

namespace akas {
	namespace gltf {

		struct Buffer {
			std::string name;
			std::string uri;
			std::vector<uint8> dataURI;
			int32 byteLength;
		};

		inline Buffer extractBuffer(const akd::PValue& bufferData) {
			std::string uri = bufferData.atOrDef("uri").getStrOrDef("");
			std::vector<uint8> dataURI;
			if (uri.rfind("data:", 0) == 0) { //@todo Test
				dataURI = akd::base64::decode(uri.substr(uri.find(',')));
				uri = "";
			}

			return Buffer{
				bufferData.atOrDef("name").getStrOrDef(""),
				uri,
				dataURI,
				bufferData["byteLength"].as<int32>()
			};
		}

	}
}



#endif /* AKRES_GLTF_BUFFER_HPP_ */
