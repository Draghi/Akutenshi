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

#ifndef AK_ENGINE_COMPONENTS_TRANSFORM3D_HPP_
#define AK_ENGINE_COMPONENTS_TRANSFORM3D_HPP_

#include <ak/container/SlotMap.hpp>
#include <ak/container/UnorderedVector.hpp>
#include <ak/engine/ECS.hpp>
#include <ak/math/Matrix.hpp>
#include <ak/math/Quaternion.hpp>
#include <ak/math/Types.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <unordered_map>
#include <utility>

namespace akec {
	class Transform3D final : public ake::ComponentManager {
		AKE_DEFINE_COMPONENT_MANAGER(Transform3D)
		private:
			struct GraphData {
				ake::ComponentID parent;
				akc::UnorderedVector<ake::ComponentID> children;

				mutable akm::Mat4 localTransform;
				mutable bool isLocalDirty;

				mutable akm::Mat4 globalTransform;
				mutable bool isGlobalDirty;
			};

			struct SpatialData {
				akm::Vec3 position;
				akm::Quat rotation;
				akm::Vec3 scale;
			};

			akc::SlotMap<GraphData> m_graphData;
			akc::SlotMap<SpatialData> m_spatialData;
			std::unordered_map<ake::ComponentID, ake::EntityID> m_entityLookup;

		public:
			class Data final {
				private:
					Transform3D& m_owner;
					ake::ComponentID m_id;

				public:
					Data(Transform3D& owner, ake::ComponentID id) : m_owner(owner), m_id(id) {}

					Data& setLocalPosition(const akm::Vec3& v) { m_owner.setLocalPosition(m_id, v); return *this; }
					Data& setLocalRotation(const akm::Quat& v) { m_owner.setLocalRotation(m_id, v); return *this; }
					Data& setLocalScale(   const akm::Vec3& v) { m_owner.setLocalScale(m_id, v);    return *this; }

					akm::Vec3 localPosition()  const { return m_owner.localPosition(m_id); }
					akm::Quat localRotation()  const { return m_owner.localRotation(m_id); }
					akm::Vec3 localScale()     const { return m_owner.localScale(m_id); }
					akm::Mat4 localTransform() const { return m_owner.localTransform(m_id); }

					akm::Vec3 worldPosition()  const { return m_owner.worldPosition(m_id); }
					akm::Quat worldRotation()  const { return m_owner.worldRotation(m_id); }
					akm::Vec3 worldScale()     const { return m_owner.worldScale(m_id); }
					akm::Mat4 worldTransform() const { return m_owner.worldTransform(m_id); }
			};

		protected:
			ake::ComponentID newComponent(ake::EntityManager& /*manager*/, ake::EntityID entityID, const akm::Vec3& position = {0,0,0}, const akm::Quat& rotation = {0,0,0,0}, const akm::Vec3& scale = {1,1,1}, const ake::ComponentID& parentID = ake::ComponentID()) {
				auto result = m_spatialData.insert({position, rotation, scale}).first;
				if (result != m_graphData.insert({parentID, {}, akm::Mat4(1), true, akm::Mat4(1), true}).first) throw std::logic_error("akec::Transform3D::newComponent: ComponentID mismatch");
				if (parentID.isValid()) m_graphData[parentID].children.insert(result);
				m_entityLookup.emplace(result, entityID);
				return result;
			}

			bool deleteComponent(ake::EntityManager& /*manager*/, ake::EntityID /*entityID*/, ake::ComponentID componentID) override {
				if (m_graphData[componentID].parent.isValid()) {
					auto& parentsChildren = m_graphData[m_graphData[componentID].parent].children;
					parentsChildren.erase(ak::find(parentsChildren, componentID));
				}

				if (!m_graphData[componentID].children.empty()) throw std::logic_error("Deleting parent with children not implemented yet.");

				m_graphData.erase(componentID);
				m_spatialData.erase(componentID);
				m_entityLookup.erase(componentID);
				return true;
			}

			void invalidateChildTransforms(ake::ComponentID id) {
				std::deque<ake::ComponentID> toProcess{{id}};
				while(!toProcess.empty()) {
					auto& cNode = m_graphData[toProcess.front()];
					cNode.isGlobalDirty = true;
					toProcess.insert(toProcess.end(), cNode.children.begin(), cNode.children.end());
					toProcess.pop_front();
				}
			}

		public:
			akm::Vec3 worldPosition(ake::ComponentID id) const {
				return akm::Vec3(akm::column(worldTransform(id), 3));
			}

			akm::Quat worldRotation(ake::ComponentID id) const {
				return akm::quat_cast(akm::scale(1.f/worldScale(id)) * worldTransform(id));
			}

			akm::Vec3 worldScale(ake::ComponentID id) const {
				return akm::Vec3(
					akm::magnitude(akm::column(worldTransform(id), 0)),
					akm::magnitude(akm::column(worldTransform(id), 1)),
					akm::magnitude(akm::column(worldTransform(id), 2))
				);
			}

			akm::Mat4 worldTransform(ake::ComponentID id) const {
				if (!id.isValid()) return akm::Mat4(1);
				auto& graphData = m_graphData[id];
				if (std::exchange(graphData.isGlobalDirty, false)) graphData.globalTransform = worldTransform(graphData.parent) * localTransform(id);
				return graphData.globalTransform;
			}



			akm::Vec3 localPosition( ake::ComponentID id) const {
				if (!id.isValid()) return akm::Vec3(0,0,0);
				return m_spatialData[id].position;
			}

			akm::Quat localRotation( ake::ComponentID id) const {
				if (!id.isValid()) return akm::Quat(0,0,0,0);
				return m_spatialData[id].rotation;
			}

			akm::Vec3 localScale(    ake::ComponentID id) const {
				if (!id.isValid()) return akm::Vec3(1,1,1);
				return m_spatialData[id].scale;
			}

			akm::Mat4 localTransform(ake::ComponentID id) const {
				if (!id.isValid()) return akm::Mat4(1);
				auto& graphData = m_graphData[id];
				if (std::exchange(graphData.isLocalDirty, false)) {
					auto& spatialData = m_spatialData[id];
					graphData.localTransform = akm::translate(spatialData.position) * akm::mat4_cast(spatialData.rotation) * akm::scale(spatialData.scale);
				}
				return graphData.localTransform;
			}

			void setLocalPosition(ake::ComponentID id, const akm::Vec3& pos) {
				m_spatialData[id].position = pos;
				m_graphData[id].isLocalDirty = true;
				invalidateChildTransforms(id);
			}

			void setLocalRotation(ake::ComponentID id, const akm::Quat& rot) {
				m_spatialData[id].rotation = rot;
				m_graphData[id].isLocalDirty = true;
				invalidateChildTransforms(id);
			}

			void setLocalScale(ake::ComponentID id, const akm::Vec3& scl) {
				m_spatialData[id].scale = scl;
				m_graphData[id].isLocalDirty = true;
				invalidateChildTransforms(id);
			}

			Data getInstance(ake::ComponentID id) { return Data(*this, id); }
			const Data getInstance(ake::ComponentID id) const { return Data(*const_cast<Transform3D*>(this), id); }
	};
}

#endif
