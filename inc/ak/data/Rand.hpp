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

#ifndef AK_DATA_RAND_HPP_
#define AK_DATA_RAND_HPP_

#include <ak/PrimitiveTypes.hpp>

namespace akd {
	class CMW4096Engine32 final {
		private:
			static constexpr uint32 cmwc_cycle = 4096u;
			static constexpr uint32 cmwc_c_max = 809430660u;
			uint32 state[cmwc_cycle];
			uint32 carry;
			uint index;

			static uint32 xorshift32Seeder(uint32& seed) {
				seed ^= seed << 13;
				seed ^= seed >> 17;
				seed ^= seed <<  5;
				return seed;
			}

		public:
			using result_type = uint32;
			static constexpr result_type default_seed = 0xA77208E6u; // Decided by xor-ing 1024-bytes of random quantum together in 32-bit chunks (aka super-quantum-true-random)

			CMW4096Engine32(result_type sVal = default_seed) { seed(sVal); }

			result_type min() { return std::numeric_limits<result_type>::min(); }
			result_type max() { return std::numeric_limits<result_type>::max(); }

			void seed(result_type seed) {
				for (auto i = 0u; i < cmwc_cycle; i++) state[i] = xorshift32Seeder(seed);
				do carry = xorshift32Seeder(seed); while (carry >= cmwc_c_max);
				index = cmwc_cycle - 1;
			}

			result_type operator()() {
				constexpr uint64 a = 18782;
				constexpr uint32 m = 0xfffffffe;
				uint64 t; uint32 x;

				// Advance
				index = (index + 1) & (cmwc_cycle - 1);

				// Modify State
				t = a * state[index] + carry;
				carry = t >> 32;
				x = t + carry;
				if (x < carry) { x++; carry++; }
				state[index] = m - x;

				return state[index];
			}

			void discard(unsigned long long z) { while(z--) (*this)(); }
	};

	class CMW4096Engine32d final {
		private:
			CMW4096Engine32 m_internal;

		public:
			using result_type = uint64;
			static constexpr result_type default_seed = CMW4096Engine32::default_seed;

			CMW4096Engine32d(result_type sVal = default_seed) : m_internal(static_cast<uint32>((sVal >> 32) ^ sVal)) {}

			result_type min() { return std::numeric_limits<result_type>::min(); }
			result_type max() { return std::numeric_limits<result_type>::max(); }

			void seed(result_type seed) { m_internal.seed(static_cast<uint32>((seed >> 32) ^ seed)); }
			result_type operator()() { return static_cast<result_type>(m_internal()) << 32 | m_internal(); }
			void discard(unsigned long long z) { while(z--) (*this)(); }
	};
}

#endif
