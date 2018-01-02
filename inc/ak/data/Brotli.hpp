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

#include <ak/PrimitiveTypes.hpp>
#include <limits>
#include <vector>

namespace akd {

	std::vector<uint8> compressBrotli(const std::vector<uint8>& inData, uint8 compressionLevel = std::numeric_limits<uint8>::max());

	/**
	 * Decompresses a buffer fill with brotli data
	 * @param inData The buffer with brotli compressed data
	 * @return The uncompressed data
	 */
	std::vector<uint8> decompressBrotli(const std::vector<uint8>& inData);

}
