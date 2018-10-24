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

#ifndef AK_LOG_LOGGER_HPP_
#define AK_LOG_LOGGER_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/String.hpp>
#include <akcommon/Time.hpp>
#include <akengine/data/PValue.hpp>
#include <akengine/data/SmartEnum.hpp>
#include <akengine/thread/CurrentThread.hpp>
#include <array>
#include <cwchar>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

namespace akl {
	AK_SMART_TENUM_CLASS_KV(Level, uint8,
		None,  0,
		Raw,   1,
		Fatal, 2,
		Error, 3,
		Warn,  4,
		Info,  5,
		Debug, 6
	)

	class Logger final {
		private:
			std::string_view m_name;

		public:
			constexpr Logger(const std::string_view& name);

			template<typename... vargs_t> bool test_fatal(bool cond, const vargs_t&... vargs) const;
			template<typename... vargs_t> bool test_error(bool cond, const vargs_t&... vargs) const;
			template<typename... vargs_t> bool test_warn( bool cond, const vargs_t&... vargs) const;
			template<typename... vargs_t> bool test_info( bool cond, const vargs_t&... vargs) const;
			template<typename... vargs_t> bool test_debug(bool cond, const vargs_t&... vargs) const;
			template<typename... vargs_t> bool test_raw(  bool cond, const vargs_t&... vargs) const;

			template<typename... vargs_t> void fatal(const vargs_t&... vargs) const;
			template<typename... vargs_t> void error(const vargs_t&... vargs) const;
			template<typename... vargs_t> void warn( const vargs_t&... vargs) const;
			template<typename... vargs_t> void info( const vargs_t&... vargs) const;
			template<typename... vargs_t> void debug(const vargs_t&... vargs) const;

			template<typename... vargs_t> void raw(const vargs_t&... vargs) const;

			static constexpr std::array<std::string_view, 7> LevelTags {{
				std::string_view(),
				std::string_view(),
				std::string_view("FATAL", 5),
				std::string_view("ERROR", 5),
				std::string_view("WARN",  4),
				std::string_view("INFO",  4),
				std::string_view("DEBUG", 5)
			}};
	};

	bool startProcessing(uint64 delayUS = 1e3);
	bool stopProcessing();
	bool isProcessing();

	void processMessageQueue();

	bool enableFileOutput();
	void disableFileOutput();

	void setConsoleLevel(Level logLevel);
	bool isConsoleFilterLevelEnabled(Level logLevel);
	Level getConsoleFilterLevel();

	void setFileLevel(Level logLevel);
	bool isFileFilterLevelEnabled(Level logLevel);
	Level getFileFilterLevel();

	bool isFilterLevelEnabled(Level logLevel);

	void captureStandardStreams();
	void restoreStandardStreams();
}

AK_SMART_ENUM_SERIALIZE(akl, Level)

/* ****************** *
 * * Implementation * *
 * ****************** */
namespace akl {
	namespace internal {
		void printMessage(Level logLevel, const std::string& str);

		template<typename... vargs_t> void build(Level level, const std::string_view& logName, const vargs_t&... vargs) {
			auto utc = aku::utcTimestamp();

			std::stringstream sstream;
			sstream << "[" << std::put_time(&utc.ctime, "%H:%M:%S");
			sstream << "." << std::setfill('0') << std::setw(3) << utc.milliseconds;
			sstream  << "][" << akt::current().name() << "][" << logName << "][" << Logger::LevelTags[static_cast<uint8>(level)] << "]";

			std::stringstream vargStream;
			aku::buildString(vargStream, vargs...);

			if (vargStream.str().front() != '[') sstream << " ";

			sstream << vargStream.str() << std::endl;

			printMessage(level, sstream.str());
		}
	}

	constexpr Logger::Logger(const std::string_view& name) : m_name(name) {}

	template<typename... vargs_t> bool Logger::test_fatal(bool cond, const vargs_t&... vargs) const { if (!cond) fatal(vargs...); return cond; }
	template<typename... vargs_t> bool Logger::test_error(bool cond, const vargs_t&... vargs) const { if (!cond) error(vargs...); return cond; }
	template<typename... vargs_t> bool Logger::test_warn( bool cond, const vargs_t&... vargs) const { if (!cond)  warn(vargs...); return cond; }
	template<typename... vargs_t> bool Logger::test_info( bool cond, const vargs_t&... vargs) const { if (!cond)  info(vargs...); return cond; }
	template<typename... vargs_t> bool Logger::test_debug(bool cond, const vargs_t&... vargs) const { if (!cond) debug(vargs...); return cond; }
	template<typename... vargs_t> bool Logger::test_raw(  bool cond, const vargs_t&... vargs) const { if (!cond)   raw(vargs...); return cond; }

	template<typename... vargs_t> void Logger::fatal(const vargs_t&... vargs) const { if (isFilterLevelEnabled(Level::Fatal)) internal::build(Level::Fatal, m_name, vargs...); }
	template<typename... vargs_t> void Logger::error(const vargs_t&... vargs) const { if (isFilterLevelEnabled(Level::Error)) internal::build(Level::Error, m_name, vargs...); }
	template<typename... vargs_t> void Logger::warn( const vargs_t&... vargs) const { if (isFilterLevelEnabled(Level::Warn))  internal::build(Level::Warn,  m_name, vargs...); }
	template<typename... vargs_t> void Logger::info( const vargs_t&... vargs) const { if (isFilterLevelEnabled(Level::Info))  internal::build(Level::Info,  m_name, vargs...); }
	template<typename... vargs_t> void Logger::debug(const vargs_t&... vargs) const { if (isFilterLevelEnabled(Level::Debug)) internal::build(Level::Debug, m_name, vargs...); }

	template<typename... vargs_t> void Logger::raw(const vargs_t&... vargs) const { if (!isFilterLevelEnabled(Level::Raw)) return; internal::printMessage(Level::Raw, aku::buildString(vargs...)); }
}

#endif
