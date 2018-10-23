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

#ifndef AK_TIME_FPSCOUNTER_HPP_
#define AK_TIME_FPSCOUNTER_HPP_

#include <AkCommon/PrimitiveTypes.hpp>
#include <AkCommon/Timer.hpp>
#include <deque>

namespace aku {

	class FPSCounter final {
		private:
			uint64 m_counter = 0;
			Timer m_timer;

			uint64 m_maxSamples;
			std::deque<fpDouble> m_samples;

		public:
			FPSCounter(uint64 maxSamples = 6) : m_maxSamples(maxSamples), m_samples() {}

			FPSCounter& update() {
				m_counter++;
				m_timer.mark();

				if (m_timer.msecs() < 1000) return *this;

				m_samples.push_back((m_counter*1.0e9)/m_timer.nsecs());
				while(m_samples.size() > m_maxSamples) m_samples.pop_front();

				m_timer.reset();
				m_counter = 0;

				return *this;
			}

			fpDouble lastTicksPerSecond() const { return m_samples.empty() ? 0.f :     m_samples.back(); }
			fpDouble lastTickDelta()      const { return m_samples.empty() ? 0.f : 1.f/m_samples.back(); }

			fpDouble avgTicksPerSecond() const {
				if (m_samples.size() == 0) return 0;
				fpDouble totalFPS = 0;
				for(auto iter = m_samples.begin(); iter != m_samples.end(); iter++) totalFPS += *iter;
				return totalFPS/m_samples.size();
			}

			fpDouble avgTickDelta() const {
				auto rate = avgTicksPerSecond();
				return rate == 0 ? 0 : 1.f/rate;
			}
	};

}

#endif
