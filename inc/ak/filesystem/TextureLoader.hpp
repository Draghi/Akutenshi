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

#ifndef AK_FILESYSTEM_TEXTURELOADER_HPP_
#define AK_FILESYSTEM_TEXTURELOADER_HPP_

#include <ak/filesystem/Filesystem.hpp>
#include <ak/render/Texture.hpp>

namespace akfs {

	struct TexLoadResult {
		akr::Texture texture;

		size_t width;
		size_t height;
		size_t depth;
		size_t layers;

		size_t components;
	};

	TexLoadResult loadTexture1D(SystemFolder folder, const stx::filesystem::path& path);
	TexLoadResult loadTextureArray1D(SystemFolder folder, const stx::filesystem::path& path);

	TexLoadResult loadTexture2D(SystemFolder folder, const stx::filesystem::path& path);
	TexLoadResult loadTextureArray2D(SystemFolder folder, const stx::filesystem::path& path);

	TexLoadResult loadTexture3D(SystemFolder folder, const stx::filesystem::path& path);

	TexLoadResult loadTextureCubemap(SystemFolder folder, const stx::filesystem::path& path);

}

#endif
