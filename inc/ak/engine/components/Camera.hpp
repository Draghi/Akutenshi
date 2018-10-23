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

#ifndef AK_ENGINE_COMPONENTS_CAMERA_HPP_
#define AK_ENGINE_COMPONENTS_CAMERA_HPP_

#include <ak/container/SlotMap.hpp>
#include <ak/engine/ComponentManager.hpp>
#include <ak/engine/Entity.hpp>
#include <ak/engine/EntityManager.hpp>
#include <ak/engine/Type.hpp>
#include <ak/event/Dispatcher.hpp>
#include <ak/math/Matrix.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec2.hpp>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace ake {

	class CameraManager;

	class Camera final {
		AKE_DEFINE_COMPONENT(CameraManager, Camera)
		private:
			EntityID m_id;
			const EntityManager& m_eManager;

			enum class Projection : uint8 {
				Orthographic,
				PerspectiveH,
				PerspectiveV,
			};

			union ProjectionData {
				struct { fpSingle fov;  akm::Vec2 size; akm::Vec2 zRange; } persp;
				struct { akm::Vec2 xRange, yRange, zRange; } ortho;

				ProjectionData() : ortho{{-1, 1}, {-1, 1}, {-1, 1}} {}
				ProjectionData(fpSingle hFOV, akm::Vec2 viewSize, akm::Vec2 zRange) : persp{hFOV, viewSize, zRange} {}
				ProjectionData(akm::Vec2 xRange, akm::Vec2 yRange, akm::Vec2 zRange) : ortho{xRange, yRange, zRange} {}
			} m_projection;
			Projection m_projectionType;

			mutable struct {
				bool isProjectionDirty;
				akm::Mat4 projectionMatrix;

				bool isViewDirty;
				akm::Mat4 viewMatrix;
			} m_cache;

			akm::Vec2 m_viewOffset;
			akm::Vec2 m_viewSize;

			akev::SubscriberID m_transformChangeSubscription;

		public:
			Camera(EntityID id, const EntityManager& eManager) : m_id(id), m_eManager(eManager), m_projection(), m_projectionType(Projection::Orthographic), m_cache{true, akm::Mat4(1), true, akm::Mat4(1)}, m_viewOffset(0,0), m_viewSize(1,1), m_transformChangeSubscription() {
				//@todo Add change tracking transform
				//m_transformChangeSubscription = m_eManager.component<Transform>(m_id).
			}

			// //////////////// //
			// // Projection // //
			// //////////////// //
			void setOrthographic(const akm::Vec2& xRange, const akm::Vec2& yRange, const akm::Vec2& zRange) {
				m_projectionType = Projection::Orthographic;
				m_projection.ortho = {xRange, yRange, zRange};
				m_cache.isProjectionDirty = true;
			}

			void setPerspectiveH(float hFov, const akm::Vec2& size, const akm::Vec2& zRange) {
				m_projectionType = Projection::PerspectiveH;
				m_projection.persp = {hFov, size, zRange};
				m_cache.isProjectionDirty = true;
			}

			void setPerspectiveV(float vFov, const akm::Vec2& size, const akm::Vec2& zRange) {
				m_projectionType = Projection::PerspectiveV;
				m_projection.persp = {vFov, size, zRange};
				m_cache.isProjectionDirty = true;
			}

			fpSingle fovVert() const {
				switch(m_projectionType) {
					case Projection::PerspectiveH: return 2 * akm::atan((0.5 * m_projection.persp.size.y) / (0.5 * m_projection.persp.size.y / tan(m_projection.persp.fov/2)));
					case Projection::PerspectiveV: return m_projection.persp.fov;
					case Projection::Orthographic: return 0;
				}
			}

			fpSingle fovHorz() const {
				switch(m_projectionType) {
					case Projection::PerspectiveH: return m_projection.persp.fov;
					case Projection::PerspectiveV: return 2 * akm::atan((0.5 * m_projection.persp.size.x) / (0.5 * m_projection.persp.size.y / tan(m_projection.persp.fov/2)));
					case Projection::Orthographic: return 0;
				}
			}

			fpSingle planeNear() const {
				switch(m_projectionType) {
					case Projection::PerspectiveH: return m_projection.persp.zRange[0];
					case Projection::PerspectiveV: return m_projection.persp.zRange[0];
					case Projection::Orthographic: return m_projection.ortho.zRange[0];
				}
			}

			fpSingle planeFar() const {
				switch(m_projectionType) {
					case Projection::PerspectiveH: return m_projection.persp.zRange[1];
					case Projection::PerspectiveV: return m_projection.persp.zRange[1];
					case Projection::Orthographic: return m_projection.ortho.zRange[1];
				}
			}

			akm::Mat4 projectionMatrix() const {
				if (std::exchange(m_cache.isProjectionDirty, false)) {
					switch(m_projectionType) {
						case Projection::PerspectiveH: {
							m_cache.projectionMatrix = akm::perspectiveH(
								m_projection.persp.fov,
								m_projection.persp.size.x, m_projection.persp.size.y,
								m_projection.persp.zRange[0], m_projection.persp.zRange[1]
							);
						} break;
						case Projection::PerspectiveV: {
							m_cache.projectionMatrix = akm::perspectiveV(
								m_projection.persp.fov,
								m_projection.persp.size.x, m_projection.persp.size.y,
								m_projection.persp.zRange[0], m_projection.persp.zRange[1]
							);
						} break;
						case Projection::Orthographic: {
							m_cache.projectionMatrix = akm::orthographic(
								m_projection.ortho.xRange[0], m_projection.ortho.xRange[1],
								m_projection.ortho.yRange[0], m_projection.ortho.yRange[1],
								m_projection.ortho.zRange[0], m_projection.ortho.zRange[1]
							);
						} break;
					};
				}
				return m_cache.projectionMatrix;
			}

			// ////////// //
			// // View // //
			// ////////// //

			akm::Mat4 viewMatrix() const {
				return m_eManager.getEntity(m_id).worldToLocal();
			}

			// ////////////// //
			// // ViewPort // //
			// ////////////// //
			void viewOffset(const akm::Vec2& val) { m_viewOffset = val; }
			void viewSize(const akm::Vec2& val) { m_viewSize = val; }

			akm::Vec2 viewOffset() const { return m_viewOffset; }
			akm::Vec2 viewSize() const { return m_viewSize; }
	};


	class CameraManager final : public ComponentManager {
		AKE_DEFINE_COMPONENT_MANAGER(CameraManager, Camera)
		private:
			std::unordered_map<EntityID, Camera> m_components;

		protected:
			bool createComponent(EntityID entityID) {
				return m_components.emplace(entityID, Camera(entityID, entityManager())).second;
			}

			bool destroyComponent(EntityID entityID) override {
				auto iter = m_components.find(entityID);
				if (iter == m_components.end()) throw std::logic_error("ake::Camera: Data corruption, tried to delete non-existent instance.");
				m_components.erase(entityID);
				return true;
			}

		public:


			Camera& component(EntityID entityID) { return m_components.at(entityID); }
			const Camera& component(EntityID entityID) const { return m_components.at(entityID); }

			const std::unordered_map<EntityID, Camera>& components() const { return m_components; }

			bool hasComponent(EntityID entityID) const override { return m_components.find(entityID) != m_components.end(); }
	};

}

#endif
