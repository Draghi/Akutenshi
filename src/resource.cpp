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

	auto processDirectory = [&](auto path) {
		if (path.isDirectory()) return;
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











