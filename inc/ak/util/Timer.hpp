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

#ifndef AK_TIMER_HPP_
#define AK_TIMER_HPP_

#include <chrono>
#include <ratio>

#include <ak/PrimitiveTypes.hpp>

namespace aku {
	class Timer final {
		private:
			using nano_t  = std::nano;
			using micro_t = std::micro;
			using milli_t = std::milli;
			using secs_t  = std::ratio<1>;
			using mins_t  = std::ratio<60>;
			using hours_t = std::ratio<3600>;
			using days_t  = std::ratio<3600*24>;

			/// The internal duration storage type
			using duration_t = std::chrono::duration<uint64, nano_t>;

			/// The start of the time period
			duration_t m_start;

			/// The end of the time period
			duration_t m_end;

		public:
			/**
			 * Constructs new timer, where the time period represents the instant it was constructed
			 */
			Timer() : m_start(currentTime()), m_end(m_start) {}

			/**
			 * Copy constructs a new timer, keeping the start/end time.
			 * @param other The timer to copy
			 */
			Timer(Timer& other) : m_start(other.m_start), m_end(other.m_end) {}

			/**
			 * Sets the start and of of the time period to now, resetting the timer
			 * @return This timer for chaining
			 */
			Timer& reset() {
				m_start = currentTime();
				m_end = m_start;
				return *this;
			}

			/**
			 * Sets the end of the timed period to the current time, defining an elapsed time
			 * @return This timer for chaining
			 */
			Timer& mark() {
				m_end = currentTime();
				return *this;
			}

			/**
			 * Calls the mark function, copies the timer, resets and returns the clone.
			 * @return The cloned timer
			 */
			Timer markAndReset() {
				auto result = Timer(mark());
				reset();
				return result;
			}

			/**
			 * Converts the timed period from seconds into the given unit
			 * @tparam ratio_t The unit of time to convert to
			 * @return The elapsed period of time in the given period
			 * @remark Make sure to call mark if you want to update the end of the time period
			 */
			template<typename ratio_t> uint64 duration() {
				using duration_type = std::chrono::duration<uint64, ratio_t>;
				return std::chrono::duration_cast<duration_type>(m_end - m_start).count();
			}

			fpSingle secsf()  { return (nano_t::num * static_cast<fpSingle>(nsecs()))/static_cast<fpSingle>(nano_t::den); }

			/**
			 * Returns the duration in nanoseconds
			 * @return The duration in nanoseconds
			 */
			uint64 nsecs() { return duration<nano_t>(); }

			/**
			 * Returns the duration in microseconds
			 * @return The duration in microseconds
			 */
			uint64 usecs() { return duration<micro_t>(); }

			/**
			 * Returns the duration in milliseconds
			 * @return The duration in milliseconds
			 */
			uint64 msecs() { return duration<milli_t>(); }

			/**
			 * Returns the duration in seconds
			 * @return The duration in seconds
			 */
			uint64 secs()  { return duration<secs_t>(); }

			/**
			 * Returns the duration in minutes
			 * @return The duration in minutes
			 */
			uint64 mins()  { return duration<mins_t>(); }

			/**
			 * Returns the duration in hours
			 * @return The duration in hours
			 */
			uint64 hours() { return duration<hours_t>(); }

			/**
			 * Returns the duration in days
			 * @return The duration in days
			 */
			uint64 days()  { return duration<days_t>(); }

			/**
			 * Returns the current time as the duration storage type
			 * @return The current time as the duration storage type
			 */
			static duration_t currentTime();
	};
}

#endif
