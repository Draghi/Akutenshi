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

#include <ak/container/SlotMap.hpp>
#include <ak/container/UnorderedVector.hpp>
#include <ak/engine/ECS.hpp>
#include <ak/String.hpp>
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

using namespace ake;

// //////////// //
// // Entity // //
// //////////// //

struct Entity final {
	std::string name;
	std::unordered_multimap<ComponentID, InstanceID> components;
	bool persistant;
	Entity(const std::string& nameVal) : name(nameVal), persistant(false) {}
};

// ///////// //
// // Fwd // //
// ///////// //

namespace ake {
	namespace internal {
		struct ComponentManagerAccessor {
			static void registerComponentManager(ComponentID id, const std::function<ComponentManagerFactory_f>& factoryFunc);
		};
	}
}

static akev::Dispatcher<RegisterComponentManagersEvent>& registerComponentManagersEventDispatcher();

// ////////// //
// // Data // //
// ////////// //
static std::unordered_map<ComponentID, std::function<ComponentManagerFactory_f>>& registry() {
	static std::unordered_map<ComponentID, std::function<ComponentManagerFactory_f>> instance;
	return instance;
}

// //////////////////// //
// // Implementation // //
// //////////////////// //

static bool& hasInit() { static bool hasInit = false; return hasInit; }
static void assertHasInit() { if (!hasInit()) throw std::logic_error("Entity Component System is not initialized."); }

bool ake::initEntityComponentSystem() {
	if (std::exchange(hasInit(), true)) return false;
	RegisterComponentManagersEvent event(ake::internal::ComponentManagerAccessor::registerComponentManager);
	registerComponentManagersEventDispatcher().send(event);
	return true;
}

EntityManager ake::createEntityManager() {
	assertHasInit();
	std::unordered_map<ComponentID, std::unique_ptr<ComponentManager>> componentRegistry;
	for(auto iter = registry().begin(); iter != registry().end(); iter++) componentRegistry.insert(std::make_pair(iter->first, iter->second()));
	return EntityManager(std::move(componentRegistry));
}

// ////////////////////////////// //
// // Controlled Functionality // //
// ////////////////////////////// //

inline void ake::internal::ComponentManagerAccessor::registerComponentManager(ComponentID id, const std::function<ComponentManagerFactory_f>& factoryFunc) {
	auto result = registry().insert(std::make_pair(id, factoryFunc));
	if (!result.second) throw std::logic_error(ak::buildString("EntityManager: ID conflict occurred for '", id, "'"));
}

// /////////// //
// // Event // //
// /////////// //

static akev::Dispatcher<RegisterComponentManagersEvent>& registerComponentManagersEventDispatcher() {
	static akev::Dispatcher<RegisterComponentManagersEvent> instance;
	return instance;
}

const akev::DispatcherProxy<RegisterComponentManagersEvent>& ake::registerComponentManagersEventDispatch() {
	static akev::DispatcherProxy<RegisterComponentManagersEvent> instance(registerComponentManagersEventDispatcher());
	return instance;
}

