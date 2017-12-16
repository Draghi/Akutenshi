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

#include "ak/util/Time.hpp"
#include <chrono>

using namespace aku;

Timestamp aku::utcTimestamp() {
	auto cTime = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(cTime);
	auto utcTime = std::gmtime(&time);
	return {*utcTime, std::chrono::duration_cast<std::chrono::milliseconds>(cTime.time_since_epoch()).count() % 1000};
}

Timestamp aku::localTimestamp() {
	auto cTime = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(cTime);
	auto utcTime = std::localtime(&time);
	return {*utcTime, std::chrono::duration_cast<std::chrono::milliseconds>(cTime.time_since_epoch()).count() % 1000};
}
