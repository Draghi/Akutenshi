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

#ifndef AKSOUND_BACKEND_SAMPLER_HPP_
#define AKSOUND_BACKEND_SAMPLER_HPP_

#include <AkCommon/PrimitiveTypes.hpp>

namespace aks {
	class Sampler {
		protected:
			Sampler() = default;
			Sampler(const Sampler&) = default;
			Sampler& operator=(const Sampler&) = default;

		public:
			virtual ~Sampler() = default;

			virtual akSize sample(fpSingle* out, akSSize start, akSize count) const = 0;

			virtual akSize sampleCount() const = 0;
			virtual bool loops() const = 0;
	};
}



#endif /* AKSOUND_BACKEND_SAMPLER_HPP_ */
