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

#include <ak/filesystem/Filesystem.hpp>
#include <ak/filesystem/Path.hpp>
#include <akres/Models.hpp>
#include <akres/Textures.hpp>
#include <iostream>
#include <stdexcept>
#include <string>

int akResourceMain();

static void packTexture();
static void packTextureDirectory();

static void packModel();

int akResourceMain() {

	while(true) {
		std::cout << "Welcome to the Akutenshi resource packer. Please select an option:" << std::endl
		          << "1) Pack Texture (Single)" << std::endl
		          << "2) Pack Texture (Directory)" << std::endl
		          << "3) Pack Model (Single)" << std::endl
		          << "Q) Quit" << std::endl;

		std::string inStr;
		std::getline(std::cin, inStr);

		if (inStr.length() != 1) {
			std::cout << "Invalid selection." << std::endl << std::endl;
			continue;
		}

		char cmdSel = inStr.front();

		try {
			switch(cmdSel) {
				default: std::cout << "Invalid selection." << std::endl; break;

				case '1': packTexture(); break;
				case '2': packTextureDirectory(); break;
				case '3': packModel(); break;

				case 'Q': case 'q': return 0;
			}
		} catch(const std::exception& ex) {
			std::cout << "Exception occured. Operation aborted. Message:" << ex.what() << std::endl;
		} catch(...) {
			std::cout << "Exception occured. Operation aborted. No information." << std::endl;
		}

		std::cout << std::endl;
	}
}

// ////////////// //
// // Textures // //
// ////////////// //
static void packTexture() {
		std::cout << std::endl << "Type filename of texture to pack (enter '/\\' to exit): ";

		std::string filenameIn;
		std::getline(std::cin, filenameIn);
		std::cout << std::endl;

		if (filenameIn == "/\\") return;

		akfs::Path filename(filenameIn);

		try {
			if (!akres::doPackTexture(filename.parent(), filename.parent(), filename.filename())) {
				std::cout << "Operation failed. Returning to menu." << std::endl;
			}
		} catch(const std::runtime_error& ex) {
			std::cout << "Operation aborted. Returning to menu. Message: " << ex.what() << std::endl;
		}
}

static void packTextureDirectory() {
	bool recursive;
	while(true) {
		std::cout << std::endl << "Recursively? y/n (enter '/\\' to exit): ";

		std::string inLine;
		std::getline(std::cin, inLine);

		if (inLine == "/\\") return;
		if ((inLine == "Y") || (inLine == "y")) { recursive = true; break; }
		else if ((inLine == "N") || (inLine == "n")) { recursive = false; break; }

		std::cout << std::endl << "Please enter 'Y' or 'N'";
	}

	akfs::Path srcDir;
	while(true) {
		std::cout << "Type directory to pack (enter '/\\' to exit): ";

		std::string directoryIn;
		std::getline(std::cin, directoryIn);

		if (directoryIn == "/\\") return;

		srcDir = directoryIn;

		if (akfs::exists(srcDir)) break;

		std::cout << "Directory does not exist: " << srcDir.str() << std::endl;
	}

	akfs::Path dstDir;
	while(true) {
		std::cout << "Type directory to output packed textures (enter '/\\' to exit): ";

		std::string directoryOut;
		std::getline(std::cin, directoryOut);

		if (directoryOut == "/\\") return;

		dstDir = directoryOut;

		if (akfs::exists(dstDir)) break;
		if (akfs::makeDirectory(dstDir)) break;

		std::cout << "Directory does not exist and could not be created: " << dstDir.str() << std::endl;
	}

	std::cout << std::endl << "Starting directory operation." << std::endl << std::endl;

	auto processDirectory = [&](auto path, auto isDir) {
		if (isDir) return true;
		if (path.extension() == ".json") {
			try {
				std::cout << "Processing: " << path.str() << std::endl;
				akres::doPackTexture(srcDir, dstDir/path.parent().relativeTo(srcDir), path.filename());
			} catch(const std::runtime_error& ex) {
				std::cout << "Operation aborted. Attempting to continue. Message: " << ex.what() << std::endl;
			}
			std::cout << std::endl;
		} else {
			std::cout << "Skipped: " << path.str() << std::endl;
		}
		return true;
	};

	iterateDirectory(srcDir, processDirectory, recursive);
}

// //////////// //
// // Models // //
// //////////// //

static void packModel() {
	std::cout << std::endl << "Type filename of model to pack (enter '/\\' to exit): ";

	std::string filenameIn;
	std::getline(std::cin, filenameIn);
	std::cout << std::endl;

	if (filenameIn == "/\\") return;

	akfs::Path filename(filenameIn);

	try {
		if (!akres::doPackModel(filename.parent(), filename.parent(), filename.filename())) {
			std::cout << "Operation failed. Returning to menu." << std::endl;
		}
	} catch(const std::runtime_error& ex) {
		std::cout << "Operation aborted. Returning to menu. Message: " << ex.what() << std::endl;
	}
}

// //////////// //
// // Models // //
// //////////// //

/*static void testGLTF() {
	std::cout << std::endl << "Type root path of the model to pack (enter '/\\' to exit): ";
	std::string pathIn;
	std::getline(std::cin, pathIn);

	std::cout << "Type name of model to pack (enter '/\\' to exit): ";
	std::string filenameIn;
	std::getline(std::cin, filenameIn);

	std::cout << std::endl;

	try {
		akfs::Path rootPath(pathIn), filename(filenameIn);
		if (!akres::doPackGLTFAsset(rootPath, filename)) {
			std::cout << "Operation failed. Returning to menu." << std::endl;
		}
	} catch(const std::runtime_error& ex) {
		std::cout << "Operation aborted. Returning to menu. Message: " << ex.what() << std::endl;
	}

	std::cout << std::endl;

	if (filenameIn == "/\\") return;

	akfs::Path filename(pathIn + filenameIn);

	try {
		for(auto& mesh : asset.meshes) {
			for(auto& primitive : mesh.primitives) {
				std::vector<uint32> indicies = akres::gltf::extractAccessorData<uint32>   (asset, primitive.indiciesID);
				std::vector<akm::Vec3> positions = akres::gltf::extractAccessorData<akm::Vec3>(asset, primitive.positionID);
				std::vector<akm::Vec3> normals   = akres::gltf::extractAccessorData<akm::Vec3>(asset, primitive.normalID  );

				std::vector<akm::Vec3> tangents;
				std::vector<akm::Vec3> bitangents; {
					std::vector<akm::Vec4> signedTangents  = akres::gltf::extractAccessorData<akm::Vec4>(asset, primitive.tangentID);
					tangents.reserve(signedTangents.size()); bitangents.reserve(signedTangents.size());
					for(akSize i = 0; i < normals.size(); i++) {
						tangents.push_back(akm::Vec3(signedTangents[i]));
						bitangents.push_back(signedTangents[i].w * akm::cross(normals[i], tangents[i]));
					}
				}

				std::vector<std::vector<akm::Vec2>> texCoords; texCoords.reserve(primitive.texCoordIDs.size());
				for(auto texCoordID : primitive.texCoordIDs) texCoords.push_back(akres::gltf::extractAccessorData<akm::Vec2>(asset, texCoordID));

				std::vector<akm::Vec4> colours;
				if (primitive.colourIDs.size() > 0) {
					if (primitive.colourIDs.size() > 1) std::cout << "Mesh Primitive requires multiple colour sets, only one supported." << std::endl;
					if (asset.accessors[primitive.colourIDs[0]].type == akres::gltf::AccessorType::Vec3) {
						std::vector<akm::Vec3> coloursRGB = akres::gltf::extractAccessorData<akm::Vec3>(asset, primitive.colourIDs[0]);
						colours.reserve(coloursRGB.size());
						for(auto& colourRGB : coloursRGB) colours.push_back({colourRGB.r, colourRGB.g, colourRGB.b, 1.0f});
					} else {
						colours = akres::gltf::extractAccessorData<akm::Vec4>(asset, primitive.colourIDs[0]);
					}
				}


				std::vector<akm::Vec4> bones;
				std::vector<akm::Vec4> weights;

				if (primitive.jointIDs.size() > 1) std::cout << "Mesh Primitive requires multiple joint sets, only one supported." << std::endl;
				if (primitive.jointIDs.size() > 0) {

					// Combine weights into map
					std::deque<std::map<uint32, fpSingle>> vertexWeights;
					for(uint32 i = 0; i < primitive.jointIDs.size(); i++) {

						std::vector<akm::Vec4> tmpBones = akres::gltf::extractAccessorData<akm::Vec4>(asset, primitive.jointIDs[0]);
						std::vector<akm::Vec4> tmpWeights = akres::gltf::extractAccessorData<akm::Vec4>(asset, primitive.weightIDs[0]);

						vertexWeights.resize(tmpBones.size());

						for(uint32 j = 0; j < tmpBones.size(); j++) {
							auto& boneWeights = vertexWeights[j];
							for(uint32 k = 0; k < 4; k++) {
								auto boneID = static_cast<uint32>(tmpBones[j][k]);
								auto weightID = tmpWeights[j][k];
								if (weightID == 0) continue;

								auto inResult = boneWeights.emplace(boneID, weightID);
								if (!inResult.second) inResult.first->second += weightID;
							}
						}
					}

					// Find heaviest bone weights for vertex
					for(auto& boneWeights : vertexWeights) {
						std::deque<uint32> selectedBones({0,0,0,0});
						std::deque<fpSingle> selectedWeights({0,0,0,0});
						for(auto boneWeight : boneWeights) {
							bool didInsert = false;
							for(akSize k = 0; k < selectedBones.size(); k++) {
								if (boneWeight.second < selectedWeights[k]) continue;
								didInsert = true;

								selectedBones.insert(selectedBones.begin() + k, boneWeight.first);
								selectedWeights.insert(selectedWeights.begin() + k, boneWeight.second);

								if (selectedWeights.size() > 4) {
									selectedBones.pop_back();
									selectedWeights.pop_back();
								}

								break;
							}

							if ((!didInsert) && (selectedBones.size() < 4)) {
								selectedBones.push_back(boneWeight.first);
								selectedWeights.push_back(boneWeight.second);
							}
						}

						bones.push_back({selectedBones[0], selectedBones[1], selectedBones[2], selectedBones[3]});
						weights.push_back({selectedWeights[0], selectedWeights[1], selectedWeights[2], selectedWeights[3]});+-
						weights.back() = weights.back()/akm::sum(weights.back()); // Normalize bone weights
					}
				}

				std::cout << "i " << indicies.size()
					      << " | p " << positions.size()
					      << " | n " << normals.size() << " | t " << tangents.size() << " | b " << bitangents.size()
					      << " | tc " << texCoords.size() << " | vc " << colours.size()
					      << " | bn " << bones.size() << " | wt " << weights.size()
					      << std::endl;
			}
		}*/

		/*std::cout << "Active Scene: " << asset.activeScene << std::endl;

		int id = 0;
		for(auto& scene : asset.scenes) {
			if (id >= 5) { std::cout << "<Array Trimmed>" << std::endl; break; }
			std::cout << "Scene [" << id++ << ", '" << scene.name << "']" << std::endl;
			std::cout << " - Nodes: ["; for(auto nodeID : scene.nodeIDs) std::cout << nodeID << ", "; std::cout << "]" << std::endl;
		}

		id = 0;
		for(auto& node : asset.nodes) {
			if (id >= 5) { std::cout << "<Array Trimmed>" << std::endl; break; }
			std::cout << "Node [" << id++ << ", '" << node.name << "']" << std::endl;
			std::cout << " -     Mesh: "  << node.meshID     << std::endl;
			std::cout << " -     Skin: "  << node.skinID     << std::endl;
			std::cout << " - Position: [" << node.position.x << ", " << node.position.y << ", " << node.position.z  << "]"  << std::endl;
			std::cout << " - Rotation: [" << node.rotation.x << ", " << node.rotation.y << ", " << node.rotation.z  << ", " << node.rotation.w << "]" << std::endl;
			std::cout << " -    Scale: [" << node.scale.x    << ", " << node.scale.y    << ", " << node.scale.z     << "]"  << std::endl;
			std::cout << " - Children: ["; for(auto childID : node.childrenIDs) std::cout << childID << ", "; std::cout << "]"  << std::endl;
		}

		id = 0;
		for(auto& mesh : asset.meshes) {
			if (id >= 5) { std::cout << "<Array Trimmed>" << std::endl; break; }
			std::cout << "Mesh [" << id++ << ", '" << mesh.name << "']" << std::endl;
			for(auto& primitive : mesh.primitives) {
				std::cout << "Primitive" << std::endl;
				std::cout << " -  Material: " << primitive.materialID << std::endl;
				std::cout << " -      Mode: " << static_cast<uint32>(primitive.mode) << std::endl;
				std::cout << " -   Indices: " << primitive.indiciesID << std::endl;
				std::cout << " -  Position: " << primitive.positionID << std::endl;
				std::cout << " -    Normal: " << primitive.normalID   << std::endl;
				std::cout << " -   Tangent: " << primitive.tangentID  << std::endl;
				std::cout << " - TexCoords ["; for(auto primID : primitive.texCoordIDs) std::cout << primID << ", "; std::cout << "]"  << std::endl;
				std::cout << " -   Colours ["; for(auto primID : primitive.colourIDs)   std::cout << primID << ", "; std::cout << "]"  << std::endl;
				std::cout << " -    Joints ["; for(auto primID : primitive.jointIDs)    std::cout << primID << ", "; std::cout << "]"  << std::endl;
				std::cout << " -   Weights ["; for(auto primID : primitive.weightIDs)   std::cout << primID << ", "; std::cout << "]"  << std::endl;
			}
		}

		id = 0;
		for(auto& skin : asset.skins) {
			if (id >= 5) { std::cout << "<Array Trimmed>" << std::endl; break; }
			std::cout << "Skin [" << id++ << ", '" << skin.name << "']" << std::endl;
			std::cout << " - InverseBindMatricies: "  << skin.inverseBindMatricies << std::endl;
			std::cout << " -           RootNodeID: "  << skin.rootNodeID           << std::endl;
			std::cout << " -         JointNodeIDs: ["; for(auto jointID : skin.jointNodeIDs) std::cout << jointID << ", "; std::cout << "]"  << std::endl;
		}

		id = 0;
		for(auto& material : asset.materials) {
			if (id >= 5) { std::cout << "<Array Trimmed>" << std::endl; break; }
			std::cout << "Material [" << id++ << ", '" << material.name << "']" << std::endl;
			std::cout << " -               BaseFactor: [" << material.baseFactor.x << ", " << material.baseFactor.y << ", " << material.baseFactor.z << ", " << material.baseFactor.w  << "]"  << std::endl;
			std::cout << " -           MetallicFactor: "  << material.metallicFactor << std::endl;
			std::cout << " -          RoughnessFactor: "  << material.roughnessFactor << std::endl;
			std::cout << " -           EmissiveFactor: [" << material.emissiveFactor.x << ", " << material.emissiveFactor.y << ", " << material.emissiveFactor.z  << "]"  << std::endl;
			std::cout << " -              BaseTexture: [" << material.baseTexture.index << ", " << material.baseTexture.texCoordSet << "]"  << std::endl;
			std::cout << " - MetallicRoughnessTexture: [" << material.metallicRoughnessTexture.index << ", " << material.metallicRoughnessTexture.texCoordSet << "]"  << std::endl;
			std::cout << " -            NormalTexture: [" << material.normalTexture.index << ", " << material.normalTexture.texCoordSet << "]"  << std::endl;
			std::cout << " -         OcclusionTexture: [" << material.occlusionTexture.index << ", " << material.occlusionTexture.texCoordSet << "]"  << std::endl;
			std::cout << " -          EmmisiveTexture: [" << material.emissiveTexture.index << ", " << material.emissiveTexture.texCoordSet << "]"  << std::endl;
			std::cout << " -                AlphaMode: "  << static_cast<uint32>(material.alphaMode) << std::endl;
			std::cout << " -              AlphaCutoff: "  << material.alphaCutoff << std::endl;
			std::cout << " -              DoubleSided: "  << (material.doubleSided ? "True" : "False") << std::endl;
		}

		id = 0;
		for(auto& texture : asset.textures) {
			if (id >= 5) { std::cout << "<Array Trimmed>" << std::endl; break; }
			std::cout << "Texture [" << id++ << ", '" << texture.name << "']" << std::endl;
			std::cout << " - SamplerID: "  << texture.samplerID << std::endl;
			std::cout << " -  DourceID: "  << texture.sourceID  << std::endl;
		}

		id = 0;
		for(auto& image : asset.images) {
			if (id >= 5) { std::cout << "<Array Trimmed>" << std::endl; break; }
			std::cout << "Image [" << id++ << ", '" << image.name << "']" << std::endl;
			std::cout << " -        URI: "  << image.uri << std::endl;
			std::cout << " -   URI-Data: "  << image.dataURI.size() << std::endl;
			std::cout << " -   MimeType: "  << static_cast<uint32>(image.mimeType) << std::endl;
			std::cout << " - BufferView: "  << image.bufferView << std::endl;
		}

		id = 0;
		for(auto& sampler : asset.samplers) {
			if (id >= 5) { std::cout << "<Array Trimmed>" << std::endl; break; }
			std::cout << "Sampler [" << id++ << ", '" << sampler.name << "']" << std::endl;
			std::cout << " -  MinFilter: "  << static_cast<uint32>(sampler.minFilter) << std::endl;
			std::cout << " -  MagFilter: "  << static_cast<uint32>(sampler.magFilter) << std::endl;
			std::cout << " -      WrapS: "  << static_cast<uint32>(sampler.wrapS) << std::endl;
			std::cout << " -      WrapT: "  << static_cast<uint32>(sampler.wrapT) << std::endl;
		}

		id = 0;
		for(auto& animation : asset.animations) {
			if (id >= 5) { std::cout << "<Array Trimmed>" << std::endl; break; }
			std::cout << "Animation [" << id++ << ", '" << animation.name << "']" << std::endl;
			std::cout << " - Channels: "  << animation.channels.size() << std::endl;
			std::cout << " - Samplers: "  << animation.samplers.size() << std::endl;
		}

		id = 0;
		for(auto& accessor : asset.accessors) {
			if (id >= 5) { std::cout << "<Array Trimmed>" << std::endl; break; }
			std::cout << "Accessor [" << id++ << ", '" << accessor.name << "']" << std::endl;
			std::cout << " -  BufferViewID: "  << accessor.bufferViewID << std::endl;
			std::cout << " -    ByteOffset: "  << accessor.byteOffset << std::endl;
			std::cout << " - ComponentType: "  << static_cast<uint32>(accessor.componentType) << std::endl;
			std::cout << " -    Normalized: "  << (accessor.normalized ? "true" : "false") << std::endl;
			std::cout << " -         Count: "  << accessor.count << std::endl;
			std::cout << " -          Type: "  << static_cast<uint32>(accessor.type) << std::endl;
			std::cout << " -           Max: [";  for(auto val : accessor.max) std::cout << val << ", "; std::cout << "]"  << std::endl;
			std::cout << " -           Min: [";  for(auto val : accessor.min) std::cout << val << ", "; std::cout << "]"  << std::endl;
			//std::cout << " - SparseAccessor::Count: " << accessor.sparseAccessor.count << std::endl;
			//std::cout << " - SparseAccessor::indicies::bufferViewID: " << accessor.sparseAccessor.indidies.bufferViewID << std::endl;
			//std::cout << " - SparseAccessor::indicies::byteOffset: " << accessor.sparseAccessor.indidies.byteOffset << std::endl;
			//std::cout << " - SparseAccessor::indicies::componentType: " << static_cast<uint32>(accessor.sparseAccessor.indidies.componentType) << std::endl;
			//std::cout << " - SparseAccessor::values::bufferViewID: " << accessor.sparseAccessor.values.bufferViewID << std::endl;
			//std::cout << " - SparseAccessor::values::byteOffset: " << accessor.sparseAccessor.values.byteOffset << std::endl;
		}

		id = 0;
		for(auto& bufferView : asset.bufferViews) {
			if (id >= 5) { std::cout << "<Array Trimmed>" << std::endl; break; }
			std::cout << "BufferView [" << id++ << ", '" << bufferView.name << "']" << std::endl;
			std::cout << " -   BufferID: "  << bufferView.bufferID << std::endl;
			std::cout << " - ByteOffset: "  << bufferView.byteOffset << std::endl;
			std::cout << " -   ByteSize: "  << bufferView.byteSize << std::endl;
			std::cout << " - ByteStride: "  << bufferView.byteStride << std::endl;
			std::cout << " -     Target: "  << static_cast<uint32>(bufferView.target) << std::endl;
		}

		id = 0;
		for(auto& buffer : asset.buffers) {
			if (id >= 5) { std::cout << "<Array Trimmed>" << std::endl; break; }
			std::cout << "Buffer [" << id++ << ", '" << buffer.name << "']" << std::endl;
			std::cout << " -        URI: "  << buffer.uri << std::endl;
			std::cout << " -   Data-URI: "  << buffer.dataURI.size() << std::endl;
			std::cout << " - ByteLength: "  << buffer.byteLength << std::endl;
		}*/

	/*} catch(const std::runtime_error& ex) {
		std::cout << "Operation aborted. Returning to menu. Message: " << ex.what() << std::endl;
	}
}*/










