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

#ifndef AK_TIME_FPSCOUNTER_HPP_
#define AK_TIME_FPSCOUNTER_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/time/Timer.hpp>
#include <deque>

namespace ak {
	namespace time {

		class FPSCounter final {
			private:
				uint64 m_counter = 0;
				Timer m_timer;

				uint64 m_maxSamples;
				std::deque<fpDouble> m_samples;

			public:
				FPSCounter(uint64 maxSamples = 6) : m_maxSamples(maxSamples), m_samples() {}

				void update() {
					m_counter++;
					m_timer.mark();

					if (m_timer.msecs() < 1000) return;

					while(m_samples.size() >= m_maxSamples) m_samples.pop_front();
					m_samples.push_back((m_counter*1.0e9)/m_timer.nsecs());

					m_timer.reset();
					m_counter = 0;
				}

				fpDouble fps() {
					if (m_samples.size() == 0) return 0;
					fpDouble totalFPS = 0;
					for(auto iter = m_samples.begin(); iter != m_samples.end(); iter++) totalFPS += *iter;
					return totalFPS/m_samples.size();
				}
		};

	}
}

#endif
