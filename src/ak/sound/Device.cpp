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

#include <ak/sound/Context.hpp>
#include <ak/sound/Device.hpp>
#include <ak/util/Iterator.hpp>

#include <mini_al.h>
#include <optional>

using namespace aks;

namespace aks {
	namespace internal {
		struct DeviceIdentifier final { mal_device_id id; };
		void DeviceIdentifierDeletetor::operator()(DeviceIdentifier* p) const { delete p; }
	}
}

static std::optional<aks::DeviceFormat> malToFormat(mal_format v) {
	switch(v) {
		case mal_format::mal_format_u8:  return {aks::DeviceFormat::UInt8};
		case mal_format::mal_format_s16: return {aks::DeviceFormat::SInt16};
		case mal_format::mal_format_s24: return {aks::DeviceFormat::SInt24};
		case mal_format::mal_format_s32: return {aks::DeviceFormat::SInt32};
		case mal_format::mal_format_f32: return {aks::DeviceFormat::FPSingle};

		case mal_format::mal_format_count:   [[fallthrough]];
		case mal_format::mal_format_unknown: [[fallthrough]];
		default: return {};
	}
}


std::vector<DeviceInfo> aks::getAvailableDevices() {
	mal_device_info* deviceInfo = nullptr; mal_uint32 deviceCount = 0;
	mal_context_get_devices(static_cast<mal_context*>(aks::internal::getContext()), &deviceInfo, &deviceCount, nullptr, nullptr);

	std::vector<DeviceInfo> result; result.reserve(deviceCount);
	for(mal_uint32 i = 0; i < deviceCount; i++) {
		auto& device = deviceInfo[i];
		result.push_back(DeviceInfo{
			DeviceIdentifier(new internal::DeviceIdentifier{device.id}),
			std::string(device.name),
			aku::convert_to_if<std::vector<aks::DeviceFormat>>(device.formats, device.formats+device.formatCount, malToFormat),
			{device.minChannels, device.maxChannels},
			{device.minSampleRate, device.maxSampleRate}
		});
	}

	return result;
}
