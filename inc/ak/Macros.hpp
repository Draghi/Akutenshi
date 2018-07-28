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

#ifndef AK_MACROS_HPP_
#define AK_MACROS_HPP_

#include <string_view>
#include <ak/internal/MacrosInternal.hpp>

// //////////////////// //
// // String Section // //
// //////////////////// //

// Simplifies creating constexpr string_views
#define AK_STRING_VIEW(str) std::string_view(str, sizeof(str) - 1)

// /////////////////////// //
// // SmartEnum Section // //
// /////////////////////// //

/**
 * A smart enum is an enum with conversions to/from strings predefined:
 *  - [enumName]ToStringView <constexpr>
 *  - [enumName]ToString
 *  - stringTo[enumName] <constexpr>
 *  - stringTo[enumName]
 *  The bellow macros help define different types of smart enums
 */

// Defines a typed smart-enum using automatic value assignment
#define AK_DEFINE_SMART_TENUM(enumName, enumType, ...) \
	enum enumName : enumType { __VA_ARGS__ }; \
	AK_INTERNAL_ENUM_CONV(enumName, __VA_ARGS__)

// Defines a typed smart-enum class using automatic value assignment
#define AK_DEFINE_SMART_TENUM_CLASS(enumName, enumType, ...) \
	enum class enumName : enumType { __VA_ARGS__ }; \
	AK_INTERNAL_ENUM_CONV(enumName, __VA_ARGS__)

// Defines an untyped smart-enum using automatic value assignment
#define AK_DEFINE_SMART_ENUM(enumName, ...) \
	enum enumName { __VA_ARGS__ }; \
	AK_INTERNAL_ENUM_CONV(enumName, __VA_ARGS__)

// Defines a untyped smart-enum class using automatic value assignment
#define AK_DEFINE_SMART_ENUM_CLASS(enumName, ...) \
	enum class enumName { __VA_ARGS__ }; \
	AK_INTERNAL_ENUM_CONV(enumName, __VA_ARGS__)

// Defines a typed smart-enum using manual value assignment
#define AK_DEFINE_SMART_TENUM_KV(enumName, enumType, ...) \
	enum enumName : enumType { AK_CONCATENATE(AK_INTERNAL_ENUM_KV_,  AK_NARGS(__VA_ARGS__))(__VA_ARGS__) }; \
	AK_INTERNAL_ENUM_CONV_KV(enumName, __VA_ARGS__)

// Defines a typed smart-enum class using manual value assignment
#define AK_DEFINE_SMART_TENUM_CLASS_KV(enumName, enumType, ...) \
	enum class enumName : enumType { AK_CONCATENATE(AK_INTERNAL_ENUM_KV_,  AK_NARGS(__VA_ARGS__))(__VA_ARGS__) }; \
	AK_INTERNAL_ENUM_CONV_KV(enumName, __VA_ARGS__)

// Defines an untyped smart-enum using manual value assignment
#define AK_DEFINE_SMART_ENUM_KV(enumName, ...) \
	enum enumName { AK_CONCATENATE(AK_INTERNAL_ENUM_KV_,  AK_NARGS(__VA_ARGS__))(__VA_ARGS__) }; \
	AK_INTERNAL_ENUM_CONV_KV(enumName, __VA_ARGS__)

// Defines an untyped smart-enum class using manual value assignment
#define AK_DEFINE_SMART_ENUM_CLASS_KV(enumName, ...) \
	enum class enumName { AK_CONCATENATE(AK_INTERNAL_ENUM_KV_,  AK_NARGS(__VA_ARGS__))(__VA_ARGS__) }; \
	AK_INTERNAL_ENUM_CONV_KV(enumName, __VA_ARGS__)

// Defines serialization/deserialization methods in the akd namespace for a qualified smart enums
#define AK_DEFINE_SMART_ENUM_SERIALIZE(qualification, enumName) \
	namespace akd { \
		inline void serialize(akd::PValue& dst, const qualification::enumName& val) { dst.setStr(qualification::convert##enumName##ToString(val)); } \
		inline bool deserialize(qualification::enumName& dst, const akd::PValue& val) { \
			try { dst = qualification::convert##StringTo##enumName(val.getStr()); } \
			catch(const std::logic_error&) { return false; } \
			return true; \
		} \
	}

// Defines serialization/deserialization methods in the akd namespace for a unqualified smart enums
#define AK_DEFINE_SMART_ENUM_SERIALIZE_NQ(enumName) \
	namespace akd { \
		inline void serialize(akd::PValue& dst, const ::enumName& val) { dst.setStr(::convert##enumName##ToString(val)); } \
		inline bool deserialize(::enumName& dst, const akd::PValue& val) { \
			try { dst = ::convert##StringTo##enumName(val.asStr()); } \
			catch(const std::logic_error&) { return false; } \
			return true; \
		} \
	}

#endif
