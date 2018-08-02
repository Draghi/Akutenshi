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

#ifndef AK_ENGINE_INTERNAL_NAMESTORAGE_HPP_
#define AK_ENGINE_INTERNAL_NAMESTORAGE_HPP_

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include <ak/container/SlotMap.hpp>
#include <ak/container/UnorderedVector.hpp>
#include <ak/engine/Type.hpp>
#include <ak/util/Iterator.hpp>

namespace ake {
	namespace internal {
		class NameStorage final {
			private:
				akc::SlotMap<std::string> m_nameStorage;
				akc::SlotMap<akc::SlotID> m_entityNameID;
				std::unordered_map<std::string, std::pair<akc::SlotID, akc::UnorderedVector<EntityID>>> m_lookupEntitiesByName;

				akc::SlotID addName(const std::string& name) {
					auto iter = m_lookupEntitiesByName.find(name);
					if (iter != m_lookupEntitiesByName.end()) return iter->second.first;
					auto iter2 = m_lookupEntitiesByName.emplace(name, std::make_pair(m_nameStorage.insert(name).first, akc::UnorderedVector<EntityID>{})).first;
					return iter2->second.first;
				}

			public:
				void registerEntity(EntityID entityID, akc::SlotID nameID) {
					m_lookupEntitiesByName.at(m_nameStorage.at(nameID)).second.insert(nameID);
					if (!m_entityNameID.insert(nameID).first == entityID) throw std::logic_error("Name storage EntityID mismatch.");
				}

				void registerEntity(EntityID entityID, const std::string& name) {
					registerEntity(entityID, addName(name));
				}

				void deregisterEntity(EntityID entityID) {
					auto nameID = m_entityNameID.at(entityID);
					auto& name = m_nameStorage.at(nameID);
					auto& nameEntry = m_lookupEntitiesByName.at(name);
					nameEntry.second.erase(aku::find(nameEntry.second, entityID));
					if (nameEntry.second.empty()) {
						m_lookupEntitiesByName.erase(name);
						m_nameStorage.erase(nameID);
					}
					m_entityNameID.erase(entityID);
				}

				akc::SlotID getNameID(const std::string& name) const { return m_lookupEntitiesByName.at(name).first; }
				akc::SlotID getNameID(EntityID entityID) const { return m_entityNameID.at(entityID); }

				const std::string& getNameByID(akc::SlotID nameID) const { return m_nameStorage.at(nameID); }
				const std::string& getNameByEntity(EntityID entityID) const { return getNameByID(getNameID(entityID)); }

		};
	}
}

#endif
