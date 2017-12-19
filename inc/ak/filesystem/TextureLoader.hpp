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

/*	std::pair<fpSingle, akr::Texture> load1DTexture(SystemFolder folder, const stx::filesystem::path& path);
	std::pair<akm::Vec2, akr::Texture> load2DTexture(SystemFolder folder, const stx::filesystem::path& path);
	std::pair<akm::Vec3, akr::Texture> load3DTexture(SystemFolder folder, const stx::filesystem::path& path);*/

	akr::Texture loadTexture(SystemFolder folder, const stx::filesystem::path& path);

}

#endif
