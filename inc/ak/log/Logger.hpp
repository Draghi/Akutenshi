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
#include <ak/time/Time.hpp>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace ak {
	namespace log {

		enum class Level : uint8 {
			FATAL = 0,
			WARN  = 1,
			INFO  = 2,
			DEBUG = 3,
		};

		bool startup(ak::filesystem::CFile file = ak::filesystem::CFile(), Level logLevel = Level::DEBUG);
		void setLogLevel(Level logLevel);
		Level getLogLevel();
		void shutdown();


		class Logger final {
			private:
				static constexpr std::string_view LevelTags[] = {
					std::string_view("FATAL", 5),
					std::string_view("WARN",  4),
					std::string_view("INFO",  4),
					std::string_view("DEBUG", 5)
				};

				std::string_view m_name;

				static void printMessage(const std::string& str);
				static bool isLevelEnabled(Level logLevel);
				static bool isLoggingEnabled();

				template<typename type_t> static void constructMessageOut(std::stringstream& sstream, const typename std::enable_if<!(std::is_invocable<type_t>::value || std::is_function<type_t>::value), type_t>::type& val) { sstream << val; }
				template<typename type_t> static void constructMessageOut(std::stringstream& sstream, const typename std::enable_if<std::is_invocable<type_t>::value || std::is_function<type_t>::value, type_t>::type& val) { sstream << val(); }

				template<typename type_t> static void constructMessage(std::stringstream& sstream, const type_t& val) {
					constructMessageOut<type_t>(sstream, val);
				}

				template<typename type_t, typename... vargs_t> static void constructMessage(std::stringstream& sstream, const type_t& val, const vargs_t&... vargs) {
					constructMessageOut<type_t>(sstream, val);
					constructMessage(sstream, vargs...);
				}

				template<typename... vargs_t> void build(uint8 tag, const vargs_t&... vargs) const {
					if (!isLoggingEnabled()) return;

					auto utc = ak::time::utcTimestamp();

					std::stringstream sstream;
					sstream << "[" << std::put_time(&utc.ctime, "%H:%M:%S");
					sstream << "." << std::setfill('0') << std::setw(3) << utc.milliseconds;
					sstream  << "][" << ak::thread::current().name() << "][" << m_name << "][" << LevelTags[tag] << "] ";

					constructMessage(sstream, vargs...);

					sstream << std::endl;

					printMessage(sstream.str());
				}

			public:
				constexpr Logger(const std::string_view& name) : m_name(name) {}

				template<typename... vargs_t> void fatal(const vargs_t&... vargs) const { if (isLevelEnabled(Level::FATAL)) build(static_cast<uint8>(Level::FATAL), vargs...); }
				template<typename... vargs_t> void warn(const vargs_t&... vargs) const { if (isLevelEnabled(Level::WARN)) build(static_cast<uint8>(Level::WARN), vargs...); }
				template<typename... vargs_t> void info(const vargs_t&... vargs) const { if (isLevelEnabled(Level::INFO)) build(static_cast<uint8>(Level::INFO), vargs...); }
				template<typename... vargs_t> void debug(const vargs_t&... vargs) const { if (isLevelEnabled(Level::DEBUG)) build(static_cast<uint8>(Level::DEBUG), vargs...); }

				template<typename... vargs_t> void raw(const vargs_t&... vargs) const { if (!isLoggingEnabled()) return; std::stringstream sstream; constructMessage(sstream, vargs...); printMessage(sstream.str()); }
		};
	}
}


#endif
