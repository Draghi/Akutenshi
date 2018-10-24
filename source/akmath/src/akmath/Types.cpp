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

#include <akmath/Types.hpp>
#include <type_traits>

static_assert(std::is_same<akm::Vec2::value_type, akm::scalar_t>(), "GLM not using scalar type.");
static_assert(std::is_same<akm::Vec3::value_type, akm::scalar_t>(), "GLM not using scalar type.");
static_assert(std::is_same<akm::Vec4::value_type, akm::scalar_t>(), "GLM not using scalar type.");

static_assert(std::is_same<akm::Mat2::value_type, akm::scalar_t>(), "GLM not using scalar type.");
static_assert(std::is_same<akm::Mat3::value_type, akm::scalar_t>(), "GLM not using scalar type.");
static_assert(std::is_same<akm::Mat4x3::value_type, akm::scalar_t>(), "GLM not using scalar type.");
static_assert(std::is_same<akm::Mat4::value_type, akm::scalar_t>(), "GLM not using scalar type.");

static_assert(std::is_same<akm::Quat::value_type, akm::scalar_t>(), "GLM not using scalar type.");


