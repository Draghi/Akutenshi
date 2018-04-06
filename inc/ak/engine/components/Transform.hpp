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
#include <ak/container/UnorderedVector.hpp>
#include <ak/engine/components/SceneGraph.hpp>
#include <ak/engine/ECS.hpp>
#include <ak/event/Dispatcher.hpp>
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
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace ake {

	class TransformManager;

	class Transform {
		AKE_DEFINE_COMPONENT(TransformManager, Transform)
		private:
			TransformManager& m_manager;
			EntityID m_id;

		public:
			Transform(TransformManager& manager, EntityID id);

			// /////////// //
			// // World // //
			// /////////// //
			akm::Mat4 transform() const;

			akm::Vec3  position() const;
			akm::Quat  rotation() const;
			akm::scalar_t scale() const;

			akm::Mat4 translationMatrix() const;
			akm::Mat3    rotationMatrix() const;
			akm::Mat3       scaleMatrix() const;

			akm::Vec3      up() const;
			akm::Vec3   right() const;
			akm::Vec3 forward() const;

			void setPosition(const akm::Vec3& pos);
			void setRotation(const akm::Quat& rot);
			void setScale(const akm::scalar_t& scl);

			// /////////// //
			// // Local // //
			// /////////// //
			akm::Mat4 localTransform() const;

			akm::Vec3  localPosition() const;
			akm::Quat  localRotation() const;
			akm::scalar_t localScale() const;

			akm::Mat4 localTranslationMatrix() const;
			akm::Mat3    localRotationMatrix() const;
			akm::Mat3       localScaleMatrix() const;

			akm::Vec3      localUp() const;
			akm::Vec3   localRight() const;
			akm::Vec3 localForward() const;

			void setLocalPosition(const akm::Vec3& pos);
			void setLocalRotation(const akm::Quat& rot);
			void setLocalScale(const akm::scalar_t& scl);

			// /////////// //
			// // Other // //
			// /////////// //
			void lookAt(const akm::Vec3& forward, const akm::Vec3& up);

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

			mutable const SceneGraphManager* m_sceneGraph = nullptr;
			const SceneGraphManager& sceneGraph() const {
				if (!m_sceneGraph) {
					m_sceneGraph = &entityManager().componentManager<SceneGraph>();
					m_sceneGraph->sceneGraphChanged().subscribe([&](const auto& ev){
						if (hasComponent(ev.data().modifiedEntity().id())) markDirty(ev.data().modifiedEntity().id());
					});
				}
				return *m_sceneGraph;
			}

			void markDirty(EntityID id) const {
				m_cache.at(id).isLocalDirty = true;

				std::deque<EntityID> dirtyList({id});
				while(!dirtyList.empty()) {
					// If the parent node is already dirty it's children should already be flagged.
					if (!std::exchange(m_cache.at(dirtyList.front()).isGlobalDirty, true)) {
						auto& children = sceneGraph().children(dirtyList.front());
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

			// /////////// //
			// // World // //
			// /////////// //

			akm::Mat4 transform(EntityID id) const {
				auto cacheIter = m_cache.find(id);
				if (cacheIter == m_cache.end()) return akm::Mat4(1);
				if (std::exchange(cacheIter->second.isGlobalDirty, false)) cacheIter->second.globalTransform = transform(sceneGraph().parent(id)) * localTransform(id);
				return cacheIter->second.globalTransform;
			}

			akm::Vec3  position(EntityID id) const { return akm::Vec3(akm::column(transform(id), 3)); }
			akm::Quat  rotation(EntityID id) const { return akm::quat_cast(rotationMatrix(id)); }
			akm::scalar_t scale(EntityID id) const { return akm::magnitude(akm::column(transform(id), 0)); }

			akm::Mat4 translationMatrix(EntityID id) const { return akm::translate(position(id)); }
			akm::Mat3    rotationMatrix(EntityID id) const { return akm::Mat3(transform(id))/scale(id); }
			akm::Mat3       scaleMatrix(EntityID id) const { auto scl = scale(id); return akm::scale3({scl, scl, scl}); }

			akm::Vec3   right(EntityID id) const { return akm::column(rotationMatrix(id), 0); }
			akm::Vec3      up(EntityID id) const { return akm::column(rotationMatrix(id), 1); }
			akm::Vec3 forward(EntityID id) const { return akm::column(rotationMatrix(id), 2); }

			void setPosition(EntityID id, const akm::Vec3& pos) {
				auto parentID = sceneGraph().parent(id);
				auto parentInvTransform = hasComponent(parentID) ? akm::inverse(transform(parentID)) : akm::Mat4(1);
				setLocalPosition(id, parentInvTransform * akm::Vec4(pos, 1));
			}

			void setRotation(EntityID id, const akm::Quat& rot) {
				auto parentID = sceneGraph().parent(id);
				auto parentRot = hasComponent(parentID) ? rotation(parentID) : akm::Quat(1,0,0,0);
				setLocalRotation(id, akm::inverse(parentRot) * rot);
			}

			void setScale(EntityID id, const akm::scalar_t& scl) {
				auto parentID = sceneGraph().parent(id);
				auto parentScl = hasComponent(parentID) ? scale(parentID) : 1.f;
				setLocalScale(id, scl/parentScl);
			}

			// /////////// //
			// // Local // //
			// /////////// //

			akm::Mat4 localTransform(EntityID id) const {
				auto& cacheEntry = m_cache.at(id);
				if (std::exchange(cacheEntry.isLocalDirty, false)) cacheEntry.localTransform = calculateTransformMatrix(m_transform.at(id));
				return cacheEntry.localTransform;
			}

			akm::Vec3  localPosition(EntityID id) const { return m_transform.at(id).position; }
			akm::Quat  localRotation(EntityID id) const { return m_transform.at(id).rotation; }
			akm::scalar_t localScale(EntityID id) const { return m_transform.at(id).scale; }

			akm::Mat4 localTranslationMatrix(EntityID id) const { return akm::translate(localPosition(id)); }
			akm::Mat3    localRotationMatrix(EntityID id) const { auto invScl = 1.f/m_transform.at(id).scale; return akm::scale({invScl,invScl,invScl}) * localTransform(id); }
			akm::Mat3       localScaleMatrix(EntityID id) const { auto scl = m_transform.at(id).scale; return akm::scale({scl,scl,scl}); }

			akm::Vec3   localRight(EntityID id) const { return akm::column(rotationMatrix(id), 0); }
			akm::Vec3      localUp(EntityID id) const { return akm::column(rotationMatrix(id), 1); }
			akm::Vec3 localForward(EntityID id) const { return akm::column(rotationMatrix(id), 2); }

			void setLocalPosition(EntityID id, const akm::Vec3& pos) {
				m_transform.at(id).position = pos;
				markDirty(id);
			}

			void setLocalRotation(EntityID id, const akm::Quat& rot) {
				m_transform.at(id).rotation = rot;
				markDirty(id);
			}

			void setLocalScale(EntityID id, const akm::scalar_t& scl) {
				m_transform.at(id).scale = scl;
				markDirty(id);
			}

			// /////////// //
			// // Other // //
			// /////////// //

			void lookAt(EntityID id, const akm::Vec3& forward, const akm::Vec3& up) {
				auto rForward = akm::normalize(forward);
				auto rUp = akm::normalize(up);
				auto rRight = akm::normalize(akm::cross(rUp, rForward));
				rUp = akm::normalize(akm::cross(rForward, rRight));

				setRotation(id, akm::quat_cast(akm::Mat3(rRight, rUp, rForward)));
			}

			// //////////////// //
			// // Components // //
			// //////////////// //

			Transform component(EntityID entityID) { return Transform(*this, entityID); }
			const Transform component(EntityID entityID) const { return Transform(const_cast<TransformManager&>(*this), entityID); }

			bool hasComponent(EntityID entityID) const override { return m_transform.find(entityID) != m_transform.end(); }
	};

}

namespace ake {
	Transform::Transform(TransformManager& manager, EntityID id) : m_manager(manager), m_id(id) {}

	// /////////// //
	// // World // //
	// /////////// //
	akm::Mat4 Transform::transform() const { return m_manager.transform(m_id); }

	akm::Vec3 Transform::position() const { return m_manager.position(m_id); }
	akm::Quat Transform::rotation() const { return m_manager.rotation(m_id); }
	akm::scalar_t Transform::scale() const { return m_manager.scale(m_id); }

	akm::Mat4 Transform::translationMatrix() const { return m_manager.translationMatrix(m_id); }
	akm::Mat3 Transform::rotationMatrix() const { return m_manager.rotationMatrix(m_id); }
	akm::Mat3 Transform::scaleMatrix() const { return m_manager.scaleMatrix(m_id); }

	akm::Vec3 Transform::up() const { return m_manager.up(m_id); }
	akm::Vec3 Transform::right() const { return m_manager.right(m_id); }
	akm::Vec3 Transform::forward() const { return m_manager.forward(m_id); }

	void Transform::setPosition(const akm::Vec3& pos)  { m_manager.setPosition(m_id, pos); }
	void Transform::setRotation(const akm::Quat& rot)  { m_manager.setRotation(m_id, rot); }
	void Transform::setScale(const akm::scalar_t& scl) { m_manager.setScale(m_id, scl); }

	// /////////// //
	// // Local // //
	// /////////// //
	akm::Mat4 Transform::localTransform() const { return m_manager.localTransform(m_id); }

	akm::Vec3 Transform::localPosition() const { return m_manager.localPosition(m_id); }
	akm::Quat Transform::localRotation() const { return m_manager.localRotation(m_id); }
	akm::scalar_t Transform::localScale() const { return m_manager.localScale(m_id); }

	akm::Mat4 Transform::localTranslationMatrix() const { return m_manager.localTranslationMatrix(m_id); }
	akm::Mat3 Transform::localRotationMatrix() const { return m_manager.localRotationMatrix(m_id); }
	akm::Mat3 Transform::localScaleMatrix() const { return m_manager.localScaleMatrix(m_id); }

	akm::Vec3 Transform::localUp() const { return m_manager.localUp(m_id); }
	akm::Vec3 Transform::localRight() const { return m_manager.localRight(m_id); }
	akm::Vec3 Transform::localForward() const { return m_manager.localForward(m_id); }

	void Transform::setLocalPosition(const akm::Vec3& pos)  { m_manager.setLocalPosition(m_id, pos); }
	void Transform::setLocalRotation(const akm::Quat& rot)  { m_manager.setLocalRotation(m_id, rot); }
	void Transform::setLocalScale(const akm::scalar_t& scl) { m_manager.setLocalScale(m_id, scl); }

	// /////////// //
	// // Other // //
	// /////////// //
	void Transform::lookAt(const akm::Vec3& forward, const akm::Vec3& up)  { m_manager.lookAt(m_id, forward, up); }

	// /////////// //
	// // Other // //
	// /////////// //
	EntityID Transform::id() const { return m_id; }
}

#endif
