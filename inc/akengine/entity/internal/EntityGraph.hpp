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

#ifndef AK_ENGINE_INTERNAL_ENTITYGRAPH_HPP_
#define AK_ENGINE_INTERNAL_ENTITYGRAPH_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/SlotMap.hpp>
#include <akcommon/UnorderedVector.hpp>
#include <akengine/entity/Type.hpp>
#include <akmath/Types.hpp>
#include <string>
#include <unordered_set>

namespace ake {
	class EntityManager;

	class EntityGraph final {
		friend class EntityManager;
		private:
			// ///////////// //
			// // General // //
			// ///////////// //
			EntityManager& m_entityManager;

			// //////////////// //
			// // Transforms // //
			// //////////////// //
			struct TransformNode final {
				akm::Vec3 position;
				akm::Quat rotation;
				akm::Vec3 scale;
				mutable bool isDirty;
				mutable akm::Mat4x3 cache;
			};

			akc::SlotMap<TransformNode> m_transformsLocal;
			mutable akc::SlotMap<TransformNode> m_transformsGlobal;

			static akm::Mat4x3 calculateTransformMatrix(const TransformNode& transform);

			void markDirty(EntityID id) const;
			TransformNode getGlobalTransform(EntityID entityID) const;
			TransformNode getLocalTransform(EntityID entityID) const;

			// /////////// //
			// // Graph // //
			// /////////// //
			struct GraphNode final {
				EntityID parent;
				akc::UnorderedVector<EntityID> children;
			};

			std::unordered_set<EntityID> m_graphRoot;
			akc::SlotMap<GraphNode> m_graphStorage;

		protected:
			void registerEntity(EntityID entityID, EntityID parentID = EntityID(), const akm::Vec3& position = {0,0,0}, const akm::Quat& rotation = {1,0,0,0}, const akm::Vec3& scale = {1,1,1}, bool worldSpace = false);
			void unregisterEntity(EntityID entityID);

		public:
			EntityGraph(EntityManager& manager) : m_entityManager(manager) {}

			// /////////// //
			// // Graph // //
			// /////////// //
			bool setParent(EntityID entityID, EntityID newParent);
			EntityID parent(EntityID entityID) const;

			const akc::UnorderedVector<EntityID>& children(EntityID entityID) const;

			EntityID findFirstNamed(EntityID baseID, const std::string& name) const;
			akc::UnorderedVector<EntityID> findAllNamed(EntityID baseID, const std::string& name) const;

			// ////////////////////// //
			// // Global Transform // //
			// ////////////////////// //
			akm::Mat4x3 localToWorld(EntityID entityID) const;
			akm::Mat4x3 worldToLocal(EntityID entityID) const;

			akm::Vec3 position(EntityID entityID) const;
			akm::Quat rotation(EntityID entityID) const;
			akm::Vec3 scale(EntityID entityID) const;

			akm::Vec3 rightward(EntityID entityID) const;
			akm::Vec3 upward(   EntityID entityID) const;
			akm::Vec3 forward(  EntityID entityID) const;
			akm::Vec3 leftward( EntityID entityID) const;
			akm::Vec3 downward( EntityID entityID) const;
			akm::Vec3 backward( EntityID entityID) const;

			void setPosition(EntityID entityID, const akm::Vec3& p);
			void setRotation(EntityID entityID, const akm::Quat& r);
			void setRotation(EntityID entityID, const akm::Vec3& f, const akm::Vec3& u);
			void setScale(EntityID entityID, const akm::Vec3& s);

			void move(EntityID entityID, const akm::Vec3& p);
			void rotate(EntityID entityID, fpSingle angle, const akm::Vec3& axis);
			void rotate(EntityID entityID, const akm::Quat& r);
			void scale(EntityID entityID, const akm::Vec3& s);

			// ///////////////////// //
			// // Local Transform // //
			// ///////////////////// //
			akm::Mat4x3 localToParent(EntityID entityID) const;
			akm::Mat4x3 parentToLocal(EntityID entityID) const;

			akm::Vec3 localPosition(EntityID entityID) const;
			akm::Quat localRotation(EntityID entityID) const;
			akm::Vec3 localScale(EntityID entityID) const;

			akm::Vec3 localRightward(EntityID entityID) const;
			akm::Vec3 localUpward(   EntityID entityID) const;
			akm::Vec3 localForward(  EntityID entityID) const;
			akm::Vec3 localLeftward( EntityID entityID) const;
			akm::Vec3 localDownward( EntityID entityID) const;
			akm::Vec3 localBackward( EntityID entityID) const;

			void setLocalPosition(EntityID entityID, const akm::Vec3& p);
			void setLocalRotation(EntityID entityID, const akm::Quat& r);
			void setLocalRotation(EntityID entityID, const akm::Vec3& f, const akm::Vec3& u);
			void setLocalScale(EntityID entityID, const akm::Vec3& s);

			void localMove(EntityID entityID, const akm::Vec3& p);
			void localRotate(EntityID entityID, fpSingle angle, const akm::Vec3& axis);
			void localRotate(EntityID entityID, const akm::Quat& r);
			void localScale(EntityID entityID, const akm::Vec3& s);
	};
}



#endif
