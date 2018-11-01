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

#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/ScopeGuard.hpp>
#include <akengine/data/Brotli.hpp>
#include <brotli/decode.h>
#include <brotli/encode.h>
#include <crtdefs.h>
#include <stdexcept>
#include <vector>

using namespace akd;

std::vector<uint8> akd::compressBrotli(const std::vector<uint8>& inData, uint8 compressionLevel) {
	BrotliEncoderState* state = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
	auto destroyBrotliInstance = ak::ScopeGuard([&]{BrotliEncoderDestroyInstance(state);});
	BrotliEncoderSetParameter(state, BROTLI_PARAM_QUALITY, std::min<uint8>(compressionLevel, BROTLI_MAX_QUALITY));
	BrotliEncoderSetParameter(state, BROTLI_PARAM_LGWIN,   BROTLI_MAX_WINDOW_BITS);
	BrotliEncoderSetParameter(state, BROTLI_PARAM_LGBLOCK, BROTLI_MAX_INPUT_BLOCK_BITS);
	BrotliEncoderSetParameter(state, BROTLI_PARAM_LGBLOCK, BROTLI_MAX_INPUT_BLOCK_BITS);

	std::vector<uint8> outData;
	outData.resize(BrotliEncoderMaxCompressedSize(inData.size()), 0);

	size_t inAvailable = inData.size();
	const uint8_t* inNext = inData.data();
	size_t outAvailable = outData.size();
	uint8* outNext = outData.data();

	while(true) {
		if (!BrotliEncoderCompressStream(state, BROTLI_OPERATION_FINISH, &inAvailable, &inNext, &outAvailable, &outNext, nullptr)) throw std::runtime_error("Failed to perform compress with brotli");
		if (BrotliEncoderIsFinished(state)) break;
	}

	outData.resize(outData.size() - outAvailable);
	return outData;
}

std::vector<uint8> akd::decompressBrotli(const std::vector<uint8>& inData) {
	BrotliDecoderState* state = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
	auto destroyBrotliInstance = ak::ScopeGuard([&]{BrotliDecoderDestroyInstance(state);});

	constexpr size_t bufferGrow = 4096;
	std::vector<uint8> buffer;
	buffer.resize(bufferGrow);

	size_t remIn = inData.size();
	const uint8* nextIn = inData.data();

	uint8* nextOut = buffer.data();
	size_t remOut = buffer.size();

	BrotliDecoderResult lastResult = BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;
	while(true) {
		lastResult = BrotliDecoderDecompressStream(state, &remIn, &nextIn, &remOut, &nextOut, nullptr);
		switch(lastResult) {
			case BROTLI_DECODER_RESULT_SUCCESS: return buffer;
			case BROTLI_DECODER_RESULT_ERROR: throw std::runtime_error("Error decoding brotli file");
			case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT: throw std::runtime_error("Incomplete brotli file");

			case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT: {
				ptrdiff_t offset = nextOut - buffer.data();
				buffer.resize(buffer.size() + bufferGrow, 0);
				remOut += bufferGrow;
				nextOut = buffer.data() + offset;
			} break;
		}
	}
}



