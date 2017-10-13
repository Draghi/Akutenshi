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

#ifndef AK_DATA_VECTOR2D_HPP_
#define AK_DATA_VECTOR2D_HPP_

#include <stddef.h>
#include <stdexcept>
#include <vector>

namespace ak {
	namespace data {

		template<typename type_t> class Vector2D {
			private:
				std::vector<type_t> m_vector;
				size_t m_width, m_height;

			public:
				Vector2D() : m_width(0), m_height(0) {}
				Vector2D(size_t w, size_t h) : m_width(w), m_height(h) { m_vector.resize(w*h); }

				type_t& at(size_t index) { return m_vector[index]; }
				const type_t& at(size_t index) const { return m_vector[index]; }

				type_t& at(size_t x, size_t y) {
					if ((x > m_width) || (y > m_height)) throw std::out_of_range("Attempt to index out of range.");
					return m_vector[y*m_width + x];
				}

				const type_t& at(size_t x, size_t y) const {
					if ((x > m_width) || (y > m_height)) throw std::out_of_range("Attempt to index out of range.");
					return m_vector[y*m_width + x];
				}

				type_t* data() { return m_vector.data(); }
				const type_t* data() const { return m_vector.data(); }

				void setSize(size_t nWidth, size_t nHeight) {
					setHeight(nHeight);
					setWidth(nWidth);
				}

				void setWidth(size_t nWidth) {
					std::vector<type_t> nVec;
					nVec.resize(nWidth * m_height);

					size_t mWidth = m_width < nWidth ? m_width : nWidth;
					for(size_t j = 0; j < m_height; j++)
						for(size_t i = 0; i < mWidth; i++)
							nVec[j * nWidth + i] = m_vector[j * m_width + i];

					m_vector = std::move(nVec);
					m_width = nWidth;
				}

				void setHeight(size_t nHeight) {
					m_height = nHeight;
					m_vector.resize(m_height * m_width);
				}

				size_t size() const { return m_vector.size(); }
				size_t width() const { return m_width; }
				size_t height() const { return m_height; }
		};

	}
}

#endif
