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

#ifndef AK_MATH_SERIALIZE_HPP_
#define AK_MATH_SERIALIZE_HPP_

#include <ak/data/PValue.hpp>
#include <ak/math/Matrix.hpp>
#include <ak/math/Quaternion.hpp>
#include <ak/math/SphericalCoord.hpp>
#include <ak/math/Transform.hpp>
#include <ak/math/Vector.hpp>

namespace akd {
	inline void serialize(akd::PValue& dest, const akm::Quat& val) {
		for(auto i = 0u; i < 4; i++) {
			dest[i].set<typename akm::Quat::value_type>(val[static_cast<int>(i)]);
		}
	}

	inline void serialize(akd::PValue& dest, const akm::Vec4& val) {
		for(auto i = 0u; i < 4; i++) {
			dest[i].set<typename akm::Vec4::value_type>(val[static_cast<int>(i)]);
		}
	}

	inline void serialize(akd::PValue& dest, const akm::Vec3& val) {
		for(auto i = 0u; i < 2; i++) {
			dest[i].set<typename akm::Vec3::value_type>(val[static_cast<int>(i)]);
		}
	}

	inline void serialize(akd::PValue& dest, const akm::Vec2& val) {
		for(auto i = 0u; i < 1; i++) {
			dest[i].set<typename akm::Vec2::value_type>(val[static_cast<int>(i)]);
		}
	}

	inline void serialize(akd::PValue& dest, const akm::Mat4& val) {
		for(auto i = 0u; i < 4; i++) {
			serialize(dest[i], val[static_cast<int>(i)]);
		}
	}

	inline void serialize(akd::PValue& dest, const akm::Mat3& val) {
		for(auto i = 0u; i < 3; i++) {
			serialize(dest[i], val[static_cast<int>(i)]);
		}
	}

	inline void serialize(akd::PValue& dest, const akm::Mat2& val) {
		for(auto i = 0u; i < 2; i++) {
			serialize(dest[i], val[static_cast<int>(i)]);
		}
	}

	inline void serialize(akd::PValue& dest, const akm::SphericalCoord& val) {
		dest[0].setDec(val.radius());
		dest[1].setDec(val.polar());
		dest[2].setDec(val.azimuth());
	}

	inline void serialize(akd::PValue& dest, const akm::Transform3D& val) {
		serialize(dest["pos"], val.position());
		serialize(dest["rot"], val.rotation());
	}


	inline bool deserialize(akm::Quat& dest, const akd::PValue& val) {
		try {
			dest = akm::Quat(
				val[0].as<typename akm::Quat::value_type>(),
				val[1].as<typename akm::Quat::value_type>(),
				val[2].as<typename akm::Quat::value_type>(),
				val[3].as<typename akm::Quat::value_type>()
			);
			return true;
		} catch(const std::logic_error& /*e*/) {
			return false;
		}
	}

	inline bool deserialize(akm::Vec4& dest, const akd::PValue& val) {
		try {
			dest = akm::Vec4(
				val[0].as<typename akm::Vec4::value_type>(),
				val[1].as<typename akm::Vec4::value_type>(),
				val[2].as<typename akm::Vec4::value_type>(),
				val[3].as<typename akm::Vec4::value_type>()
			);
			return true;
		} catch(const std::logic_error& /*e*/) {
			return false;
		}
	}

	inline bool deserialize(akm::Vec3& dest, const akd::PValue& val) {
		try {
			dest = akm::Vec3(
				val[0].as<typename akm::Vec3::value_type>(),
				val[1].as<typename akm::Vec3::value_type>(),
				val[2].as<typename akm::Vec3::value_type>()
			);
			return true;
		} catch(const std::logic_error& /*e*/) {
			return false;
		}
	}

	inline bool deserialize(akm::Vec2& dest, const akd::PValue& val) {
		try {
			dest = akm::Vec2(
				val[0].as<typename akm::Vec2::value_type>(),
				val[1].as<typename akm::Vec2::value_type>()
			);
			return true;
		} catch(const std::logic_error& /*e*/) {
			return false;
		}
	}

	inline bool deserialize(akm::Mat4& dest, const akd::PValue& val) {
		akm::Vec4 c0; if (!deserialize(c0, val[0])) return false;
		akm::Vec4 c1; if (!deserialize(c1, val[1])) return false;
		akm::Vec4 c2; if (!deserialize(c2, val[2])) return false;
		akm::Vec4 c3; if (!deserialize(c3, val[3])) return false;
		dest = akm::Mat4(c0, c1, c2, c3);
		return true;
	}

	inline bool deserialize(akm::Mat3& dest, const akd::PValue& val) {
		akm::Vec3 c0; if (!deserialize(c0, val[0])) return false;
		akm::Vec3 c1; if (!deserialize(c1, val[1])) return false;
		akm::Vec3 c2; if (!deserialize(c2, val[2])) return false;
		dest = akm::Mat3(c0, c1, c2);
		return true;
	}

	inline bool deserialize(akm::Mat2& dest, const akd::PValue& val) {
		akm::Vec2 c0; if (!deserialize(c0, val[0])) return false;
		akm::Vec2 c1; if (!deserialize(c1, val[1])) return false;
		dest = akm::Mat2(c0, c1);
		return true;
	}

	inline bool deserialize(akm::SphericalCoord& dest, const akd::PValue& val) {
		try {
			dest = akm::SphericalCoord();
			dest.setRadius(val[0].asOrDef<typename akm::SphericalCoord::value_type>(0));
			dest.rotateLR(val[1].asOrDef<typename akm::SphericalCoord::value_type>(0));
			dest.rotateUD(val[2].asOrDef<typename akm::SphericalCoord::value_type>(0));
			return true;
		} catch(const std::logic_error& /*e*/) {
			return false;
		}
	}

	inline bool deserialize(akm::Transform3D& dest, const akd::PValue& val) {
		akm::Vec3 pos; if (!deserialize(pos, val["pos"])) return false;
		akm::Quat rot; if (!deserialize(rot, val["rot"])) return false;
		dest.position() = pos;
		dest.rotation() = rot;
		return true;
	}
}

#endif
