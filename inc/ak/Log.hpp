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

#ifndef AK_LOG_LOGGER_HPP_
#define AK_LOG_LOGGER_HPP_

#include <ak/filesystem/CFile.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/thread/CurrentThread.hpp>
#include <ak/thread/Thread.hpp>
#include <ak/util/Time.hpp>
#include <ak/String.hpp>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace ak {
	namespace log {

		/**
		 * The level to filter the log at
		 */
		enum class Level : uint8 {
			NONE  = 0,                                //!< NONE Don't log anything, you won't get any info from the program at all. Not reccomended.

			RAW   = 1,                                //!< RAW Raw output, unfiltered unless the log level is set to none. Doesn't include level, tag, thread or logger name.
			FATAL = 2,                                //!< FATAL An unrecoverable error has occurred, the program will likely be terminated after this is received.
			WARN  = 3,                                //!< WARN A recoverable/expected error has occurred and steps have been taken to mitigate it's effect.
			INFO  = 4,                                //!< INFO General information, useful for general debugging in the case of a crash.
			DEBUG = 5,                                //!< DEBUG Debug specific information. Information that is only useful for developers, generally should be disabled in user configs.

			ALL   = std::numeric_limits<uint8>::max() //!< ALL Log everything. Practically the same as setting the log level to debug, except that it will accounts for adding new levels.
		};

		bool startProcessing(uint64 delayUS = 1e3);
		bool stopProcessing();
		bool isProcessing();

		void processMessageQueue();

		bool enableFileOutput();
		void disableFileOutput();

		void setConsoleFilterLevel(Level logLevel);
		bool isConsoleFilterLevelEnabled(Level logLevel);
		Level getConsoleFilterLevel();

		void setFileFilterLevel(Level logLevel);
		bool isFileFilterLevelEnabled(Level logLevel);
		Level getFileFilterLevel();

		bool isFilterLevelEnabled(Level logLevel);

		void captureStandardStreams();
		void restoreStandardStreams();

		class Logger final {
			private:
				static constexpr std::string_view LevelTags[] = {
					std::string_view(),
					std::string_view(),
					std::string_view("FATAL", 5),
					std::string_view("WARN",  4),
					std::string_view("INFO",  4),
					std::string_view("DEBUG", 5)
				};

				std::string_view m_name;

				static void printMessage(Level logLevel, const std::string& str);

				template<typename... vargs_t> void build(Level level, const vargs_t&... vargs) const {
					auto utc = ak::util::utcTimestamp();

					std::stringstream sstream;
					sstream << "[" << std::put_time(&utc.ctime, "%H:%M:%S");
					sstream << "." << std::setfill('0') << std::setw(3) << utc.milliseconds;
					sstream  << "][" << ak::thread::current().name() << "][" << m_name << "][" << LevelTags[static_cast<uint8>(level)] << "] ";

					ak::buildString(sstream, vargs...);

					sstream << std::endl;

					printMessage(level, sstream.str());
				}

			public:
				constexpr Logger(const std::string_view& name) : m_name(name) {}

				template<typename... vargs_t> void fatal(const vargs_t&... vargs) const { if (isFilterLevelEnabled(Level::FATAL)) build(Level::FATAL, vargs...); }
				template<typename... vargs_t> void warn(const vargs_t&... vargs) const { if (isFilterLevelEnabled(Level::WARN))   build(Level::WARN, vargs...); }
				template<typename... vargs_t> void info(const vargs_t&... vargs) const { if (isFilterLevelEnabled(Level::INFO))   build(Level::INFO, vargs...); }
				template<typename... vargs_t> void debug(const vargs_t&... vargs) const { if (isFilterLevelEnabled(Level::DEBUG)) build(Level::DEBUG, vargs...); }

				template<typename... vargs_t> void raw(const vargs_t&... vargs) const { if (!isFilterLevelEnabled(Level::RAW)) return; printMessage(Level::RAW, ak::buildString(vargs...)); }
		};
	}
}

#if not(defined(AK_NAMESPACE_ALIAS_DISABLE) || defined(AK_LOG_ALIAS_DISABLE))
namespace akl = ak::log;
#endif

#endif
