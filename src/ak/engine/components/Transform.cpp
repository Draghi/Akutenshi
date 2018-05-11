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

#include "ak/engine/components/Transform.hpp"

using namespace ake;

// ///////////// //
// // Manager // //
// ///////////// //

void TransformManager::registerHooks() {
	if (!m_entityGraphID) return;
	m_entityGraphID = entityManager().entityParentChanged().subscribe([&](const auto& ev){
		if (hasComponent(ev.data().modifiedEntity())) markDirty(ev.data().modifiedEntity());
	});
}

void TransformManager::markDirty(EntityID id) const {
	std::deque<EntityID> dirtyList({id});
	while(!dirtyList.empty()) {
		// If the parent node is already dirty it's children should already be flagged.
		if (!std::exchange(m_globalTransform.at(dirtyList.front()).isDirty, true)) {
			auto& children = entityManager().entityChildrenIDs(dirtyList.front());
			dirtyList.insert(dirtyList.end(), children.begin(), children.end());
		}
		dirtyList.pop_front();
	}
}

bool TransformManager::createComponent(EntityID entityID, const akm::Vec3& position, const akm::Quat& rotation, const akm::scalar_t& scale, bool worldSpace) {
	if (!m_localTransform.emplace(entityID, LocalTransformNode{position, rotation, scale}).second) return false;
	if (!m_globalTransform.emplace(entityID, GlobalTransformNode{{akm::Vec3(0,0,0), akm::Quat(1,0,0,0), 1}, true}).second) throw std::logic_error("ake::TransformManager: Data corruption, cache data exists when transform exists");

	if (worldSpace) {
		setPosition(entityID, position);
		setRotation(entityID, rotation);
		setScale(entityID, scale);
	}

	return true;
}

bool TransformManager::destroyComponent(EntityID entityID) {
	auto iter = m_localTransform.find(entityID);
	if (iter == m_localTransform.end()) throw std::logic_error("ake::Transform: Data corruption, tried to delete non-existent instance.");
	m_localTransform.erase(entityID);
	m_globalTransform.erase(entityID);
	return true;
}

akm::Mat4 TransformManager::calculateTransformMatrix(const LocalTransformNode& transform) const {
	auto rotationMatrix = transform.scale * akm::mat3_cast(transform.rotation);
	return akm::Mat4(
		akm::Vec4(akm::column(rotationMatrix, 0), 0),
		akm::Vec4(akm::column(rotationMatrix, 1), 0),
		akm::Vec4(akm::column(rotationMatrix, 2), 0),
		akm::Vec4(transform.position, 1)
	);
}

TransformManager::LocalTransformNode TransformManager::getWorldTransform(EntityID entityID) const {
	if (!entityID) return LocalTransformNode{akm::Vec3(0,0,0), akm::Quat(1,0,0,0), 1.f};
	auto& globalNode = m_globalTransform.at(entityID);
	if (!std::exchange(globalNode.isDirty, false)) return globalNode.transform;

	// TODO Experimental, matrix-less transforms. Should be about ~38 multiplications rather than 64.
	auto& localNode = m_localTransform.at(entityID);
	auto parentNode = getWorldTransform(entityManager().entityParentID(entityID));
	globalNode.transform.position = parentNode.position + parentNode.rotation * (parentNode.scale * localNode.position);
	globalNode.transform.rotation = parentNode.rotation * localNode.rotation;
	globalNode.transform.scale    = parentNode.scale * localNode.scale;

	return globalNode.transform;
}

// ///////// //
// // Get // //
// ///////// //

akm::Mat4 TransformManager::localToWorld(EntityID entityID) const {
	auto cacheIter = m_globalTransform.find(entityID);
	if (cacheIter == m_globalTransform.end()) return akm::Mat4(1);
	return calculateTransformMatrix(getWorldTransform(entityID));
}

akm::Mat4 TransformManager::worldToLocal(EntityID entityID) const { return akm::inverse(localToWorld(entityID)); }

akm::Vec3 TransformManager::position(EntityID entityID) const { return getWorldTransform(entityID).position; }

akm::Mat4 TransformManager::rotationMatrix(EntityID entityID) const { return akm::mat3_cast(rotationQuat(entityID)); }
akm::Quat TransformManager::rotationQuat(  EntityID entityID) const { return getWorldTransform(entityID).rotation; }
akm::Vec3 TransformManager::rotationEuler( EntityID entityID) const { return akm::toEuler(rotationQuat(entityID)); }

fpSingle TransformManager::scale(EntityID entityID) const { return getWorldTransform(entityID).scale; }

akm::Vec3 TransformManager::rightward(EntityID entityID) const { return akm::extractAxisX(getWorldTransform(entityID).rotation); } //akm::column(rotationMatrix(entityID), 0); }
akm::Vec3 TransformManager::upward(   EntityID entityID) const { return akm::extractAxisY(getWorldTransform(entityID).rotation); } //akm::column(rotationMatrix(entityID), 1); }
akm::Vec3 TransformManager::forward(  EntityID entityID) const { return akm::extractAxisZ(getWorldTransform(entityID).rotation); } //akm::column(rotationMatrix(entityID), 2); }
akm::Vec3 TransformManager::leftward( EntityID entityID) const { return -rightward(entityID); }
akm::Vec3 TransformManager::downward( EntityID entityID) const { return -upward(entityID); }
akm::Vec3 TransformManager::backward( EntityID entityID) const { return -forward(entityID); }

// ///////// //
// // Set // //
// ///////// //

void TransformManager::setPosition(EntityID entityID, const akm::Vec3& p) {
	auto parentID = entityManager().entityParentID(entityID);
	auto parentInvTransform = worldToLocal(parentID);
	setLocalPosition(entityID, parentInvTransform * akm::Vec4(p, 1));
}

void TransformManager::setRotation(EntityID entityID, const akm::Mat4& r) {
	setRotation(entityID, akm::normalize(akm::quat_cast(r)));
}

void TransformManager::setRotation(EntityID entityID, const akm::Quat& r) {
	auto parentID = entityManager().entityParentID(entityID);
	auto parentRot = hasComponent(parentID) ? rotationQuat(parentID) : akm::Quat(1,0,0,0);
	setLocalRotation(entityID, akm::inverse(parentRot) * r);
}

void TransformManager::setRotation(EntityID entityID, const akm::Vec3& r) {
	setRotation(entityID, akm::fromEuler(r));
}

void TransformManager::setRotation(EntityID entityID, const akm::Vec3& f, const akm::Vec3& u) {
	auto rForward = akm::normalize(f);
	auto rUp = akm::normalize(u);
	auto rRight = akm::normalize(akm::cross(rUp, rForward));
	rUp = akm::normalize(akm::cross(rForward, rRight));

	setRotation(entityID, akm::quat_cast(akm::Mat3(rRight, rUp, rForward)));
}


void TransformManager::setScale(EntityID entityID, fpSingle s) {
	auto parentID = entityManager().entityParentID(entityID);
	auto parentScl = hasComponent(parentID) ? scale(parentID) : 1.f;
	setLocalScale(entityID, s/parentScl);
}

// /////////// //
// // Apply // //
// /////////// //

void TransformManager::move(EntityID entityID, const akm::Vec3& p) { setPosition(entityID, position(entityID) + p); }

void TransformManager::moveRightward(EntityID entityID, fpSingle dist) { move(entityID, rightward(entityID)*dist); }
void TransformManager::moveUpward(   EntityID entityID, fpSingle dist) { move(entityID,    upward(entityID)*dist); }
void TransformManager::moveForward(  EntityID entityID, fpSingle dist) { move(entityID,   forward(entityID)*dist); }
void TransformManager::moveLeftward( EntityID entityID, fpSingle dist) { move(entityID,  leftward(entityID)*dist); }
void TransformManager::moveDownward( EntityID entityID, fpSingle dist) { move(entityID,  downward(entityID)*dist); }
void TransformManager::moveBackward( EntityID entityID, fpSingle dist) { move(entityID,  backward(entityID)*dist); }

void TransformManager::rotate(EntityID entityID, fpSingle angle, const akm::Vec3& axis) { rotate(entityID, akm::rotateQ(angle, axis)); }
void TransformManager::rotate(EntityID entityID, const akm::Mat4& r) { rotate(entityID, akm::quat_cast(r)); }
void TransformManager::rotate(EntityID entityID, const akm::Quat& r) { setRotation(entityID, rotationQuat(entityID) * r); }
void TransformManager::rotate(EntityID entityID, const akm::Vec3& r) { rotate(entityID, akm::fromEuler(r)); }

void TransformManager::rotatePre(EntityID entityID, fpSingle angle, const akm::Vec3& axis) { rotatePre(entityID, akm::rotateQ(angle, axis)); }
void TransformManager::rotatePre(EntityID entityID, const akm::Mat4& r) { rotatePre(entityID, akm::quat_cast(r)); }
void TransformManager::rotatePre(EntityID entityID, const akm::Quat& r) { setRotation(entityID, r * rotationQuat(entityID)); }
void TransformManager::rotatePre(EntityID entityID, const akm::Vec3& r) { rotatePre(entityID, akm::fromEuler(r)); }

void TransformManager::scaleByFactor(EntityID entityID, fpSingle s) { setScale(entityID, scale(entityID) * s); }

// ///////// //
// // Get // //
// ///////// //

akm::Mat4 TransformManager::localToParent(EntityID entityID) const { return calculateTransformMatrix(m_localTransform.at(entityID)); }

akm::Mat4 TransformManager::parentToLocal(EntityID entityID) const { return akm::inverse(localToParent(entityID)); }

akm::Vec3 TransformManager::localPosition(EntityID entityID) const { return m_localTransform.at(entityID).position; }

akm::Mat4 TransformManager::localRotationMatrix(EntityID entityID) const { return akm::mat4_cast(localRotationQuat(entityID)); }
akm::Quat TransformManager::localRotationQuat(  EntityID entityID) const { return m_localTransform.at(entityID).rotation; }
akm::Vec3 TransformManager::localRotationEuler( EntityID entityID) const { return akm::toEuler(localRotationQuat(entityID)); }

fpSingle TransformManager::localScale(EntityID entityID) const { return m_localTransform.at(entityID).scale; }

akm::Vec3 TransformManager::localRightward(EntityID entityID) const { return akm::extractAxisX(m_localTransform.at(entityID).rotation); }
akm::Vec3 TransformManager::localUpward(   EntityID entityID) const { return akm::extractAxisY(m_localTransform.at(entityID).rotation); }
akm::Vec3 TransformManager::localForward(  EntityID entityID) const { return akm::extractAxisZ(m_localTransform.at(entityID).rotation); }
akm::Vec3 TransformManager::localLeftward( EntityID entityID) const { return -localRightward(entityID); }
akm::Vec3 TransformManager::localDownward( EntityID entityID) const { return -localUpward(entityID); }
akm::Vec3 TransformManager::localBackward( EntityID entityID) const { return -localForward(entityID); }

// ///////// //
// // Set // //
// ///////// //

void TransformManager::setLocalPosition(EntityID entityID, const akm::Vec3& p){
	m_localTransform.at(entityID).position = p;
	markDirty(entityID);
}

void TransformManager::setLocalRotation(EntityID entityID, const akm::Mat4& r) { setLocalRotation(entityID, akm::quat_cast(r)); }
void TransformManager::setLocalRotation(EntityID entityID, const akm::Quat& r) { m_localTransform.at(entityID).rotation = r; markDirty(entityID); }
void TransformManager::setLocalRotation(EntityID entityID, const akm::Vec3& r) { setLocalRotation(entityID, akm::fromEuler(r)); }
void TransformManager::setLocalRotation(EntityID entityID, const akm::Vec3& f, const akm::Vec3& u) {
	auto rForward = akm::normalize(f);
	auto rUp = akm::normalize(u);
	auto rRight = akm::normalize(akm::cross(rUp, rForward));
	rUp = akm::normalize(akm::cross(rForward, rRight));

	setLocalRotation(entityID, akm::quat_cast(akm::Mat3(rRight, rUp, rForward)));
}

void TransformManager::setLocalScale(EntityID entityID, fpSingle s) {
	m_localTransform.at(entityID).scale = s;
	markDirty(entityID);
}

// /////////// //
// // Apply // //
// /////////// //

void TransformManager::moveLocal(EntityID entityID, const akm::Vec3& p) { setLocalPosition(entityID, localPosition(entityID) + p); }

void TransformManager::moveLocalRightward(EntityID entityID, fpSingle dist) { moveLocal(entityID, localRightward(entityID)*dist); }
void TransformManager::moveLocalUpward(   EntityID entityID, fpSingle dist) { moveLocal(entityID,    localUpward(entityID)*dist); }
void TransformManager::moveLocalForward(  EntityID entityID, fpSingle dist) { moveLocal(entityID,   localForward(entityID)*dist); }
void TransformManager::moveLocalLeftward( EntityID entityID, fpSingle dist) { moveLocal(entityID,  localLeftward(entityID)*dist); }
void TransformManager::moveLocalDownward( EntityID entityID, fpSingle dist) { moveLocal(entityID,  localDownward(entityID)*dist); }
void TransformManager::moveLocalBackward( EntityID entityID, fpSingle dist) { moveLocal(entityID,  localBackward(entityID)*dist); }

void TransformManager::rotateLocal(EntityID entityID, fpSingle angle, const akm::Vec3& axis) { rotateLocal(entityID, akm::rotateQ(angle, axis)); }
void TransformManager::rotateLocal(EntityID entityID, const akm::Mat4& r) { rotateLocal(entityID, akm::quat_cast(r)); }
void TransformManager::rotateLocal(EntityID entityID, const akm::Quat& r) { setLocalRotation(entityID, rotationQuat(entityID) * r); }
void TransformManager::rotateLocal(EntityID entityID, const akm::Vec3& r) { rotateLocal(entityID, akm::fromEuler(r)); }

void TransformManager::scaleLocalByFactor(EntityID entityID, fpSingle s) { setLocalScale(entityID, localScale(entityID) * s); }

// //////////////// //
// // Components // //
// //////////////// //

Transform TransformManager::component(EntityID entityID) { return Transform(*this, entityID); }
const Transform TransformManager::component(EntityID entityID) const { return Transform(const_cast<TransformManager&>(*this), entityID); }

bool TransformManager::hasComponent(EntityID entityID) const { return m_localTransform.find(entityID) != m_localTransform.end(); }




// /////////////// //
// // Transform // //
// /////////////// //

Transform::Transform(TransformManager& manager, EntityID id) : m_manager(&manager), m_id(id) {}

// ///////// //
// // Get // //
// ///////// //

akm::Mat4 Transform::localToWorld() const { return m_manager->localToWorld(m_id); }
akm::Mat4 Transform::worldToLocal() const { return m_manager->worldToLocal(m_id); }

akm::Vec3 Transform::position() const { return m_manager->position(m_id); }

akm::Mat4 Transform::rotationMatrix() const { return m_manager->rotationMatrix(m_id); }
akm::Quat Transform::rotationQuat() const { return m_manager->rotationQuat(m_id); }
akm::Vec3 Transform::rotationEuler() const { return m_manager->rotationEuler(m_id); }

fpSingle Transform::scale() const { return m_manager->scale(m_id); }

akm::Vec3 Transform::rightward() const { return m_manager->rightward(m_id); }
akm::Vec3 Transform::upward()    const { return m_manager->upward(m_id);    }
akm::Vec3 Transform::forward()   const { return m_manager->forward(m_id);   }
akm::Vec3 Transform::leftward()  const { return m_manager->leftward(m_id);  }
akm::Vec3 Transform::downward()  const { return m_manager->downward(m_id);  }
akm::Vec3 Transform::backward()  const { return m_manager->backward(m_id);  }

// ///////// //
// // Set // //
// ///////// //

Transform& Transform::setPosition(const akm::Vec3& p) { m_manager->setPosition(m_id, p); return *this; }

Transform& Transform::setRotation(const akm::Mat4& r) { m_manager->setRotation(m_id, r); return *this; }
Transform& Transform::setRotation(const akm::Quat& r) { m_manager->setRotation(m_id, r); return *this; }
Transform& Transform::setRotation(const akm::Vec3& r) { m_manager->setRotation(m_id, r); return *this; }
Transform& Transform::setRotation(const akm::Vec3& f, const akm::Vec3& u) { m_manager->setRotation(m_id, f, u); return *this; }

Transform& Transform::setScale(fpSingle s) { m_manager->setScale(m_id, s); return *this; }

// /////////// //
// // Apply // //
// /////////// //

Transform& Transform::move(const akm::Vec3& p) { m_manager->move(m_id, p); return *this; }
Transform& Transform::moveRightward(fpSingle dist) { m_manager->moveRightward(m_id, dist); return *this; }
Transform& Transform::moveUpward(   fpSingle dist) { m_manager->moveUpward(   m_id, dist); return *this; }
Transform& Transform::moveForward(  fpSingle dist) { m_manager->moveForward(  m_id, dist); return *this; }
Transform& Transform::moveLeftward( fpSingle dist) { m_manager->moveLeftward( m_id, dist); return *this; }
Transform& Transform::moveDownward( fpSingle dist) { m_manager->moveDownward( m_id, dist); return *this; }
Transform& Transform::moveBackward( fpSingle dist) { m_manager->moveBackward( m_id, dist); return *this; }

Transform& Transform::rotate(fpSingle angle, const akm::Vec3& axis) { m_manager->rotate(m_id, angle, axis); return *this; }
Transform& Transform::rotate(const akm::Mat4& r) { m_manager->rotate(m_id, r); return *this; }
Transform& Transform::rotate(const akm::Quat& r) { m_manager->rotate(m_id, r); return *this; }
Transform& Transform::rotate(const akm::Vec3& r) { m_manager->rotate(m_id, r); return *this; }

Transform& Transform::scaleByFactor(fpSingle s) { m_manager->scaleByFactor(m_id, s); return *this; }

// ///////// //
// // Get // //
// ///////// //

akm::Mat4 Transform::localToParent() const { return m_manager->localToParent(m_id); }
akm::Mat4 Transform::parentToLocal() const { return m_manager->parentToLocal(m_id); }

akm::Vec3 Transform::localPosition() const { return m_manager->localPosition(m_id); }

akm::Mat4 Transform::localRotationMatrix() const { return m_manager->localRotationMatrix(m_id); }
akm::Quat Transform::localRotationQuat() const { return m_manager->localRotationQuat(m_id); }
akm::Vec3 Transform::localRotationEuler() const { return m_manager->localRotationEuler(m_id); }

fpSingle Transform::localScale() const { return m_manager->localScale(m_id); }

akm::Vec3 Transform::localRightward() const { return m_manager->localRightward(m_id); }
akm::Vec3 Transform::localUpward()    const { return m_manager->localUpward(m_id);    }
akm::Vec3 Transform::localForward()   const { return m_manager->localForward(m_id);   }
akm::Vec3 Transform::localLeftward()  const { return m_manager->localLeftward(m_id);  }
akm::Vec3 Transform::localDownward()  const { return m_manager->localDownward(m_id);  }
akm::Vec3 Transform::localBackward()  const { return m_manager->localBackward(m_id);  }

// ///////// //
// // Set // //
// ///////// //

Transform& Transform::setLocalPosition(const akm::Vec3& p) { m_manager->setPosition(m_id, p); return *this; }

Transform& Transform::setLocalRotation(const akm::Mat4& r) { m_manager->setLocalRotation(m_id, r); return *this; }
Transform& Transform::setLocalRotation(const akm::Quat& r) { m_manager->setLocalRotation(m_id, r); return *this; }
Transform& Transform::setLocalRotation(const akm::Vec3& r) { m_manager->setLocalRotation(m_id, r); return *this; }
Transform& Transform::setLocalRotation(const akm::Vec3& f, const akm::Vec3& u) { m_manager->setLocalRotation(m_id, f, u); return *this; }

Transform& Transform::setLocalScale(fpSingle s) { m_manager->setLocalScale(m_id, s); return *this; }

// /////////// //
// // Apply // //
// /////////// //

Transform& Transform::moveLocal(const akm::Vec3& p) { m_manager->moveLocal(m_id, p); return *this; }
Transform& Transform::moveLocalRightward(fpSingle dist) { m_manager->moveLocalRightward(m_id, dist); return *this; }
Transform& Transform::moveLocalUpward(   fpSingle dist) { m_manager->moveLocalUpward(   m_id, dist); return *this; }
Transform& Transform::moveLocalForward(  fpSingle dist) { m_manager->moveLocalForward(  m_id, dist); return *this; }
Transform& Transform::moveLocalLeftward( fpSingle dist) { m_manager->moveLocalLeftward( m_id, dist); return *this; }
Transform& Transform::moveLocalDownward( fpSingle dist) { m_manager->moveLocalDownward( m_id, dist); return *this; }
Transform& Transform::moveLocalBackward( fpSingle dist) { m_manager->moveLocalBackward( m_id, dist); return *this; }

Transform& Transform::rotateLocal(fpSingle angle, const akm::Vec3& axis) { m_manager->rotateLocal(m_id, angle, axis); return *this; }
Transform& Transform::rotateLocal(const akm::Mat4& r) { m_manager->rotateLocal(m_id, r); return *this; }
Transform& Transform::rotateLocal(const akm::Quat& r) { m_manager->rotateLocal(m_id, r); return *this; }
Transform& Transform::rotateLocal(const akm::Vec3& r) { m_manager->rotateLocal(m_id, r); return *this; }

Transform& Transform::scaleLocalByFactor(fpSingle s) { m_manager->scaleLocalByFactor(m_id, s); return *this; }

// //////////////// //
// // Components // //
// //////////////// //

EntityID Transform::id() const { return m_id; }

