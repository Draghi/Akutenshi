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

#ifndef AK_ASSETS_GLTF_IMAGE_HPP_
#define AK_ASSETS_GLTF_IMAGE_HPP_

#include <akasset/gltf/Types.hpp>
#include <akcommon/PrimitiveTypes.hpp>
#include <akengine/data/Base64.hpp>
#include <akengine/data/PValue.hpp>
#include <string>
#include <vector>

namespace aka {
	namespace gltf {

		enum class MimeType {
			undefined,
			jpeg,
			png
		};

		struct Image {
			std::string name;
			std::string uri;
			std::vector<uint8> dataURI;
			MimeType mimeType;
			gltfID bufferView;
		};

		inline Image extractImage(const akd::PValue& imageData) {
			std::string uri = imageData.atOrDef("uri").getStrOrDef("");
			std::vector<uint8> dataURI;
			if (uri.rfind("data:", 0) == 0) { //@todo Test
				dataURI = akd::base64::decode(uri.substr(uri.find(',')));
				uri = "";
			}

			MimeType mimeType;
			std::string mimeTypeStr = imageData.atOrDef("mimeType").getStrOrDef("");
			if (mimeTypeStr == "image/jpeg") mimeType = MimeType::jpeg;
			else if (mimeTypeStr == "image/png") mimeType = MimeType::png;
			else mimeType = MimeType::undefined;

			return Image{
				imageData.atOrDef("name").getStrOrDef(""),
				uri, dataURI, mimeType,
				imageData.atOrDef("bufferView").asOrDef(-1)
			};
		}

	}
}



#endif /* AKRES_GLTF_IMAGE_HPP_ */
