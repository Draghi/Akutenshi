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

#ifndef AK_SOUND_BACKEND_HPP_
#define AK_SOUND_BACKEND_HPP_

#include <ak/PrimitiveTypes.hpp>
#include <ak/sound/Enums.hpp>
#include <ak/sound/Types.hpp>
#include <functional>
#include <vector>

namespace aks {
	bool init(const DeviceIdentifier& deviceID, uint32 sampleRate, Format format, ChannelMap channelMap, const std::function<upload_callback_f>& callback);
	bool init(const std::vector<Backend>& backends, const DeviceIdentifier& deviceID, uint32 sampleRate, Format format, ChannelMap channelMap, const std::function<upload_callback_f>& callback);

	void startDevice();
	void stopDevice();
	bool isDeviceStarted();

	ContextInfo getContextInfo();

	std::vector<DeviceInfo> getAvailableDevices();
	std::vector<DeviceInfo> getAvailableDevices(Backend backends);

}

#endif /* AK_SOUND_BACKEND_HPP_ */
