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

#ifndef AK_ENGINE_COMPONENTS_TRANSFORM_HPP_
#define AK_ENGINE_COMPONENTS_TRANSFORM_HPP_


#include <ak/container/SlotMap.hpp>
#include <ak/engine/EntityManager.hpp>
#include <ak/engine/Type.hpp>
#include <ak/math/Matrix.hpp>
#include <ak/math/Quaternion.hpp>
#include <ak/math/Types.hpp>
#include <ak/math/Vector.hpp>
#include <glm/detail/type_mat3x3.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <deque>
#include <initializer_list>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace ake {

	class TransformManager;

	class Transform {
		AKE_DEFINE_COMPONENT(TransformManager, Transform)
		private:
			TransformManager* m_manager;
			EntityID m_id;

		public:
			Transform(TransformManager& manager, EntityID id);

			// ///////// //
			// // Get // //
			// ///////// //

			akm::Mat4 localToWorld() const;
			akm::Mat4 worldToLocal() const;

			akm::Vec3 position() const;

			akm::Mat4 rotationMatrix() const;
			akm::Quat rotationQuat() const;
			akm::Vec3 rotationEuler() const;

			fpSingle scale() const;

			akm::Vec3 rightward() const;
			akm::Vec3 leftward() const;
			akm::Vec3 upward() const;
			akm::Vec3 downward() const;
			akm::Vec3 forward() const;
			akm::Vec3 backward() const;

			// ///////// //
			// // Set // //
			// ///////// //

			Transform& setPosition(const akm::Vec3& p);

			Transform& setRotation(const akm::Mat4& r);
			Transform& setRotation(const akm::Quat& r);
			Transform& setRotation(const akm::Vec3& r);
			Transform& setRotation(const akm::Vec3& forward, const akm::Vec3& upward);

			Transform& setScale(fpSingle s);

			// /////////// //
			// // Apply // //
			// /////////// //

			Transform& move(const akm::Vec3& p);
			Transform& moveForward(fpSingle dist);
			Transform& moveBackward(fpSingle dist);
			Transform& moveUpward(fpSingle dist);
			Transform& moveDownward(fpSingle dist);
			Transform& moveRightward(fpSingle dist);
			Transform& moveLeftward(fpSingle dist);

			Transform& rotate(fpSingle angle, const akm::Vec3& axis);
			Transform& rotate(const akm::Mat4& r);
			Transform& rotate(const akm::Quat& r);
			Transform& rotate(const akm::Vec3& r);

			Transform& scaleByFactor(fpSingle s);

			// ///////// //
			// // Get // //
			// ///////// //

			akm::Mat4 localToParent() const;
			akm::Mat4 parentToLocal() const;

			akm::Vec3 localPosition() const;

			akm::Mat4 localRotationMatrix() const;
			akm::Quat localRotationQuat() const;
			akm::Vec3 localRotationEuler() const;

			fpSingle localScale() const;

			akm::Vec3 localRightward() const;
			akm::Vec3 localLeftward() const;
			akm::Vec3 localUpward() const;
			akm::Vec3 localDownward() const;
			akm::Vec3 localForward() const;
			akm::Vec3 localBackward() const;

			// ///////// //
			// // Set // //
			// ///////// //

			Transform& setLocalPosition(const akm::Vec3& p);

			Transform& setLocalRotation(const akm::Mat4& r);
			Transform& setLocalRotation(const akm::Quat& r);
			Transform& setLocalRotation(const akm::Vec3& r);
			Transform& setLocalRotation(const akm::Vec3& forward, const akm::Vec3& upward);

			Transform& setLocalScale(fpSingle s);

			// /////////// //
			// // Apply // //
			// /////////// //

			Transform& moveLocal(const akm::Vec3& p);
			Transform& moveLocalForward(fpSingle dist);
			Transform& moveLocalBackward(fpSingle dist);
			Transform& moveLocalUpward(fpSingle dist);
			Transform& moveLocalDownward(fpSingle dist);
			Transform& moveLocalRightward(fpSingle dist);
			Transform& moveLocalLeftward(fpSingle dist);

			Transform& rotateLocal(fpSingle angle, const akm::Vec3& axis);
			Transform& rotateLocal(const akm::Mat4& r);
			Transform& rotateLocal(const akm::Quat& r);
			Transform& rotateLocal(const akm::Vec3& r);

			Transform& scaleLocalByFactor(fpSingle s);

			// //////////////// //
			// // Components // //
			// //////////////// //

			EntityID id() const;
	};

	class TransformManager final : public ComponentManager {
		AKE_DEFINE_COMPONENT_MANAGER(TransformManager, Transform)
		private:
			struct TransformNode final {
				akm::Vec3 position;
				akm::Quat rotation;
				akm::scalar_t scale;
			};

			struct CacheNode final {
				mutable bool isGlobalDirty;
				mutable akm::Mat4 globalTransform;
				mutable bool isLocalDirty;
				mutable akm::Mat4 localTransform;
			};


			std::unordered_map<EntityID, TransformNode> m_transform;
			std::unordered_map<EntityID, CacheNode> m_cache;

			akev::SubscriberID m_entityGraphID;

			void registerHooks() override {
				if (!m_entityGraphID) return;
				m_entityGraphID = entityManager().entityParentChanged().subscribe([&](const auto& ev){
					if (hasComponent(ev.data().modifiedEntity())) markDirty(ev.data().modifiedEntity());
				});
			}

			void markDirty(EntityID id) const {
				m_cache.at(id).isLocalDirty = true;

				std::deque<EntityID> dirtyList({id});
				while(!dirtyList.empty()) {
					// If the parent node is already dirty it's children should already be flagged.
					if (!std::exchange(m_cache.at(dirtyList.front()).isGlobalDirty, true)) {
						auto& children = entityManager().entityChildrenIDs(dirtyList.front());
						dirtyList.insert(dirtyList.end(), children.begin(), children.end());
					}
					dirtyList.pop_front();
				}
			}

		protected:
			bool createComponent(EntityID entityID, const akm::Vec3& position = {0,0,0}, const akm::Quat& rotation = {1,0,0,0}, const akm::scalar_t& scale = 1.f, bool worldSpace = false) {
				if (!m_transform.emplace(entityID, TransformNode{position, rotation, scale}).second) return false;
				if (!m_cache.emplace(entityID, CacheNode{true, akm::Mat4(1), true, akm::Mat4(1)}).second) throw std::logic_error("ake::TransformManager: Data corruption, cache data exists when transform exists");

				if (worldSpace) {
					setPosition(entityID, position);
					setRotation(entityID, rotation);
					setScale(entityID, scale);
				}

				return true;
			}

			bool destroyComponent(EntityID entityID) override {
				auto iter = m_transform.find(entityID);
				if (iter == m_transform.end()) throw std::logic_error("ake::Transform: Data corruption, tried to delete non-existent instance.");
				m_transform.erase(entityID);
				m_cache.erase(entityID);
				return true;
			}

			//return akm::translate(node.position) * akm::mat4_cast(node.rotation) * akm::scale({node.scale,node.scale,node.scale});
			akm::Mat4 calculateTransformMatrix(const TransformNode& node) const {
				auto rotationMatrix = node.scale * akm::mat3_cast(node.rotation);
				return akm::Mat4(
					akm::Vec4(akm::column(rotationMatrix, 0), 0),
					akm::Vec4(akm::column(rotationMatrix, 1), 0),
					akm::Vec4(akm::column(rotationMatrix, 2), 0),
					akm::Vec4(node.position, 1)
				);
			}

		public:

			// ///////// //
			// // Get // //
			// ///////// //

			akm::Mat4 localToWorld(EntityID entityID) const {
				auto cacheIter = m_cache.find(entityID);
				if (cacheIter == m_cache.end()) return akm::Mat4(1);
				if (std::exchange(cacheIter->second.isGlobalDirty, false)) cacheIter->second.globalTransform = localToWorld(entityManager().entityParentID(entityID)) * localToParent(entityID);
				return cacheIter->second.globalTransform;
			}

			akm::Mat4 worldToLocal(EntityID entityID) const { return akm::inverse(localToWorld(entityID)); }

			akm::Vec3 position(EntityID entityID) const { return akm::column(localToWorld(entityID), 3); }

			akm::Mat4 rotationMatrix(EntityID entityID) const { return akm::Mat3(localToWorld(entityID))/scale(entityID); }
			akm::Quat rotationQuat(  EntityID entityID) const { return akm::quat_cast(rotationMatrix(entityID)); }
			akm::Vec3 rotationEuler( EntityID entityID) const { return akm::toEuler(rotationQuat(entityID)); }

			fpSingle scale(EntityID entityID) const { return akm::magnitude(akm::column(localToWorld(entityID), 0)); }

			akm::Vec3 rightward(EntityID entityID) const { return akm::column(rotationMatrix(entityID), 0); }
			akm::Vec3 leftward( EntityID entityID) const { return -upward(entityID); }
			akm::Vec3 upward(   EntityID entityID) const { return akm::column(rotationMatrix(entityID), 1); }
			akm::Vec3 downward( EntityID entityID) const { return -upward(entityID); }
			akm::Vec3 forward(  EntityID entityID) const { return akm::column(rotationMatrix(entityID), 2); }
			akm::Vec3 backward( EntityID entityID) const { return -forward(entityID); }

			// ///////// //
			// // Set // //
			// ///////// //

			void setPosition(EntityID entityID, const akm::Vec3& p) {
				auto parentID = entityManager().entityParentID(entityID);
				auto parentInvTransform = worldToLocal(parentID);
				setLocalPosition(entityID, parentInvTransform * akm::Vec4(p, 1));
			}


			void setRotation(EntityID entityID, const akm::Mat4& r) {
				setRotation(entityID, akm::quat_cast(r));
			}

			void setRotation(EntityID entityID, const akm::Quat& r) {
				auto parentID = entityManager().entityParentID(entityID);
				auto parentRot = hasComponent(parentID) ? rotationQuat(parentID) : akm::Quat(1,0,0,0);
				setLocalRotation(entityID, akm::inverse(parentRot) * r);
			}

			void setRotation(EntityID entityID, const akm::Vec3& r) {
				setRotation(entityID, akm::fromEuler(r));
			}

			void setRotation(EntityID entityID, const akm::Vec3& f, const akm::Vec3& u) {
				auto rForward = akm::normalize(f);
				auto rUp = akm::normalize(u);
				auto rRight = akm::normalize(akm::cross(rUp, rForward));
				rUp = akm::normalize(akm::cross(rForward, rRight));

				setRotation(entityID, akm::quat_cast(akm::Mat3(rRight, rUp, rForward)));
			}


			void setScale(EntityID entityID, fpSingle s) {
				auto parentID = entityManager().entityParentID(entityID);
				auto parentScl = hasComponent(parentID) ? scale(parentID) : 1.f;
				setLocalScale(entityID, s/parentScl);
			}

			// /////////// //
			// // Apply // //
			// /////////// //

			void move(EntityID entityID, const akm::Vec3& p) { setPosition(entityID, position(entityID) + p); }

			void moveForward(  EntityID entityID, fpSingle dist) { move(entityID,   forward(entityID)*dist); }
			void moveBackward( EntityID entityID, fpSingle dist) { move(entityID,  backward(entityID)*dist); }
			void moveUpward(   EntityID entityID, fpSingle dist) { move(entityID,    upward(entityID)*dist); }
			void moveDownward( EntityID entityID, fpSingle dist) { move(entityID,  downward(entityID)*dist); }
			void moveRightward(EntityID entityID, fpSingle dist) { move(entityID, rightward(entityID)*dist); }
			void moveLeftward( EntityID entityID, fpSingle dist) { move(entityID,  leftward(entityID)*dist); }

			void rotate(EntityID entityID, fpSingle angle, const akm::Vec3& axis) { rotate(entityID, akm::rotateQ(angle, axis)); }
			void rotate(EntityID entityID, const akm::Mat4& r) { rotate(entityID, akm::quat_cast(r)); }
			void rotate(EntityID entityID, const akm::Quat& r) { setRotation(entityID, rotationQuat(entityID) * r); }
			void rotate(EntityID entityID, const akm::Vec3& r) { rotate(entityID, akm::fromEuler(r)); }

			void rotatePre(EntityID entityID, fpSingle angle, const akm::Vec3& axis) { rotatePre(entityID, akm::rotateQ(angle, axis)); }
			void rotatePre(EntityID entityID, const akm::Mat4& r) { rotatePre(entityID, akm::quat_cast(r)); }
			void rotatePre(EntityID entityID, const akm::Quat& r) { setRotation(entityID, r * rotationQuat(entityID)); }
			void rotatePre(EntityID entityID, const akm::Vec3& r) { rotatePre(entityID, akm::fromEuler(r)); }

			void scaleByFactor(EntityID entityID, fpSingle s) { setScale(entityID, scale(entityID) * s); }

			// ///////// //
			// // Get // //
			// ///////// //

			akm::Mat4 localToParent(EntityID entityID) const {
				auto& cacheEntry = m_cache.at(entityID);
				if (std::exchange(cacheEntry.isLocalDirty, false)) cacheEntry.localTransform = calculateTransformMatrix(m_transform.at(entityID));
				return cacheEntry.localTransform;
			}

			akm::Mat4 parentToLocal(EntityID entityID) const { return akm::inverse(localToParent(entityID)); }

			akm::Vec3 localPosition(EntityID entityID) const { return m_transform.at(entityID).position; }

			akm::Mat4 localRotationMatrix(EntityID entityID) const { return akm::mat4_cast(localRotationQuat(entityID)); }
			akm::Quat localRotationQuat(  EntityID entityID) const { return m_transform.at(entityID).rotation; }
			akm::Vec3 localRotationEuler( EntityID entityID) const { return akm::toEuler(localRotationQuat(entityID)); }

			fpSingle localScale(EntityID entityID) const { return m_transform.at(entityID).scale; }

			akm::Vec3 localRightward(EntityID entityID) const { return akm::column(localRotationMatrix(entityID), 0); }
			akm::Vec3 localLeftward( EntityID entityID) const { return -localRightward(entityID); }
			akm::Vec3 localUpward(   EntityID entityID) const { return akm::column(localRotationMatrix(entityID), 1); }
			akm::Vec3 localDownward( EntityID entityID) const { return -localUpward(entityID); }
			akm::Vec3 localForward(  EntityID entityID) const { return akm::column(localRotationMatrix(entityID), 2); }
			akm::Vec3 localBackward( EntityID entityID) const { return -localForward(entityID); }

			// ///////// //
			// // Set // //
			// ///////// //

			void setLocalPosition(EntityID entityID, const akm::Vec3& p){
				m_transform.at(entityID).position = p;
				markDirty(entityID);
			}

			void setLocalRotation(EntityID entityID, const akm::Mat4& r) { setLocalRotation(entityID, akm::quat_cast(r)); }
			void setLocalRotation(EntityID entityID, const akm::Quat& r) { m_transform.at(entityID).rotation = r; markDirty(entityID); }
			void setLocalRotation(EntityID entityID, const akm::Vec3& r) { setLocalRotation(entityID, akm::fromEuler(r)); }
			void setLocalRotation(EntityID entityID, const akm::Vec3& f, const akm::Vec3& u) {
				auto rForward = akm::normalize(f);
				auto rUp = akm::normalize(u);
				auto rRight = akm::normalize(akm::cross(rUp, rForward));
				rUp = akm::normalize(akm::cross(rForward, rRight));

				setLocalRotation(entityID, akm::quat_cast(akm::Mat3(rRight, rUp, rForward)));
			}

			void setLocalScale(EntityID entityID, fpSingle s) {
				m_transform.at(entityID).scale = s;
				markDirty(entityID);
			}

			// /////////// //
			// // Apply // //
			// /////////// //

			void moveLocal(EntityID entityID, const akm::Vec3& p) { setLocalPosition(entityID, localPosition(entityID) + p); }

			void moveLocalForward(  EntityID entityID, fpSingle dist) { moveLocal(entityID,   localForward(entityID)*dist); }
			void moveLocalBackward( EntityID entityID, fpSingle dist) { moveLocal(entityID,  localBackward(entityID)*dist); }
			void moveLocalUpward(   EntityID entityID, fpSingle dist) { moveLocal(entityID,    localUpward(entityID)*dist); }
			void moveLocalDownward( EntityID entityID, fpSingle dist) { moveLocal(entityID,  localDownward(entityID)*dist); }
			void moveLocalRightward(EntityID entityID, fpSingle dist) { moveLocal(entityID, localRightward(entityID)*dist); }
			void moveLocalLeftward( EntityID entityID, fpSingle dist) { moveLocal(entityID,  localLeftward(entityID)*dist); }

			void rotateLocal(EntityID entityID, fpSingle angle, const akm::Vec3& axis) { rotateLocal(entityID, akm::rotateQ(angle, axis)); }
			void rotateLocal(EntityID entityID, const akm::Mat4& r) { rotateLocal(entityID, akm::quat_cast(r)); }
			void rotateLocal(EntityID entityID, const akm::Quat& r) { setLocalRotation(entityID, rotationQuat(entityID) * r); }
			void rotateLocal(EntityID entityID, const akm::Vec3& r) { rotateLocal(entityID, akm::fromEuler(r)); }

			void scaleLocalByFactor(EntityID entityID, fpSingle s) { setLocalScale(entityID, localScale(entityID) * s); }

			// //////////////// //
			// // Components // //
			// //////////////// //

			Transform component(EntityID entityID) { return Transform(*this, entityID); }
			const Transform component(EntityID entityID) const { return Transform(const_cast<TransformManager&>(*this), entityID); }

			bool hasComponent(EntityID entityID) const override { return m_transform.find(entityID) != m_transform.end(); }
	};

}

namespace ake {
	inline Transform::Transform(TransformManager& manager, EntityID id) : m_manager(&manager), m_id(id) {}

	// ///////// //
	// // Get // //
	// ///////// //

	inline akm::Mat4 Transform::localToWorld() const { return m_manager->localToWorld(m_id); }
	inline akm::Mat4 Transform::worldToLocal() const { return m_manager->worldToLocal(m_id); }

	inline akm::Vec3 Transform::position() const { return m_manager->position(m_id); }

	inline akm::Mat4 Transform::rotationMatrix() const { return m_manager->rotationMatrix(m_id); }
	inline akm::Quat Transform::rotationQuat() const { return m_manager->rotationQuat(m_id); }
	inline akm::Vec3 Transform::rotationEuler() const { return m_manager->rotationEuler(m_id); }

	inline fpSingle Transform::scale() const { return m_manager->scale(m_id); }

	inline akm::Vec3 Transform::rightward() const { return m_manager->rightward(m_id); }
	inline akm::Vec3 Transform::leftward()  const { return m_manager->leftward(m_id);  }
	inline akm::Vec3 Transform::upward()    const { return m_manager->upward(m_id);    }
	inline akm::Vec3 Transform::downward()  const { return m_manager->downward(m_id);  }
	inline akm::Vec3 Transform::forward()   const { return m_manager->forward(m_id);   }
	inline akm::Vec3 Transform::backward()  const { return m_manager->backward(m_id);  }

	// ///////// //
	// // Set // //
	// ///////// //

	inline Transform& Transform::setPosition(const akm::Vec3& p) { m_manager->setPosition(m_id, p); return *this; }

	inline Transform& Transform::setRotation(const akm::Mat4& r) { m_manager->setRotation(m_id, r); return *this; }
	inline Transform& Transform::setRotation(const akm::Quat& r) { m_manager->setRotation(m_id, r); return *this; }
	inline Transform& Transform::setRotation(const akm::Vec3& r) { m_manager->setRotation(m_id, r); return *this; }
	inline Transform& Transform::setRotation(const akm::Vec3& f, const akm::Vec3& u) { m_manager->setRotation(m_id, f, u); return *this; }

	inline Transform& Transform::setScale(fpSingle s) { m_manager->setScale(m_id, s); return *this; }

	// /////////// //
	// // Apply // //
	// /////////// //

	inline Transform& Transform::move(const akm::Vec3& p) { m_manager->move(m_id, p); return *this; }
	inline Transform& Transform::moveForward(  fpSingle dist) { m_manager->moveForward(  m_id, dist); return *this; }
	inline Transform& Transform::moveBackward( fpSingle dist) { m_manager->moveBackward( m_id, dist); return *this; }
	inline Transform& Transform::moveUpward(   fpSingle dist) { m_manager->moveUpward(   m_id, dist); return *this; }
	inline Transform& Transform::moveDownward( fpSingle dist) { m_manager->moveDownward( m_id, dist); return *this; }
	inline Transform& Transform::moveRightward(fpSingle dist) { m_manager->moveRightward(m_id, dist); return *this; }
	inline Transform& Transform::moveLeftward( fpSingle dist) { m_manager->moveLeftward( m_id, dist); return *this; }

	inline Transform& Transform::rotate(fpSingle angle, const akm::Vec3& axis) { m_manager->rotate(m_id, angle, axis); return *this; }
	inline Transform& Transform::rotate(const akm::Mat4& r) { m_manager->rotate(m_id, r); return *this; }
	inline Transform& Transform::rotate(const akm::Quat& r) { m_manager->rotate(m_id, r); return *this; }
	inline Transform& Transform::rotate(const akm::Vec3& r) { m_manager->rotate(m_id, r); return *this; }

	inline Transform& Transform::scaleByFactor(fpSingle s) { m_manager->scaleByFactor(m_id, s); return *this; }

	// ///////// //
	// // Get // //
	// ///////// //

	inline akm::Mat4 Transform::localToParent() const { return m_manager->localToParent(m_id); }
	inline akm::Mat4 Transform::parentToLocal() const { return m_manager->parentToLocal(m_id); }

	inline akm::Vec3 Transform::localPosition() const { return m_manager->localPosition(m_id); }

	inline akm::Mat4 Transform::localRotationMatrix() const { return m_manager->localRotationMatrix(m_id); }
	inline akm::Quat Transform::localRotationQuat() const { return m_manager->localRotationQuat(m_id); }
	inline akm::Vec3 Transform::localRotationEuler() const { return m_manager->localRotationEuler(m_id); }

	inline fpSingle Transform::localScale() const { return m_manager->localScale(m_id); }

	inline akm::Vec3 Transform::localRightward() const { return m_manager->localRightward(m_id); }
	inline akm::Vec3 Transform::localLeftward()  const { return m_manager->localLeftward(m_id);  }
	inline akm::Vec3 Transform::localUpward()    const { return m_manager->localUpward(m_id);    }
	inline akm::Vec3 Transform::localDownward()  const { return m_manager->localDownward(m_id);  }
	inline akm::Vec3 Transform::localForward()   const { return m_manager->localForward(m_id);   }
	inline akm::Vec3 Transform::localBackward()  const { return m_manager->localBackward(m_id);  }

	// ///////// //
	// // Set // //
	// ///////// //

	inline Transform& Transform::setLocalPosition(const akm::Vec3& p) { m_manager->setPosition(m_id, p); return *this; }

	inline Transform& Transform::setLocalRotation(const akm::Mat4& r) { m_manager->setLocalRotation(m_id, r); return *this; }
	inline Transform& Transform::setLocalRotation(const akm::Quat& r) { m_manager->setLocalRotation(m_id, r); return *this; }
	inline Transform& Transform::setLocalRotation(const akm::Vec3& r) { m_manager->setLocalRotation(m_id, r); return *this; }
	inline Transform& Transform::setLocalRotation(const akm::Vec3& f, const akm::Vec3& u) { m_manager->setLocalRotation(m_id, f, u); return *this; }

	inline Transform& Transform::setLocalScale(fpSingle s) { m_manager->setLocalScale(m_id, s); return *this; }

	// /////////// //
	// // Apply // //
	// /////////// //

	inline Transform& Transform::moveLocal(const akm::Vec3& p) { m_manager->moveLocal(m_id, p); return *this; }
	inline Transform& Transform::moveLocalForward(  fpSingle dist) { m_manager->moveLocalForward(  m_id, dist); return *this; }
	inline Transform& Transform::moveLocalBackward( fpSingle dist) { m_manager->moveLocalBackward( m_id, dist); return *this; }
	inline Transform& Transform::moveLocalUpward(   fpSingle dist) { m_manager->moveLocalUpward(   m_id, dist); return *this; }
	inline Transform& Transform::moveLocalDownward( fpSingle dist) { m_manager->moveLocalDownward( m_id, dist); return *this; }
	inline Transform& Transform::moveLocalRightward(fpSingle dist) { m_manager->moveLocalRightward(m_id, dist); return *this; }
	inline Transform& Transform::moveLocalLeftward( fpSingle dist) { m_manager->moveLocalLeftward( m_id, dist); return *this; }

	inline Transform& Transform::rotateLocal(fpSingle angle, const akm::Vec3& axis) { m_manager->rotateLocal(m_id, angle, axis); return *this; }
	inline Transform& Transform::rotateLocal(const akm::Mat4& r) { m_manager->rotateLocal(m_id, r); return *this; }
	inline Transform& Transform::rotateLocal(const akm::Quat& r) { m_manager->rotateLocal(m_id, r); return *this; }
	inline Transform& Transform::rotateLocal(const akm::Vec3& r) { m_manager->rotateLocal(m_id, r); return *this; }

	inline Transform& Transform::scaleLocalByFactor(fpSingle s) { m_manager->scaleLocalByFactor(m_id, s); return *this; }

	// //////////////// //
	// // Components // //
	// //////////////// //

	inline EntityID Transform::id() const { return m_id; }
}

#endif
