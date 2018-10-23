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

#ifndef AK_ENGINE_COMPONENTS_POINTLIGHT_HPP_
#define AK_ENGINE_COMPONENTS_POINTLIGHT_HPP_

#include <ak/container/SlotMap.hpp>
#include <ak/engine/ComponentManager.hpp>
#include <ak/engine/Type.hpp>
#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <glm/detail/type_vec3.hpp>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ake {
	class PointLightManager;

	struct PointLight final {
		AKE_DEFINE_COMPONENT(PointLightManager, PointLight)
		fpSingle radius;
		akm::Vec3 colour;
		fpSingle intensity;
	};

	class PointLightManager final : public ake::ComponentManager {
		AKE_DEFINE_COMPONENT_MANAGER(PointLightManager, PointLight)
		private:
			std::unordered_map<EntityID, PointLight> m_pointLights;

		protected:
			bool createComponent(EntityID entityID, fpSingle radius = 1.f, akm::Vec3 colour = akm::Vec3(1,1,1), fpSingle intensity = 1.f) {
				return m_pointLights.emplace(entityID, PointLight{radius, colour, intensity}).second;
			}

			bool destroyComponent(EntityID entityID) override {
				return m_pointLights.erase(entityID) > 0;
			}

			void registerHooks() override {}

		public:
			PointLight& component(EntityID entityID) {
				return m_pointLights.at(entityID);
			}

			const PointLight& component(EntityID entityID) const {
				return m_pointLights.at(entityID);
			}

			bool hasComponent(EntityID entityID) const override {
				return m_pointLights.find(entityID) != m_pointLights.end();
			}

	};

}



#endif /* AK_ENGINE_COMPONENTS_POINTLIGHT_HPP_ */
