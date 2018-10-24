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

#include <akcommon/UnorderedVector.hpp>
#include <akengine/entity/EntityManager.hpp>
#include <akengine/entity/internal/EntityGraph.hpp>
#include <akmath/Matrix.hpp>
#include <akmath/Quaternion.hpp>
#include <akmath/Vector.hpp>
#include <glm/gtc/quaternion.hpp>
#include <deque>
#include <stdexcept>
#include <utility>


using namespace ake;

// ///////////// //
// // Manager // //
// ///////////// //

akm::Mat4x3 EntityGraph::calculateTransformMatrix(const TransformNode& transform) {
	auto rotationMatrix = akm::mat3_cast(transform.rotation);
	return akm::Mat4x3(
		{akm::column(rotationMatrix, 0) * transform.scale.x},
		{akm::column(rotationMatrix, 1) * transform.scale.y},
		{akm::column(rotationMatrix, 2) * transform.scale.z},
		{transform.position}
	);
}

void EntityGraph::markDirty(EntityID id) const {
	std::deque<EntityID> dirtyList({id});
	while(!dirtyList.empty()) {
		// If the parent node is already dirty it's children should already be flagged.
		if (!std::exchange(m_transformsGlobal.at(dirtyList.front()).isDirty, true)) {
			auto& childList = children(dirtyList.front());
			dirtyList.insert(dirtyList.end(), childList.begin(), childList.end());
		}
		dirtyList.pop_front();
	}
}

EntityGraph::TransformNode EntityGraph::getLocalTransform(EntityID entityID) const {
	if (entityID == akc::SlotID()) return {akm::Vec3(0,0,0), akm::Quat(1,0,0,0), akm::Vec3(1,1,1), false, akm::Mat4x3(1)};
	auto& localNode = m_transformsLocal.at(entityID);
	if (std::exchange(localNode.isDirty, false)) localNode.cache = calculateTransformMatrix(localNode);
	return localNode;
}

EntityGraph::TransformNode EntityGraph::getGlobalTransform(EntityID entityID) const {
	if (entityID == akc::SlotID()) return {akm::Vec3(0,0,0), akm::Quat(1,0,0,0), akm::Vec3(1,1,1), false, akm::Mat4x3(1)};

	auto& globalNode = m_transformsGlobal.at(entityID);
	if (std::exchange(globalNode.isDirty, false)) {
		auto localNode  = getLocalTransform(entityID);
		auto parentNode = getGlobalTransform(parent(entityID));
		globalNode.position = parentNode.position + parentNode.rotation * (parentNode.scale * localNode.position);
		globalNode.rotation = parentNode.rotation * localNode.rotation;
		globalNode.scale    = parentNode.scale    * localNode.scale;
		globalNode.cache = calculateTransformMatrix(globalNode);
	}

	return globalNode;
}


// ////////////// //
// // Register // //
// ////////////// //

void EntityGraph::registerEntity(EntityID entityID, EntityID parentID, const akm::Vec3& position, const akm::Quat& rotation, const akm::Vec3& scale, bool worldSpace) {
	if (!parentID) m_graphRoot.insert(entityID);
	m_graphStorage.insert(GraphNode{parentID, {}});
	m_transformsLocal.insert({position, rotation, scale, true, akm::Mat4x3{1}});
	m_transformsGlobal.insert({position, rotation, scale, true, akm::Mat4x3{1}});
	if (!worldSpace) return;
	setPosition(entityID, position);
	setRotation(entityID, rotation);
	setScale(entityID, scale);
}

void EntityGraph::unregisterEntity(EntityID entityID) {
	m_graphRoot.erase(entityID);
	m_graphStorage.erase(entityID);
	m_transformsLocal.erase(entityID);
	m_transformsGlobal.erase(entityID);
}

// /////////// //
// // Graph // //
// /////////// //
bool EntityGraph::setParent(EntityID entityID, EntityID newParent) {
	auto& childEntry = m_graphStorage.at(entityID);
	if (childEntry.parent == newParent) return true;

	// Cycle detection
	EntityID cParentID = newParent;
	while(cParentID.isValid()) {
		if (cParentID == entityID) return false;
		cParentID = m_graphStorage.at(cParentID).parent;
	}

	// Store old parentID
	// auto oldParent = childEntry.parent;

	// Change parent
	auto& parentEntry = m_graphStorage.at(childEntry.parent);
	parentEntry.children.erase(aku::find(parentEntry.children, entityID));
	childEntry.parent = newParent;
	if (newParent) m_graphStorage.at(newParent).children.insert(entityID);
	else m_graphRoot.insert(entityID);

	// Send update notification
	// sendChangeEvent(entityID, oldParent, newParent);
	markDirty(entityID);

	return true;
}

EntityID EntityGraph::parent(EntityID entityID) const {
	return m_graphStorage.at(entityID).parent;
}

const akc::UnorderedVector<EntityID>& EntityGraph::children(EntityID entityID) const {
	return m_graphStorage.at(entityID).children;
}

EntityID EntityGraph::findFirstNamed(EntityID baseID, const std::string& name) const {
	auto findFirstNamed = [&](const auto& childIDs, const std::string& childName) {
		for(auto childID : childIDs) if (m_entityManager.entityName(childID) == childName) return childID;
		return EntityID();
	};
	return baseID ? findFirstNamed(children(baseID), name) : findFirstNamed(m_graphRoot, name);
}

akc::UnorderedVector<EntityID> EntityGraph::findAllNamed(EntityID baseID, const std::string& name) const {
	auto findAllNamed = [&](const auto& childIDs, const std::string& childName) {
		akc::UnorderedVector<EntityID> result;
		for(auto childID : childIDs) if (m_entityManager.entityName(childID) == childName) result.insert(childID);
		return result;
	};
	return baseID ? findAllNamed(children(baseID), name) : findAllNamed(m_graphRoot, name);
}

// ////////////////////// //
// // Global Transform // //
// ////////////////////// //
akm::Mat4x3 EntityGraph::localToWorld(EntityID entityID) const { return getGlobalTransform(entityID).cache; }
akm::Mat4x3 EntityGraph::worldToLocal(EntityID entityID) const { return akm::inverse(localToWorld(entityID)); }

akm::Vec3 EntityGraph::position(EntityID entityID) const { return getGlobalTransform(entityID).position; }
akm::Quat EntityGraph::rotation(EntityID entityID) const { return getGlobalTransform(entityID).rotation; }
akm::Vec3 EntityGraph::scale(EntityID entityID) const { return getGlobalTransform(entityID).scale; }

akm::Vec3 EntityGraph::rightward(EntityID entityID) const { return akm::extractAxisX(getGlobalTransform(entityID).rotation); }
akm::Vec3 EntityGraph::upward(   EntityID entityID) const { return akm::extractAxisY(getGlobalTransform(entityID).rotation); }
akm::Vec3 EntityGraph::forward(  EntityID entityID) const { return akm::extractAxisZ(getGlobalTransform(entityID).rotation); }
akm::Vec3 EntityGraph::leftward( EntityID entityID) const { return -rightward(entityID); }
akm::Vec3 EntityGraph::downward( EntityID entityID) const { return -upward(entityID); }
akm::Vec3 EntityGraph::backward( EntityID entityID) const { return -forward(entityID); }

void EntityGraph::setPosition(EntityID entityID, const akm::Vec3& p) {
	setLocalPosition(entityID, worldToLocal(parent(entityID)) * akm::Vec4(p, 1));
}

void EntityGraph::setRotation(EntityID entityID, const akm::Quat& r) {
	setLocalRotation(entityID, akm::inverse(rotation(parent(entityID))) * r);
}

void EntityGraph::setRotation(EntityID entityID, const akm::Vec3& forwardNormal, const akm::Vec3& upNormal) {
	auto rightNormal = akm::normalize(akm::cross(upNormal, forwardNormal));
	setRotation(entityID, akm::quat_cast(akm::Mat3(rightNormal, akm::normalize(akm::cross(forwardNormal, rightNormal)), forwardNormal)));
}

void EntityGraph::setScale(EntityID entityID, const akm::Vec3& s) {
	setLocalScale(entityID, s/scale(parent(entityID)));
}

void EntityGraph::move(EntityID entityID, const akm::Vec3& p) { setPosition(entityID, position(entityID) + p); }
void EntityGraph::rotate(EntityID entityID, fpSingle angle, const akm::Vec3& axis) { rotate(entityID, akm::rotateQ(angle, axis)); }
void EntityGraph::rotate(EntityID entityID, const akm::Quat& r) { setRotation(entityID, rotation(entityID) * r); }
void EntityGraph::scale(EntityID entityID, const akm::Vec3& s) { setScale(entityID, scale(entityID) * s); }

// ///////////////////// //
// // Local Transform // //
// ///////////////////// //
akm::Mat4x3 EntityGraph::localToParent(EntityID entityID) const { return getLocalTransform(entityID).cache; }
akm::Mat4x3 EntityGraph::parentToLocal(EntityID entityID) const { return akm::inverse(localToParent(entityID)); }

akm::Vec3 EntityGraph::localPosition(EntityID entityID) const { return m_transformsLocal.at(entityID).position; }
akm::Quat EntityGraph::localRotation(EntityID entityID) const { return m_transformsLocal.at(entityID).rotation; }
akm::Vec3 EntityGraph::localScale(EntityID entityID) const { return m_transformsLocal.at(entityID).scale; }

akm::Vec3 EntityGraph::localRightward(EntityID entityID) const { return akm::extractAxisX(m_transformsLocal.at(entityID).rotation); }
akm::Vec3 EntityGraph::localUpward(   EntityID entityID) const { return akm::extractAxisY(m_transformsLocal.at(entityID).rotation); }
akm::Vec3 EntityGraph::localForward(  EntityID entityID) const { return akm::extractAxisZ(m_transformsLocal.at(entityID).rotation); }
akm::Vec3 EntityGraph::localLeftward( EntityID entityID) const { return -localRightward(entityID); }
akm::Vec3 EntityGraph::localDownward( EntityID entityID) const { return -localUpward(entityID); }
akm::Vec3 EntityGraph::localBackward( EntityID entityID) const { return -localForward(entityID); }

void EntityGraph::setLocalPosition(EntityID entityID, const akm::Vec3& p){
	auto& localNode = m_transformsLocal.at(entityID);
	localNode.position = p; localNode.isDirty = true;
	markDirty(entityID);
}

void EntityGraph::setLocalRotation(EntityID entityID, const akm::Quat& r) {
	auto& localNode = m_transformsLocal.at(entityID);
	localNode.rotation = r; localNode.isDirty = true;
	markDirty(entityID);
}

void EntityGraph::setLocalRotation(EntityID entityID, const akm::Vec3& forwardNormal, const akm::Vec3& upwardNormal) {
	auto rightNormal = akm::normalize(akm::cross(upwardNormal, forwardNormal));
	setLocalRotation(entityID, akm::quat_cast(akm::Mat3(rightNormal, akm::normalize(akm::cross(forwardNormal, rightNormal)), forwardNormal)));
}

void EntityGraph::setLocalScale(EntityID entityID, const akm::Vec3& s) {
	auto& localNode = m_transformsLocal.at(entityID);
	localNode.scale = s; localNode.isDirty = true;
	markDirty(entityID);
}

void EntityGraph::localMove(EntityID entityID, const akm::Vec3& p) { setLocalPosition(entityID, localPosition(entityID) + akm::Mat3(localRotation(entityID)) * p); }
void EntityGraph::localRotate(EntityID entityID, fpSingle angle, const akm::Vec3& axis) { localRotate(entityID, akm::rotateQ(angle, axis)); }
void EntityGraph::localRotate(EntityID entityID, const akm::Quat& r) { setLocalRotation(entityID, rotation(entityID) * r); }
void EntityGraph::localScale(EntityID entityID, const akm::Vec3& s) { setLocalScale(entityID, localScale(entityID) * s); }
