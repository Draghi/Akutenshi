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

#ifndef AK_ITERATOR_HPP_
#define AK_ITERATOR_HPP_

namespace ak {

	template<typename type_t> type_t prevIter(type_t& val) {
		type_t ret = val; ret--;
		return ret;
	}

	template<typename type_t> type_t nextIter(type_t& val) {
		type_t ret = val; ret++;
		return ret;
	}

	template<typename container_t> typename container_t::iterator find(container_t& container, const typename container_t::value_type& val) {
		return std::find(container.begin(), container.end(), val);
	}

	template<typename container_t> typename container_t::const_iterator find(const container_t& container, const typename container_t::value_type& val) {
		return std::find(container.begin(), container.end(), val);
	}

}

#endif