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

#include "ak/filesystem/CFile.hpp"

using namespace akfs;

std::string CFile::openFlagsToCOpenFlags(uint8 openFlags) {
	bool hasInFlag       = (openFlags & OpenFlags::In      ) == OpenFlags::In;
	bool hasOutFlag      = (openFlags & OpenFlags::Out     ) == OpenFlags::Out;
	bool hasInOutFlag    = (openFlags & OpenFlags::InOut   ) == OpenFlags::InOut;
	bool hasTruncateFlag = (openFlags & OpenFlags::Truncate) == OpenFlags::Truncate;
	bool hasAppendFlag   = (openFlags & OpenFlags::Append  ) == OpenFlags::Append;
	bool hasTextFlag     = (openFlags & OpenFlags::Text    ) == OpenFlags::Text;

	std::string cFlags;

	if (hasAppendFlag && hasOutFlag) return "";
	if (!hasOutFlag && hasTruncateFlag) return "";

	if (hasAppendFlag)     cFlags = hasInFlag ? "a+" : "a";
	else if (hasInOutFlag) cFlags = hasTruncateFlag ? "w+" : "r+";
	else if (hasOutFlag)   cFlags = hasTruncateFlag ? "w" : "wx";
	else if (hasInFlag)    cFlags = "r";
	else return "";

	if (!hasTextFlag) cFlags += "b";

	return cFlags;
}

CFile::CFile() : m_handle(nullptr), m_openFlags(None), m_path("") {}

CFile::CFile(const stx::filesystem::path& path, uint8 openFlags) : m_handle(nullptr), m_openFlags(openFlags), m_path(stx::filesystem::absolute(path)) {
	bool hasNoCreateFlag = (openFlags & OpenFlags::NoCreate) == OpenFlags::NoCreate;
	bool hasTruncateFlag = (openFlags & OpenFlags::Truncate) == OpenFlags::Truncate;
	bool hasOutFlag      = (openFlags & OpenFlags::Out     ) == OpenFlags::Out;

	if (hasOutFlag && hasNoCreateFlag && hasTruncateFlag) return;
	if (hasNoCreateFlag && !stx::filesystem::exists(m_path)) return;

	/// @HACK Race Condition - Filesystem
	/// Should only cause issues in rare circumstances (IE. another thread/process creates and writes to a file by the time we call fopen)
	/// Spitball repercussions: dataloss, a failed fopen or invalid file pointer in another thread/program.
	/// This is to support opening a file for non-appending write (+read) without truncating
	if (hasOutFlag && !hasTruncateFlag && !stx::filesystem::exists(m_path)) openFlags |= OpenFlags::Truncate;

	auto cFlags = openFlagsToCOpenFlags(openFlags);
	if (cFlags.empty()) throw std::invalid_argument("CFile::CFile - openFlags parameter is invalid.");

	std::error_code errorCode;
	if (hasOutFlag && !hasNoCreateFlag) stx::filesystem::create_directories(stx::filesystem::path(path).remove_filename(), errorCode);

	m_handle = std::fopen(m_path.string().c_str(), cFlags.c_str());
}

CFile::CFile(CFile&& other) : m_handle(std::move(other.m_handle)), m_openFlags(std::move(other.m_openFlags)), m_path(std::move(other.m_path)) {
	other.m_handle = nullptr;
}

CFile& CFile::operator=(CFile&& other) {
	if (isOpen()) std::fclose(m_handle);
	m_handle = std::move(other.m_handle);
	other.m_handle = nullptr;

	m_openFlags = std::move(other.m_openFlags);
	m_path = std::move(other.m_path);

	return *this;
}

CFile::~CFile() {
	if (isOpen()) std::fclose(m_handle);
}

size_t CFile::readLine(std::string& dest, bool whitespaceSeperator, std::vector<std::string> seperators) {
	std::stringstream sstream;

	constexpr size_t BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];

	while(true) {
		auto cPos = tell();

		size_t readCount = read(buffer, BUFFER_SIZE);
		if (readCount <= 0) {
			dest = sstream.str();
			return dest.size();
		}

		for(size_t i = 0; i < readCount; i++) {
			if ((whitespaceSeperator) && (std::isspace(buffer[i], std::locale("C")))) {
				sstream << std::string_view(buffer, i);
				seek(cPos + i + 1, SeekDir::Start);
				dest = sstream.str();
				return dest.size() + 1;
			}

			for(size_t j = 0; j < seperators.size(); j++) {
				if (i + seperators[j].size() >= readCount) continue;
				if (std::string_view(buffer, seperators[j].size()) == seperators[j]) {
					sstream << std::string_view(buffer, i);
					seek(cPos + i + seperators[j].size(), SeekDir::Start);
					dest = sstream.str();
					return dest.size() + seperators[j].size();
				}
			}
		}

		sstream << std::string_view(buffer, readCount);
	}
}

size_t CFile::writeLine(const std::string& line, const std::string& lineEnd) {
	auto countWritten = write(line.c_str(), line.size());
	if (countWritten < line.size()) return countWritten;
	return countWritten + (lineEnd.size() > 0 ? write(lineEnd.c_str(), lineEnd.size()) : 0);
}

size_t CFile::writeLines(const std::string* lines, size_t lineCount, const std::string& lineEnd) {
	for(size_t i = 0; i < lineCount; i++) {
		if (writeLine(lines[i], lineEnd) < (lines[i].size() + lineEnd.size())) return i;
	}
	return lineCount;
}

size_t CFile::sizeOnDisk() const {
	std::error_code error_code;
	auto size = stx::filesystem::file_size(m_path, error_code);
	return error_code ? 0 : size;
}
