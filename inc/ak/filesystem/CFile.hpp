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

#ifndef AK_FILESYSTEM_CFILE_HPP_
#define AK_FILESYSTEM_CFILE_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <stx/Filesystem.hpp>
#include <stddef.h>
#include <sys/types.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <vector>

namespace ak {
	namespace filesystem {

		/**
		 * The bitwise-or-able OpenFlags
		 */
		enum OpenFlags : uint8 {
			None = 0x00,     //!< No OpenFlags

			In    = 0x01,    //!< In Open stream for input
			Out   = 0x02,    //!< Out Open stream for output
			InOut = In | Out,//!< InOut Shortcut for In | Out

			NoCreate = 0x04, //!< Do not create a file if it does not exist (Requires Out flag)
			Truncate = 0x08, //!< Truncate Should the file be truncated when opened? (Requires Out flag)
			Append   = 0x10, //!< Append Should the file be opened in append mode? (Out is mutually exclusive)
			Text     = 0x20  //!< Text Should the file be opened in text mode?
		};

		/**
		 * The seek direction
		 */
		enum SeekDir : int {
			Start = SEEK_SET,   //!< Start
			Relative = SEEK_CUR,//!< Relative
			End = SEEK_END      //!< End
		};

		class CFile final {
			CFile(const CFile&) = delete;
			CFile& operator=(const CFile&) = delete;
			private:
				FILE* m_handle;
				uint8 m_openFlags;
				stx::filesystem::path m_path;

				/**
				 * Converts the constructed open flags to the equivalent fopen flags
				 * @param openFlags The flags to convert
				 * @return The given flags converted to the fopen flags
				 */
				static std::string openFlagsToCOpenFlags(uint8 openFlags);

			public:
				/**
				 * Constructs a closed file handle
				 */
				CFile();

				/**
				 * Attempts to open the given path with the given flags
				 * @param path The path to open
				 * @param openFlags The OpenFlags flags (bitwise-or'd) to open the file with
				 */
				CFile(const stx::filesystem::path& path, uint8 openFlags);

				/**
				 * Move constructs a file
				 * @param other The file to move
				 */
				CFile(CFile&& other);

				/**
				 * Closes the file on destruction
				 */
				~CFile();

				/**
				 * Move assigns using the given file, closing the currently stored file if neccesary
				 * @param other The other file to move
				 * @return This object
				 */
				CFile& operator=(CFile&& other);

				/**
				 * Attempts to read *count* number of *type_t* sized blocks of data into the dest array
				 * @param dest The destination array for the read data, must contain at least count * sizeof(type_t) entries
				 * @param count The number of blocks of data to read
				 * @tparam type_t The type of data to read, must be a POD type
				 * @return The number of complete entries read
				 */
				template<typename type_t> size_t read(type_t* dest, size_t count) {
					static_assert(std::is_pod<type_t>(), "CFile::read - type_t must be a POD type");
					return isOpen() ? std::fread(dest, sizeof(type_t), count, m_handle) : 0;
				}

				/**
				 * Attempts to read a sized block of data into the dest variable
				 * @param dest The destination variable
				 * @tparam type_t The type of data to read, must be a POD type
				 * @return The number of complete entries read
				 */
				template<typename type_t> size_t read(type_t& dest) {
					return read(&dest, 1);
				}

				/**
				 * Attempts to read a delimited line
				 * @param dest The location to store the string in
				 * @param whitespaceSeperator If whitespace (std::isspace with "C" locale) should be treated as a seperator
				 * @param seperators The other strings to seperate on
				 * @return The number of characters read
				 * @remarks EOF is always treated as EOL
				 * @remarks Does not include the seperator in the dest string, however, the return count includes it to differentiate a failed read from a zero-length delimiter
				 */
				size_t readLine(std::string& dest, bool whitespaceSeperator = true, std::vector<std::string> seperators = {"\r\n", "\r", "\n"});

				/**
				 * Attempts to write *count* entries in given src array as as type_t sized blocks of data
				 * @param src The source array, must contain atleast count * sizeof(type_t) entries
				 * @param count The number of entries to write
				 * @tparam type_t The type of data to write, must be a POD type
				 * @return The number of complete entries written
				 */
				template<typename type_t> size_t write(const type_t* src, size_t count) {
					static_assert(std::is_pod<type_t>(), "CFile::write - type_t must be a POD type");
					return isOpen() ? std::fwrite(src, sizeof(type_t), count, m_handle) : 0;
				}

				/**
				 * Attempts to the given src value as as type_t sized block of data
				 * @param src The source value
				 * @tparam type_t The type of data to write, must be a POD type
				 * @return The number of complete entries written
				 */
				template<typename type_t> size_t write(const type_t& src) {
					return write(&src, 1);
				}

				/**
				 * Attempts to write a single string with the given delimiter [default=""]
				 * @param line The line to write
				 * @param lineEnd The delimiter to use [default=""]
				 * @return The number of characters written
				 */
				size_t writeLine(const std::string& line, const std::string& lineEnd = "");


				/**
				 * Attempts to write an array of strings with the given delimiter [default=""]
				 * @param lines The lines to write, must contain *lineCount* number of lines
				 * @param lineCount the number of lines to write
				 * @param lineEnd The delimiter to use [default=""]
				 * @return The number of complete lines written
				 */
				size_t writeLines(const std::string* lines, size_t lineCount, const std::string& lineEnd = "");

				// //////////// //
				// // Stream // //
				// //////////// //

				/**
				 * Returns the position of the steam relative to the beginning
				 * @return The position of the steam relative to the beggining
				 * @remarks A value less than 0 signifies an error
				 */
				ssize_t tell() {
					return isOpen() ? std::ftell(m_handle) : -1;
				}

				/**
				 * Attempts to seek to a certain position in a file
				 * @param offset The offset to seek to
				 * @param seekDir The position to seek relative to
				 * @return If the seek operation was successful
				 */
				bool seek(ssize_t offset, SeekDir seekDir) {
					return isOpen() && (std::fseek(m_handle, offset, seekDir) == 0);
				}

				/**
				 * Attempts to flush any buffered output
				 * @return If the flush was successful
				 * @remark Behaviour is undefined if the last action wasn't a write operation
				 */
				bool flush() {
					return isOpen() && (std::fflush(m_handle) != EOF);
				}

				/**
				 * Returns if the EOF flag has been set on the stream
				 * @return If the EOF flag has been set on the stream
				 */
				bool eof() const {
					return isOpen() && (std::feof(m_handle) != 0);
				}

				/**
				 * Returns the error code on the stream
				 * @return The current error code
				 * @remarks An error code of 0 means that there is no error to report
				 */
				int error() const {
					return isOpen() ? std::ferror(m_handle) : 0;
				}

				/**
				 * Returns the size of the file on disk, in bytes
				 * @return The size of the file on disk
				 */
				int64_t sizeOnDisk() const;

				/**
				 * Returns the path of the file that was opened
				 * @return The path of the file that was opened
				 */
				stx::filesystem::path path() const {
					return m_path;
				}

				/**
				 * Returns the flags used to open the file with
				 * @return The flags used to open the file
				 */
				uint8 openFlags() const {
					return m_openFlags;
				}

				/**
				 * Returns if the file is open
				 * @return If the file is open
				 * @remarks Doesn't return if the file handle is still valid
				 */
				bool isOpen() const {
					return m_handle != nullptr;
				}

				/**
				 * Returns if the file is open
				 * @return If the file is open
				 * @remarks Doesn't return if the file handle is still valid
				 */
				operator bool() const {
					return isOpen();
				}
		};

		inline CFile open(const stx::filesystem::path& path, OpenFlags flags) {
			return CFile(path, flags);
		}

	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_FILESYSTEM_ALIAS_DISABLE))
namespace akfs = ak::filesystem;
#endif

#endif
