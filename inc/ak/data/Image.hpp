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

#ifndef AK_DATA_IMAGE_HPP_
#define AK_DATA_IMAGE_HPP_

#include <ak/data/PValue.hpp>
#include <ak/math/Scalar.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <glm/common.hpp>
#include <vector>

namespace akd {

	template<typename type_t> class Image final {
		private:
			std::vector<type_t> m_data;

			akSize m_components;
			akSize m_width, m_height, m_depth;

		public:
			Image() : m_data(), m_components(0), m_width(0), m_height(0), m_depth(0) {}

			Image(const std::vector<type_t>& data, akSize components, akSize width, akSize height, akSize depth) : m_data(data), m_components(components), m_width(width), m_height(akm::max(width ? 1u : 0u, height)), m_depth(akm::max(width ? 1u : 0u, depth)) {}

			Image(std::vector<type_t>&& data, akSize components, akSize width, akSize height, akSize depth) : m_data(data), m_components(components), m_width(width), m_height(akm::max(width ? 1u : 0u, height)), m_depth(akm::max(width ? 1u : 0u, depth)) {}

			Image(const type_t* data, akSize components, akSize width, akSize height, akSize depth) : m_data(), m_components(components), m_width(width), m_height(akm::max(width ? 1u : 0u, height)), m_depth(akm::max(width ? 1u : 0u, depth)) {
				if ((data == nullptr) || (m_width == 0) || (m_components == 0)) return;
				m_data.resize(m_width*m_height*m_depth*m_components);
				std::memcpy(m_data.data(), data, m_data.size()*sizeof(type_t));
			}

			Image(const Image& other) : m_data(other.m_data), m_components(other.m_components), m_width(other.m_width), m_height(other.m_height), m_depth(other.m_depth) {}
			Image(Image&& other) : m_data(std::move(other.m_data)), m_components(other.m_components), m_width(other.m_width), m_height(other.m_height), m_depth(other.m_depth) {}

			void setComponents(akSize newComponents) {
				if ((m_width == 0) || (newComponents == m_components)) return;
				if (newComponents == 0) {
					m_data.clear();
					m_width = 0; m_height = 0; m_depth = 0;
					m_components = 0;
					return;
				}

				akSize minComp = std::min(newComponents, m_components);
				std::vector<type_t> newData;
				newData.resize(m_width*m_height*m_depth*newComponents);
				for(size_t i = 0; i < m_width*m_height*m_depth; i++) {
					for(akSize j = 0; i < minComp; i++) {
						newData[i*newComponents + j] = m_data[i*m_components + j];

					}
					// @todo Add min/max value to template and fix this line
					if ((newComponents < m_components) && (newComponents == 4)) newData[i*newComponents + 3] = 1;
				}


			}

			type_t* data() {
				return m_data.data();
			}

			type_t* row(akSize row) {
				if (row > m_height) return nullptr;
				return m_data.data() + row*rowSize();
			}

			type_t* layer(akSize layer) {
				if (layer > m_depth) return nullptr;
				return m_data.data() + layer*layerSize();
			}

			const type_t* data() const {
				return m_data.data();
			}

			const type_t* row(akSize row) const {
				if (row > m_height) return nullptr;
				return m_data.data() + row*rowSize();
			}

			const type_t* layer(akSize layer) const {
				if (layer > m_depth) return nullptr;
				return m_data.data() + layer*layerSize();
			}

			akSize components() const {
				return m_components;
			}

			akSize width() const {
				return m_width;
			}

			akSize height() const {
				return m_height;
			}

			akSize depth() const {
				return m_depth;
			}

			akSize size() const {
				return static_cast<akSize>(m_data.size());
			}

			akSize rowSize() const {
				return m_components*m_width;
			}

			akSize layerSize() const {
				return m_components*m_width*m_height;
			}

			Image& operator=(const Image& other) {
				m_data = other.m_data;
				m_components = other.m_components;
				m_width = other.m_width;
				m_height = other.m_height;
				m_depth = other.m_depth;
				return *this;
			}

			Image& operator=(Image&& other) {
				m_data = std::move(other.m_data);
				m_components = other.m_components;
				m_width = other.m_width;
				m_height = other.m_height;
				m_depth = other.m_depth;
				return *this;
			}
	};

	using ImageF32 = Image<fpSingle>;
	using ImageU8  = Image<uint8>;

	template<typename type_t> void serialize(const Image<type_t>& src, akd::PValue& dst) {
		dst["size"][0].set<akSize>(src.width());
		dst["size"][1].set<akSize>(src.height());
		dst["size"][2].set<akSize>(src.depth());
		dst["components"].set<akSize>(src.components());

		// @todo Not portable across endianess... But it's way too slow otherwise...
		std::vector<uint8> binData;
		binData.resize(src.size()*sizeof(type_t));
		std::memcpy(binData.data(), src.data(), src.size()*sizeof(type_t));
		dst["data"].set<akd::PValue::bin_t>(binData);
	}

	template<typename type_t> bool deserialize(const akd::PValue& src, Image<type_t>& dst) {
		try {
			akSize width  = src["size"][0].as<akSize>();
			akSize height = akm::max(width ? 1u : 0u, src["size"][1].as<akSize>());
			akSize depth  = akm::max(width ? 1u : 0u, src["size"][2].as<akSize>());
			akSize components = src["components"].as<akSize>();

			if ((width == 0) || (components == 0)) return false;

			size_t dataSize = width*height*depth*components;

			std::vector<type_t> data;
			data.resize(dataSize);

			auto& srcData = src["data"].asBin();
			dst = Image<type_t>(reinterpret_cast<const type_t*>(srcData.data()), components, width, height, depth);

			return true;
		} catch(const std::logic_error&) {
			return false;
		}
	}

	std::optional<ImageF32> loadImageF32(const uint8* data, akSize len, bool bottomUp = true);
	std::optional<ImageF32> loadImageF32(const std::vector<std::pair<const uint8*, akSize>>& data, bool bottomUp = true);
	std::optional<ImageU8> loadImageU8(const uint8* data, akSize len, bool bottomUp = true);
	std::optional<ImageU8> loadImageU8(const std::vector<std::pair<const uint8*, akSize>>& data, bool bottomUp = true);

	template<typename type_t> std::optional<Image<type_t>> loadImage(const uint8* data, akSize len, bool bottomUp = true);
	template<typename type_t> std::optional<Image<type_t>> loadImage(const std::vector<std::pair<const uint8*, akSize>>& data, bool bottomUp = true);

	template<> inline std::optional<Image<fpSingle>> loadImage<fpSingle>(const uint8* data, akSize len, bool bottomUp) { return loadImageF32(data, len, bottomUp); }
	template<> inline std::optional<Image<fpSingle>> loadImage<fpSingle>(const std::vector<std::pair<const uint8*, akSize>>& data, bool bottomUp) { return loadImageF32(data, bottomUp); }
	template<> inline std::optional<Image<uint8>> loadImage<uint8>(const uint8* data, akSize len, bool bottomUp) { return loadImageU8(data, len, bottomUp); }
	template<> inline std::optional<Image<uint8>> loadImage<uint8>(const std::vector<std::pair<const uint8*, akSize>>& data, bool bottomUp) { return loadImageU8(data, bottomUp); }

}

#endif
