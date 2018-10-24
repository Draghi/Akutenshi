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

#ifndef AK_ENGINE_ENTITY_HPP_
#define AK_ENGINE_ENTITY_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <akcommon/SlotMap.hpp>
#include <akcommon/UnorderedVector.hpp>
#include <akengine/entity/EntityManager.hpp>
#include <akengine/entity/internal/EntityGraph.hpp>
#include <akengine/entity/Type.hpp>
#include <akmath/Types.hpp>
#include <string>

namespace ake {

	class Entity final {
		private:
			EntityManager* m_manager;
			EntityID m_id;

		public:
			Entity(EntityManager& manager, EntityID id) : m_manager(&manager), m_id(id) {}

			// //////////////// //
			// // Components // //
			// //////////////// //

			template<typename component_t, typename... vargs_t> bool createComponent(const vargs_t&... vargs) { return m_manager->createComponent<component_t, vargs_t...>(m_id, std::forward<vargs_t>(vargs)...); }
			template<typename component_t> bool destroyComponent() { return m_manager->destroyComponent<component_t>(m_id); }

			bool hasComponent(ComponentID componentID) const { return m_manager->hasComponent(m_id, componentID); }
			template<typename component_t> bool hasComponent() const { return m_manager->hasComponent<component_t>(m_id); }

			template<typename component_t> decltype(std::declval<typename component_t::MANAGER>().component(EntityID())) component() { return m_manager->component<component_t>(m_id); }
			template<typename component_t> decltype(std::declval<const typename component_t::MANAGER>().component(EntityID())) component() const { return m_manager->component<component_t>(m_id); }

			// /////////// //
			// // Graph // //
			// /////////// //

			void setParent(EntityID parent) { m_manager->entityGraph().setParent(m_id, parent); }
			Entity parent() { return Entity(*m_manager, m_id); }
			const Entity parent() const { return Entity(const_cast<EntityManager&>(*m_manager), m_id); }
			EntityID findFirstNamed(EntityID baseID, const std::string& name) const;
			akc::UnorderedVector<EntityID> findAllNamed(EntityID baseID, const std::string& name) const;

			// ////////////////////// //
			// // Global Transform // //
			// ////////////////////// //

			akm::Mat4x3 localToWorld() const { return m_manager->entityGraph().localToWorld(m_id); }
			akm::Mat4x3 worldToLocal() const { return m_manager->entityGraph().worldToLocal(m_id); }

			akm::Vec3 position() const  { return m_manager->entityGraph().position(m_id); }
			akm::Quat rotation() const  { return m_manager->entityGraph().rotation(m_id); }
			akm::Vec3 scale()    const  { return m_manager->entityGraph().scale(m_id); }

			akm::Vec3 rightward() const { return m_manager->entityGraph().rightward(m_id); }
			akm::Vec3  leftward() const { return m_manager->entityGraph(). leftward(m_id); }
			akm::Vec3    upward() const { return m_manager->entityGraph().   upward(m_id); }
			akm::Vec3  downward() const { return m_manager->entityGraph(). downward(m_id); }
			akm::Vec3   forward() const { return m_manager->entityGraph().  forward(m_id); }
			akm::Vec3  backward() const { return m_manager->entityGraph(). backward(m_id); }

			void setPosition(const akm::Vec3& position) { return m_manager->entityGraph().setPosition(m_id, position); }
			void setRotation(const akm::Quat& rotation) { return m_manager->entityGraph().setRotation(m_id, rotation); }
			void setRotation(const akm::Vec3& forward, const akm::Vec3& up) { return m_manager->entityGraph().setRotation(m_id, forward, up); }
			void setScale(const akm::Vec3& scale) { return m_manager->entityGraph().setScale(m_id, scale); }

			void move(const akm::Vec3& offset) { return m_manager->entityGraph().move(m_id, offset); }
			void rotate(fpSingle angle, const akm::Vec3& axis) { return m_manager->entityGraph().rotate(m_id, angle, axis); }
			void rotate(const akm::Quat& rotation) { return m_manager->entityGraph().rotate(m_id, rotation); }
			void scale(const akm::Vec3& scale) { return m_manager->entityGraph().scale(m_id, scale); }

			// ///////////////////// //
			// // Local Transform // //
			// ///////////////////// //

			akm::Mat4x3 localToParent() const { return m_manager->entityGraph().localToParent(m_id); }
			akm::Mat4x3 parentToLocal() const { return m_manager->entityGraph().parentToLocal(m_id); }

			akm::Vec3 localPosition() const  { return m_manager->entityGraph().position(m_id); }
			akm::Quat localRotation() const  { return m_manager->entityGraph().rotation(m_id); }
			akm::Vec3 localScale()    const  { return m_manager->entityGraph().scale(m_id); }

			akm::Vec3 localRightward() const { return m_manager->entityGraph().localRightward(m_id); }
			akm::Vec3  localLeftward() const { return m_manager->entityGraph(). localLeftward(m_id); }
			akm::Vec3    localUpward() const { return m_manager->entityGraph().   localUpward(m_id); }
			akm::Vec3  localDownward() const { return m_manager->entityGraph(). localDownward(m_id); }
			akm::Vec3   localForward() const { return m_manager->entityGraph().  localForward(m_id); }
			akm::Vec3  localBackward() const { return m_manager->entityGraph(). localBackward(m_id); }

			void setLocalPosition(const akm::Vec3& position) { return m_manager->entityGraph().setLocalPosition(m_id, position); }
			void setLocalRotation(const akm::Quat& rotation) { return m_manager->entityGraph().setLocalRotation(m_id, rotation); }
			void setLocalRotation(const akm::Vec3& forward, const akm::Vec3& up) { return m_manager->entityGraph().setLocalRotation(m_id, forward, up); }
			void setLocalScale(const akm::Vec3& scale) { return m_manager->entityGraph().setLocalScale(m_id, scale); }

			void localMove(const akm::Vec3& offset) { return m_manager->entityGraph().localMove(m_id, offset); }
			void localRotate(fpSingle angle, const akm::Vec3& axis) { return m_manager->entityGraph().localRotate(m_id, angle, axis); }
			void localRotate(const akm::Quat& rotation) { return m_manager->entityGraph().localRotate(m_id, rotation); }
			void localScale(const akm::Vec3& scale) { return m_manager->entityGraph().localScale(m_id, scale); }

			// ///////////// //
			// // General // //
			// ///////////// //

			EntityID id() const { return m_id; }
			EntityUID uid() const { return m_manager->entityUID(m_id); }
			const std::string name() const { return m_manager->entityName(m_id); }

			EntityManager& manager() { return *m_manager; }
			const EntityManager& manager() const { return *m_manager; }
	};
}



#endif /* AK_ENGINE_ENTITY_HPP_ */
