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

#ifndef AK_MATH_SERIALIZE_HPP_
#define AK_MATH_SERIALIZE_HPP_

#include <akengine/data/Serialize.hpp>
#include <akengine/data/SmartClass.hpp>
#include <akmath/Types.hpp>

AK_SMART_CLASS_ARRAY(akm::Vec2, akm::Vec2::length());
AK_SMART_CLASS_ARRAY(akm::Vec3, akm::Vec3::length());
AK_SMART_CLASS_ARRAY(akm::Vec4, akm::Vec4::length());

AK_SMART_CLASS_ARRAY(akm::Mat2, akm::Mat2::length());
AK_SMART_CLASS_ARRAY(akm::Mat3, akm::Mat3::length());
AK_SMART_CLASS_ARRAY(akm::Mat4, akm::Mat4::length());

AK_SMART_CLASS_ARRAY(akm::Quat, akm::Quat::length());

#endif
