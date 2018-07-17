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

#ifndef AK_ENGINE_COMPONENTS_TRANSFORM_HPP_
#define AK_ENGINE_COMPONENTS_TRANSFORM_HPP_

#include <unordered_map>

#include <ak/container/SlotMap.hpp>
#include <ak/engine/ComponentManager.hpp>
#include <ak/engine/Type.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>

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

			akm::Vec3 scale() const;

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

			Transform& setScale(const akm::Vec3& s);

			// /////////// //
			// // Apply // //
			// /////////// //

			Transform& move(const akm::Vec3& p);
			Transform& moveRightward(fpSingle dist);
			Transform& moveUpward(fpSingle dist);
			Transform& moveForward(fpSingle dist);
			Transform& moveLeftward(fpSingle dist);
			Transform& moveDownward(fpSingle dist);
			Transform& moveBackward(fpSingle dist);

			Transform& rotate(fpSingle angle, const akm::Vec3& axis);
			Transform& rotate(const akm::Mat4& r);
			Transform& rotate(const akm::Quat& r);
			Transform& rotate(const akm::Vec3& r);

			Transform& scaleByFactor(const akm::Vec3& s);

			// ///////// //
			// // Get // //
			// ///////// //

			akm::Mat4 localToParent() const;
			akm::Mat4 parentToLocal() const;

			akm::Vec3 localPosition() const;

			akm::Mat4 localRotationMatrix() const;
			akm::Quat localRotationQuat() const;
			akm::Vec3 localRotationEuler() const;

			akm::Vec3 localScale() const;

			akm::Vec3 localRightward() const;
			akm::Vec3 localUpward() const;
			akm::Vec3 localForward() const;
			akm::Vec3 localLeftward() const;
			akm::Vec3 localDownward() const;
			akm::Vec3 localBackward() const;

			// ///////// //
			// // Set // //
			// ///////// //

			Transform& setLocalPosition(const akm::Vec3& p);

			Transform& setLocalRotation(const akm::Mat4& r);
			Transform& setLocalRotation(const akm::Quat& r);
			Transform& setLocalRotation(const akm::Vec3& r);
			Transform& setLocalRotation(const akm::Vec3& forward, const akm::Vec3& upward);

			Transform& setLocalScale(const akm::Vec3& s);

			// /////////// //
			// // Apply // //
			// /////////// //

			Transform& moveLocal(const akm::Vec3& p);
			Transform& moveLocalRightward(fpSingle dist);
			Transform& moveLocalUpward(fpSingle dist);
			Transform& moveLocalForward(fpSingle dist);
			Transform& moveLocalLeftward(fpSingle dist);
			Transform& moveLocalDownward(fpSingle dist);
			Transform& moveLocalBackward(fpSingle dist);

			Transform& rotateLocal(fpSingle angle, const akm::Vec3& axis);
			Transform& rotateLocal(const akm::Mat4& r);
			Transform& rotateLocal(const akm::Quat& r);
			Transform& rotateLocal(const akm::Vec3& r);

			Transform& scaleLocalByFactor(const akm::Vec3& s);

			// //////////////// //
			// // Components // //
			// //////////////// //

			EntityID id() const;
	};

	class TransformManager final : public ComponentManager {
		AKE_DEFINE_COMPONENT_MANAGER(TransformManager, Transform)
		private:
			struct LocalTransformNode final {
				akm::Vec3 position;
				akm::Quat rotation;
				akm::Vec3 scale;
			};

			struct GlobalTransformNode final {
				LocalTransformNode transform;
				bool isDirty;
			};

			struct CacheNode final {
				mutable bool isGlobalDirty;
				mutable akm::Mat4 globalTransform;
				mutable bool isLocalDirty;
				mutable akm::Mat4 localTransform;
			};


			std::unordered_map<EntityID, LocalTransformNode> m_localTransform;
			mutable std::unordered_map<EntityID, GlobalTransformNode> m_globalTransform;

			//std::unordered_map<EntityID, CacheNode> m_cache;

			akev::SubscriberID m_entityGraphID;

			void registerHooks() override;
			void markDirty(EntityID id) const;

		protected:
			bool createComponent(EntityID entityID, const akm::Vec3& position = {0,0,0}, const akm::Quat& rotation = {1,0,0,0}, const akm::Vec3& scale = {1,1,1}, bool worldSpace = false);
			bool destroyComponent(EntityID entityID) override;

			akm::Mat4 calculateTransformMatrix(const LocalTransformNode& transform) const;
			LocalTransformNode getWorldTransform(EntityID entityID) const;

		public:

			// ///////// //
			// // Get // //
			// ///////// //

			akm::Mat4 localToWorld(EntityID entityID) const;
			akm::Mat4 worldToLocal(EntityID entityID) const;

			akm::Vec3 position(EntityID entityID) const;

			akm::Mat4 rotationMatrix(EntityID entityID) const;
			akm::Quat rotationQuat(  EntityID entityID) const;
			akm::Vec3 rotationEuler( EntityID entityID) const;

			akm::Vec3 scale(EntityID entityID) const;

			akm::Vec3 rightward(EntityID entityID) const;
			akm::Vec3 upward(   EntityID entityID) const;
			akm::Vec3 forward(  EntityID entityID) const;
			akm::Vec3 leftward( EntityID entityID) const;
			akm::Vec3 downward( EntityID entityID) const;
			akm::Vec3 backward( EntityID entityID) const;

			// ///////// //
			// // Set // //
			// ///////// //

			void setPosition(EntityID entityID, const akm::Vec3& p);
			void setRotation(EntityID entityID, const akm::Mat4& r);
			void setRotation(EntityID entityID, const akm::Quat& r);

			void setRotation(EntityID entityID, const akm::Vec3& r);
			void setRotation(EntityID entityID, const akm::Vec3& f, const akm::Vec3& u);

			void setScale(EntityID entityID, const akm::Vec3& s);

			// /////////// //
			// // Apply // //
			// /////////// //

			void move(EntityID entityID, const akm::Vec3& p);

			void moveRightward(EntityID entityID, fpSingle dist);
			void moveUpward(   EntityID entityID, fpSingle dist);
			void moveForward(  EntityID entityID, fpSingle dist);
			void moveLeftward( EntityID entityID, fpSingle dist);
			void moveDownward( EntityID entityID, fpSingle dist);
			void moveBackward( EntityID entityID, fpSingle dist);

			void rotate(EntityID entityID, fpSingle angle, const akm::Vec3& axis);
			void rotate(EntityID entityID, const akm::Mat4& r);
			void rotate(EntityID entityID, const akm::Quat& r);
			void rotate(EntityID entityID, const akm::Vec3& r);

			void rotatePre(EntityID entityID, fpSingle angle, const akm::Vec3& axis);
			void rotatePre(EntityID entityID, const akm::Mat4& r);
			void rotatePre(EntityID entityID, const akm::Quat& r);
			void rotatePre(EntityID entityID, const akm::Vec3& r);

			void scaleByFactor(EntityID entityID, const akm::Vec3& s);

			// ///////// //
			// // Get // //
			// ///////// //

			akm::Mat4 localToParent(EntityID entityID) const;

			akm::Mat4 parentToLocal(EntityID entityID) const;

			akm::Vec3 localPosition(EntityID entityID) const;

			akm::Mat4 localRotationMatrix(EntityID entityID) const;
			akm::Quat localRotationQuat(  EntityID entityID) const;
			akm::Vec3 localRotationEuler( EntityID entityID) const;

			akm::Vec3 localScale(EntityID entityID) const;

			akm::Vec3 localRightward(EntityID entityID) const;
			akm::Vec3 localUpward(   EntityID entityID) const;
			akm::Vec3 localForward(  EntityID entityID) const;
			akm::Vec3 localLeftward( EntityID entityID) const;
			akm::Vec3 localDownward( EntityID entityID) const;
			akm::Vec3 localBackward( EntityID entityID) const;

			// ///////// //
			// // Set // //
			// ///////// //

			void setLocalPosition(EntityID entityID, const akm::Vec3& p);

			void setLocalRotation(EntityID entityID, const akm::Mat4& r);
			void setLocalRotation(EntityID entityID, const akm::Quat& r);
			void setLocalRotation(EntityID entityID, const akm::Vec3& r);
			void setLocalRotation(EntityID entityID, const akm::Vec3& f, const akm::Vec3& u);

			void setLocalScale(EntityID entityID, const akm::Vec3& s);

			// /////////// //
			// // Apply // //
			// /////////// //

			void moveLocal(EntityID entityID, const akm::Vec3& p);

			void moveLocalRightward(EntityID entityID, fpSingle dist);
			void moveLocalUpward(   EntityID entityID, fpSingle dist);
			void moveLocalForward(  EntityID entityID, fpSingle dist);
			void moveLocalLeftward( EntityID entityID, fpSingle dist);
			void moveLocalDownward( EntityID entityID, fpSingle dist);
			void moveLocalBackward( EntityID entityID, fpSingle dist);

			void rotateLocal(EntityID entityID, fpSingle angle, const akm::Vec3& axis);
			void rotateLocal(EntityID entityID, const akm::Mat4& r);
			void rotateLocal(EntityID entityID, const akm::Quat& r);
			void rotateLocal(EntityID entityID, const akm::Vec3& r);

			void scaleLocalByFactor(EntityID entityID, const akm::Vec3& s);

			// //////////////// //
			// // Components // //
			// //////////////// //

			Transform component(EntityID entityID);
			const Transform component(EntityID entityID) const;

			bool hasComponent(EntityID entityID) const override;
	};
}



#endif
