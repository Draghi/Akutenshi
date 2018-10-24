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

#ifndef AK_ENGINE_DATA_RAND_HPP_
#define AK_ENGINE_DATA_RAND_HPP_

#include <akcommon/PrimitiveTypes.hpp>
#include <limits>

namespace akd {

	template<typename int_t, typename dec_t> class RandomEngine {
		protected:
			RandomEngine() = default;
		public:
			using int_type = int_t;
			using dec_type = dec_t;

			virtual ~RandomEngine() = default;

			virtual int_type nextInt() = 0;

			virtual int_type nextInt(int_type min, int_type max) {
				return min + (this->nextInt() - min) % (max - min);
			}

			virtual dec_type nextDec() {
				auto val = static_cast<dec_type>(nextInt());
				return (1+(val < 0  ? (-val/std::numeric_limits<int_type>::min()) : (val/std::numeric_limits<int_type>::max())))/2;
			}

			virtual dec_type nextDec(dec_type min, dec_type max) {
				return min + nextDec()*(max-min);
			}

			virtual void discard(uint64 count) {
				while(count--) this->nextInt();
			}
	};

	class CMW4096Engine32 final : public RandomEngine<uint32, fpSingle> {
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

			void seed(uint32 seed) {
				for (auto i = 0u; i < cmwc_cycle; i++) state[i] = xorshift32Seeder(seed);
				do carry = xorshift32Seeder(seed); while (carry >= cmwc_c_max);
				index = cmwc_cycle - 1;
			}

		public:
			static constexpr uint32 default_seed = 0xA77208E6u; // Decided by xor-ing 1024-bytes of random quantum together in 32-bit chunks (aka super-quantum-true-random)

			CMW4096Engine32(uint32 sVal = default_seed) { seed(sVal); }

			int_type nextInt() override {
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
	};

	class CMW4096Engine32d final : public RandomEngine<uint64, fpDouble> {
		private:
			CMW4096Engine32 m_internal;

		public:
			static constexpr uint32 default_seed = CMW4096Engine32::default_seed;

			CMW4096Engine32d(uint32 sVal = default_seed) : m_internal(sVal) {}

			int_type nextInt() override {
				return static_cast<int_type>(m_internal.nextInt()) << 32 | m_internal.nextInt();
			}
	};
}

#endif
