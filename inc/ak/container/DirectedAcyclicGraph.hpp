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

#ifndef AK_CONTAINER_DIRECTEDACYCLICGRAPH_HPP_
#define AK_CONTAINER_DIRECTEDACYCLICGRAPH_HPP_

#include <ak/container/SlotMap.hpp>
#include <ak/container/UnorderedVector.hpp>
#include <ak/Iter.hpp>
#include <unordered_set>

namespace akc {

	using DAGraphID = akc::SlotID;
	template<typename type_t> class DirectedAcyclicGraph {
		private:

			struct Node {
				type_t value;
				std::unordered_set<DAGraphID> dependancies;
				std::unordered_set<DAGraphID> dependants;
			};

			akc::SlotMap<Node> m_nodes;
			std::unordered_set<DAGraphID> m_sources;
			
		public:
			
			DAGraphID insert(const type_t& val) {
				auto id = m_nodes.insert({val, {}, {}});
				m_sources.insert(id);
				return id;
			}

			bool addDependancy(DAGraphID id, DAGraphID depID) {
				auto& cNode = m_nodes[id];
				auto& dNode = m_nodes[depID];
				if (cNode.dependancies.empty()) m_sources.erase(id);
				if (!cNode.dependancies.insert(depID).first) return false;

				std::
				std::deque<DAGraphID> nodesToProcess{{depID}};
				while(!nodesToProcess.empty()) {
					auto id = nodesToProcess.front(); nodesToProcess.pop_front();
					auto& pNode = m_nodes[id];
					if (pNode.dependancies.find(id) != pNode.dependancies.end()) return false;

					for(auto dID : pNode.dependancies) {

					}
					nodesToProcess.push_front(pNode.dependancies.begin(), pNode.dependancies.end());
				}

				dNode.dependants.insert(depID);
			}


	};

}

#endif
