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

#include <ak/time/Timer.hpp>

#ifdef __linux__

#include <bits/types/struct_timespec.h>
#include <sys/time.h>
#include <ctime>
#include <iostream>

using namespace ak::time;

Timer::duration_t Timer::currentTime() {
	using nanoseconds_dur = std::chrono::duration<uint64, nano_t>;
	using seconds_dur = std::chrono::duration<uint64, secs_t>;

	timespec timeRaw;
	clock_gettime(CLOCK_MONOTONIC, &timeRaw);

	auto time = seconds_dur(timeRaw.tv_sec) + nanoseconds_dur(timeRaw.tv_nsec);

	return std::chrono::duration_cast<duration_t>(time);
}

#elif defined(_WIN32)

#include <windows.h>

using namespace ak;

static uint64 getCurrentFrequency() {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;
}

static uint64 getFrequency() {
	static uint64 frequency = getCurrentFrequency();
	return frequency;
}

static uint64 getCount() {
	LARGE_INTEGER counter;
	QueryPerformanceCountthreader(&counter);
	return counter.QuadPart;
}

uint64 getTimeInNanoseconds() {
	return (getCount() / getFrequency()) * 1000000000;
}

Timer::duration_t Timer::currentTime() {
	using nanoseconds_t = std::chrono::duration<uint64, nano_t>;
	return std::chrono::duration_cast<duration_t>(nanoseconds_t(getTimeInNanoseconds()));
}

#else
#	warning "Timer unsupported"
#endif
