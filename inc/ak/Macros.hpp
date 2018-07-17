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

/**
 * Simplifies creating constexpr string_views
 */
#define AK_STRING_VIEW(str) std::string_view(str, sizeof(str) - 1)

/**
 * Defines an enum called "enumName" with the underlaying type "enumType"
 * Also defines functions the following functions:
 *  - [enumName]ToStringView <constexpr>
 *  - [enumName]ToString
 *  - stringTo[enumName] <constexpr>
 *  - stringTo[enumName]
 * To assist in converting to/from string representations
 */
#define AK_DEFINE_SMART_ENUM(enumName, enumType, ...) \
	enum enumName : enumType { AK_CONCATENATE(AK_INTERNAL_ENUM_KV_,  AK_NARGS(__VA_ARGS__))(__VA_ARGS__) }; \
	inline constexpr std::string_view enumName##ToStringView(enumName val) { switch(val) { AK_CONCATENATE(AK_INTERNAL_ENUM_TO_STR_,  AK_NARGS(__VA_ARGS__))(enumName, __VA_ARGS__) } } \
	inline std::string enumName##ToString(enumName val) { return std::string(enumName##ToStringView(val)); } \
	inline constexpr enumName stringTo##enumName(std::string_view val) { \
		AK_CONCATENATE(AK_INTERNAL_ENUM_TO_ENUM_,  AK_NARGS(__VA_ARGS__))(enumName, __VA_ARGS__) \
		throw std::logic_error(std::string("Invalid identifier for enum '"#enumName"': ") + std::string(val)); \
	} \
	inline enumName stringTo##enumName(const std::string& val) { \
		AK_CONCATENATE(AK_INTERNAL_ENUM_TO_ENUM_,  AK_NARGS(__VA_ARGS__))(enumName, __VA_ARGS__) \
		throw std::logic_error(std::string("Invalid identifier for enum '"#enumName"': ") + val); \
	}

/**
 * Defines an enum class called "enumName" with the underlaying type "enumType"
 * Also defines functions the following functions:
 *  - [enumName]ToStringView <constexpr>
 *  - [enumName]ToString
 *  - stringTo[enumName] <constexpr>
 *  - stringTo[enumName]
 * To assist in converting to/from string representations
 */
#define AK_DEFINE_SMART_ENUM_CLASS(enumName, enumType, ...) \
	enum class enumName : enumType { AK_CONCATENATE(AK_INTERNAL_ENUM_KV_,  AK_NARGS(__VA_ARGS__))(__VA_ARGS__) }; \
	inline constexpr std::string_view enumName##ToStringView(enumName val) { switch(val) { AK_CONCATENATE(AK_INTERNAL_ENUM_TO_STR_,  AK_NARGS(__VA_ARGS__))(enumName, __VA_ARGS__) } } \
	inline std::string enumName##ToString(enumName val) { return std::string(enumName##ToStringView(val)); } \
	inline constexpr enumName stringTo##enumName(std::string_view val) { \
		AK_CONCATENATE(AK_INTERNAL_ENUM_TO_ENUM_,  AK_NARGS(__VA_ARGS__))(enumName, __VA_ARGS__) \
		throw std::logic_error(std::string("Invalid identifier for enum '"#enumName"': ") + std::string(val)); \
	} \
	inline enumName stringTo##enumName(const std::string& val) { \
		AK_CONCATENATE(AK_INTERNAL_ENUM_TO_ENUM_,  AK_NARGS(__VA_ARGS__))(enumName, __VA_ARGS__) \
		throw std::logic_error(std::string("Invalid identifier for enum '"#enumName"': ") + val); \
	}

/**
 * Defines serialization/deserialization methods in the akd namespace for enums defined using AK_DEFINE_SMART_ENUM/AK_DEFINE_SMART_ENUM_CLASS
 */
#define AK_DEFINE_SMART_ENUM_SERIALIZE(qualification, enumName) \
	namespace akd { \
		inline void serialize(akd::PValue& dst, const qualification::enumName& val) { dst.setStr(qualification::enumName##ToString(val)); } \
		inline bool deserialize(qualification::enumName& dst, const akd::PValue& val) { \
			try { dst = qualification::stringTo##enumName(val.asStr()); } \
			catch(const std::logic_error&) { return false; } \
			return true; \
		} \
	}

/**
 * Token pasting for use in macros
 */
#define AK_CONCATENATE(arg1, arg2)   AK_INTERNAL_CONCATENATE1(arg1, arg2)

/**
 * Macro varg counter, counts 0-99 inclusive
 */
#define AK_NARGS(...) AK_INTERNAL_NARGS(0, ## __VA_ARGS__, \
		99, 98, 97, 96, 95, 94, 93, 92, 91, 90, \
		89, 88, 87, 86, 85, 84, 83, 82, 81, 80, \
		79, 78, 77, 76, 75, 74, 73, 72, 71, 70, \
		69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
		59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
		49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
		39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
		29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
		19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
		 9,  8,  7,  6,  5,  4,  3,  2,  1,  0)

// //////////////////////////// //
// // Implementation Section // //
// //////////////////////////// //

#define AK_INTERNAL_CONCATENATE1(arg1, arg2)  AK_INTERNAL_CONCATENATE2(arg1, arg2)
#define AK_INTERNAL_CONCATENATE2(arg1, arg2)  arg1##arg2

#define AK_INTERNAL_NARGS(\
		 _0,  _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, \
		_10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
		_20, _21, _22, _23, _24, _25, _26, _27, _28, _29, \
		_30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
		_40, _41, _42, _43, _44, _45, _46, _47, _48, _49, \
		_50, _51, _52, _53, _54, _55, _56, _57, _58, _59, \
		_60, _61, _62, _63, _64, _65, _66, _67, _68, _69, \
		_70, _71, _72, _73, _74, _75, _76, _77, _78, _79, \
		_80, _81, _82, _83, _84, _85, _86, _87, _88, _89, \
		_90, _91, _92, _93, _94, _95, _96, _97, _98, _99, \
		N, ...) N

#define AK_INTERNAL_ENUM_KV_2( x, y) x = y,
#define AK_INTERNAL_ENUM_KV_4( x, y, ...) x = y, AK_INTERNAL_ENUM_KV_2( __VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_6( x, y, ...) x = y, AK_INTERNAL_ENUM_KV_4( __VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_8( x, y, ...) x = y, AK_INTERNAL_ENUM_KV_6( __VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_10(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_8( __VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_12(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_10(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_14(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_12(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_16(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_14(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_18(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_18(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_20(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_18(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_22(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_20(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_24(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_22(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_26(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_24(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_28(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_26(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_30(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_28(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_32(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_30(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_34(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_32(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_36(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_34(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_38(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_36(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_40(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_38(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_42(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_40(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_44(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_42(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_46(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_44(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_48(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_46(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_50(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_48(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_52(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_50(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_54(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_52(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_56(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_54(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_58(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_56(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_60(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_58(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_62(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_60(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_64(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_62(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_66(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_64(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_68(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_66(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_70(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_68(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_72(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_70(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_74(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_72(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_76(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_74(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_78(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_76(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_80(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_78(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_82(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_80(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_84(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_82(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_86(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_84(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_88(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_86(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_90(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_88(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_92(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_90(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_94(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_92(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_96(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_94(__VA_ARGS__)
#define AK_INTERNAL_ENUM_KV_98(x, y, ...) x = y, AK_INTERNAL_ENUM_KV_96(__VA_ARGS__)

#define AK_INTERNAL_ENUM_TO_STR_2( enumName, x, y)      case enumName::x: return AK_STRING_VIEW(#x);
#define AK_INTERNAL_ENUM_TO_STR_4( enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_2( enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_6( enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_4( enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_8( enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_6( enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_10(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_8( enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_12(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_10(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_14(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_12(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_16(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_14(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_18(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_18(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_20(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_18(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_22(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_20(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_24(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_22(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_26(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_24(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_28(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_26(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_30(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_28(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_32(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_30(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_34(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_32(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_36(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_34(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_38(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_36(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_40(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_38(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_42(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_40(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_44(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_42(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_46(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_44(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_48(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_46(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_50(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_48(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_52(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_50(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_54(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_52(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_56(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_54(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_58(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_56(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_60(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_58(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_62(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_60(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_64(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_62(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_66(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_64(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_68(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_66(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_70(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_68(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_72(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_70(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_74(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_72(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_76(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_74(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_78(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_76(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_80(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_78(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_82(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_80(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_84(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_82(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_86(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_84(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_88(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_86(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_90(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_88(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_92(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_90(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_94(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_92(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_96(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_94(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_STR_98(enumName, x, y, ...) case enumName::x: return AK_STRING_VIEW(#x); AK_INTERNAL_ENUM_TO_STR_96(enumName, __VA_ARGS__)

#define AK_INTERNAL_ENUM_TO_ENUM_2( enumName, x, y)      if(val == #x) return enumName::x;
#define AK_INTERNAL_ENUM_TO_ENUM_4( enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_2( enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_6( enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_4( enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_8( enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_6( enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_10(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_8( enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_12(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_10(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_14(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_12(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_16(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_14(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_18(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_18(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_20(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_18(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_22(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_20(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_24(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_22(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_26(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_24(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_28(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_26(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_30(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_28(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_32(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_30(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_34(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_32(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_36(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_34(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_38(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_36(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_40(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_38(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_42(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_40(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_44(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_42(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_46(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_44(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_48(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_46(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_50(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_48(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_52(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_50(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_54(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_52(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_56(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_54(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_58(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_56(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_60(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_58(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_62(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_60(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_64(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_62(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_66(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_64(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_68(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_66(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_70(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_68(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_72(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_70(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_74(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_72(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_76(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_74(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_78(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_76(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_80(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_78(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_82(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_80(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_84(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_82(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_86(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_84(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_88(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_86(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_90(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_88(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_92(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_90(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_94(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_92(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_96(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_94(enumName, __VA_ARGS__)
#define AK_INTERNAL_ENUM_TO_ENUM_98(enumName, x, y, ...) if(val == #x) return enumName::x; AK_INTERNAL_ENUM_TO_ENUM_96(enumName, __VA_ARGS__)

#endif
