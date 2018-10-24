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

#ifndef AK_COMMON_SCOPEGUARD_HPP_
#define AK_COMMON_SCOPEGUARD_HPP_

#include <algorithm>
#include <functional>
#include <utility>

namespace ak {
	/**
	 * A ScopeGuard that executes an arbitrary function.
	 * Supports releasing and move semantics.
	 */
	class ScopeGuard final {
		private:
			using func_t = std::function<void()>;

			ScopeGuard(const ScopeGuard&) = delete;
			ScopeGuard& operator=(const ScopeGuard&) = delete;

			func_t m_func;

		public:
			/**
			 * Creates a new, empty, ScopeGuard
			 */
			ScopeGuard() : m_func(nullptr) { }

			/**
			 * Creates a new, functioning, ScopeGuard.
			 * @tparam func_t Callable type that will be wrapped by the ScopeGuard
			 * @param func The function to call when the scope is exited.
			 */
			template<typename func_t> ScopeGuard(const func_t& func) : m_func(func) { }

			/**
			 * Move-constructs a copy of the given ScopeGuard
			 * @param other The scope guard to move
			 */
			ScopeGuard(ScopeGuard&& other) : m_func(std::move(other.m_func)) { other.clear(); }

			/**
			 * Destructor, calls stored function
			 */
			~ScopeGuard() { execute(); }

			/**
			 * Move-constructs a copy of the given ScopeGuard
			 * @param other The scope guard to move
			 */
			ScopeGuard& operator=(ScopeGuard&& other) {
				m_func = std::move(other.m_func);
				other.clear();
				return *this;
			}

			/**
			 * Releases the stored function.
			 * @return The stored function
			 */
			func_t release() { return exchange(m_func, func_t()); }

			/**
			 * Releases the stored function.
			 */
			void clear() { m_func = func_t(); }

			/**
			 * Executes the stored function and then releases it
			 */
			void execute() {
				if (m_func) {
					m_func();
					clear();
				}
			}

			/**
			 * Returns if the ScopeGuard is empty or has been released
			 * @return If the ScopeGuard is empty or has been released
			 */
			bool empty() const {
				return !m_func;
			}

			/**
			 * Returns if the ScopeGuard is not empty and has not been released
			 * @return id the ScopeGuard is not empty and has not been released
			 */
			operator bool() { return !empty(); }
	};
}



#endif
