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

#ifndef AK_ENGINE_RESOURCEMANAGER_HPP_
#define AK_ENGINE_RESOURCEMANAGER_HPP_

#include <ak/animation/Fwd.hpp>
#include <ak/animation/Mesh.hpp>
#include <ak/animation/Type.hpp>
#include <ak/animation/Serialize.hpp>
#include <ak/data/Brotli.hpp>
#include <ak/data/Image.hpp>
#include <ak/data/MsgPack.hpp>
#include <ak/data/PValue.hpp>
#include <ak/filesystem/CFile.hpp>
#include <ak/filesystem/Filesystem.hpp>
#include <ak/filesystem/Path.hpp>
#include <ak/Iterator.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/gl/Buffers.hpp>
#include <ak/render/gl/Textures.hpp>
#include <ak/render/gl/VertexArrays.hpp>
#include <ak/String.hpp>
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ake {

	class ResourceManager final {
		public:
			struct Mesh {
				uint32 elementCount;
				akr::gl::VertexArray vertexArray;
				akr::gl::Buffer indexData;
				akr::gl::Buffer vertexData;
			};

			struct AnimMesh {
				uint32 elementCount;
				akr::gl::VertexArray vertexArray;
				akr::gl::Buffer indexData;
				akr::gl::Buffer vertexData;
			};
		private:
			std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;
			std::unordered_map<std::string, std::shared_ptr<akr::gl::Texture>> m_textures;

			template<typename type_t> static type_t readMeshFile(const akfs::Path& filename) {
				auto file = akfs::CFile(filename, akfs::OpenFlags::In);
				if (!file) throw std::runtime_error(ak::buildString("Failed to open mesh: ", filename.str()));

				std::vector<uint8> compressedData;
				compressedData.resize(file.sizeOnDisk());
				if (!file.read(compressedData.data(), compressedData.size())) throw std::runtime_error(ak::buildString("Failed to read mesh: ", filename.str()));
				auto data = akd::decompressBrotli(compressedData);
				akd::PValue dTree;
				if (!akd::fromMsgPack(dTree, data)) throw std::runtime_error(ak::buildString("Failed to parse mesh: ", filename.str()));

				type_t result; // @TODO Remove hack for loading mesh eventually
				if (!akd::deserialize(result, dTree.asObj().begin()->second["mesh"])) throw std::runtime_error(ak::buildString("Failed to deserialize mesh: ", filename.str()));
				return result;
			}

			akd::PValue readTextureFile(const akfs::Path& filename) {
				auto textureFile = akfs::CFile(filename, akfs::OpenFlags::In);
				if (!textureFile) throw std::runtime_error(ak::buildString("Failed to open texture: ", filename.str()));

				std::vector<uint8> textureData;
				if (!textureFile.readAll(textureData)) throw std::runtime_error(ak::buildString("Failed to read texture: ", filename.str()));
				textureData = akd::decompressBrotli(textureData);

				akd::PValue textureConfig;
				if (!akd::fromMsgPack(textureConfig, textureData)) throw std::runtime_error(ak::buildString("Failed to parse texture: ", filename.str()));

				return textureConfig;
			}

		public:
			std::shared_ptr<Mesh> loadUniqueMesh(const akfs::Path& filename) {
				auto mesh = readMeshFile<aka::Mesh>(filename);
				std::shared_ptr<Mesh> result(new Mesh());

				result->vertexArray.enableVAttribs({0, 1, 2, 3, 4});
				result->vertexArray.setVAttribFormats({0, 1, 2, 3}, 3, akr::gl::DataType::Single);
				result->vertexArray.setVAttribFormat(4, 2, akr::gl::DataType::Single);

				result->vertexData = akr::gl::Buffer(mesh.vertexData().data(), mesh.vertexData().size()*sizeof(aka::VertexData));
				result->vertexArray.bindVertexBuffer(0, result->vertexData, sizeof(aka::VertexData), offsetof(aka::VertexData, position));
				result->vertexArray.bindVertexBuffer(1, result->vertexData, sizeof(aka::VertexData), offsetof(aka::VertexData, tangent));
				result->vertexArray.bindVertexBuffer(2, result->vertexData, sizeof(aka::VertexData), offsetof(aka::VertexData, bitangent));
				result->vertexArray.bindVertexBuffer(3, result->vertexData, sizeof(aka::VertexData), offsetof(aka::VertexData, normal));
				result->vertexArray.bindVertexBuffer(4, result->vertexData, sizeof(aka::VertexData), offsetof(aka::VertexData, texCoord));

				result->indexData = akr::gl::Buffer(mesh.indexData().data(), mesh.indexData().size()*sizeof(aka::IndexData));
				result->vertexArray.bindIndexBuffer(result->indexData);

				result->elementCount = mesh.indexData().size()*3;

				return result;
			}

			std::shared_ptr<akr::gl::Texture> loadUniqueTexture(const akfs::Path& filename) {
				akd::PValue textureConfig = readTextureFile(filename);

				std::shared_ptr<akr::gl::Texture> texResult;
				akr::gl::TexTarget texTarget = akr::gl::TexTarget::Tex1D;

				std::string textureType = textureConfig["type"].asStr();
				if (textureConfig["hdr"].asBool()) {
					akd::ImageF32 image;
					if (!akd::deserialize(textureConfig["image"], image)) throw std::runtime_error(ak::buildString("Failed to deserialize texture, invalid data: ", filename.str()));
						 if (textureType == "2D")      { texResult = std::make_shared<akr::gl::Texture>(     *akr::gl::createTex2D(0, akr::gl::TexStorage::Single, image)); texTarget = akr::gl::TexTarget::Tex2D; }
					else if (textureType == "cubemap") { texResult = std::make_shared<akr::gl::Texture>(*akr::gl::createTexCubemap(0, akr::gl::TexStorage::Single, image)); texTarget = akr::gl::TexTarget::TexCubemap; }
				} else {
					akd::ImageU8 image;
					if (!akd::deserialize(textureConfig["image"], image)) throw std::runtime_error(ak::buildString("Failed to deserialize texture, invalid data: ", filename.str()));
					     if (textureType == "2D")      { texResult = std::make_shared<akr::gl::Texture>(     *akr::gl::createTex2D(0, akr::gl::TexStorage::Byte, image)); texTarget = akr::gl::TexTarget::Tex2D; }
					else if (textureType == "cubemap") { texResult = std::make_shared<akr::gl::Texture>(*akr::gl::createTexCubemap(0, akr::gl::TexStorage::Byte, image)); texTarget = akr::gl::TexTarget::TexCubemap; }
				}

				if (!texResult) throw std::runtime_error(ak::buildString("Failed to deserialize texture, invalid type: ", filename.str()));

				akr::gl::setTexFilters(texTarget, akr::gl::FilterType::Linear, akr::gl::MipFilterType::Linear, akr::gl::FilterType::Linear);
				akr::gl::genTexMipmaps(texTarget);

				return texResult;
			}

			std::shared_ptr<Mesh> loadMesh(const akfs::Path& filename) {
				auto filePath = akfs::toSystemPath(akfs::Path(filename));
				auto iter = m_meshes.find(filePath);
				if (iter != m_meshes.end()) return iter->second;
				return m_meshes.emplace(filePath, loadUniqueMesh(filename)).first->second;
			}

			std::shared_ptr<akr::gl::Texture> loadTexture(const std::string& filename) {
				auto filePath = akfs::toSystemPath(akfs::Path(filename));
				auto iter = m_textures.find(filePath);
				if (iter != m_textures.end()) return iter->second;
				return m_textures.emplace(filePath, loadUniqueTexture(filename)).first->second;
			}

			void freeUnused() {
				ak::erase_if(m_meshes, [](const auto& iter){ return iter->second.unique(); });
				ak::erase_if(m_textures, [](const auto& iter){ return iter->second.unique(); });
			}
	};

}



#endif /* AK_ENGINE_RESOURCEMANAGER_HPP_ */
