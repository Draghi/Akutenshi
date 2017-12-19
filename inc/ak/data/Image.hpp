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

#include <ak/math/Vector.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <cstddef>
#include <vector>

namespace akd {
	class Image {
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		public:
			Image() = default;
			virtual ~Image() = default;

			virtual size_t componentCount() const = 0;
			virtual void setComponentCount(size_t targetFormat) = 0;
			virtual const fpSingle* data() const = 0;

			virtual size_t width()  const = 0;
			virtual size_t height() const = 0;
			virtual size_t depth()  const = 0;
	};

	class Image1D : public Image {
		private:
			size_t m_componentCount;
			std::vector<fpSingle> m_imageData;
			size_t m_w;

		public:
			Image1D(const fpSingle* data, size_t components, size_t width);
			Image1D(const Image1D& other);
			Image1D(Image1D&& other);
			~Image1D() override;

			void set(uint32 x, fpSingle value);
			void set(uint32 x, akm::Vec2 value);
			void set(uint32 x, akm::Vec3 value);
			void set(uint32 x, akm::Vec4 value);

			fpSingle asR(uint32 x) const;
			akm::Vec2 asRG(uint32 x) const;
			akm::Vec3 asRGB(uint32 x) const;
			akm::Vec4 asRGBA(uint32 x) const;

			size_t componentCount() const override;
			void setComponentCount(size_t targetFormat) override;
			const fpSingle* data() const override;

			size_t width()  const override;
			size_t height() const override;
			size_t depth()  const override;

			Image1D& operator=(const Image1D& other);
			Image1D& operator=(Image1D&& other);
	};

	class Image2D : public Image {
		private:
			size_t m_componentCount;
			std::vector<fpSingle> m_imageData;
			size_t m_w, m_h;

		public:
			Image2D(const fpSingle* data, size_t components, size_t width, size_t height);
			Image2D(const Image2D& other);
			Image2D(Image2D&& other);
			~Image2D() override;

			void set(uint32 x, uint32 y, fpSingle value);
			void set(uint32 x, uint32 y, akm::Vec2 value);
			void set(uint32 x, uint32 y, akm::Vec3 value);
			void set(uint32 x, uint32 y, akm::Vec4 value);

			fpSingle asR(uint32 x, uint32 y) const;
			akm::Vec2 asRG(uint32 x, uint32 y) const;
			akm::Vec3 asRGB(uint32 x, uint32 y) const;
			akm::Vec4 asRGBA(uint32 x, uint32 y) const;

			size_t componentCount() const override;
			void setComponentCount(size_t targetFormat) override;
			const fpSingle* data() const override;

			size_t width()  const override;
			size_t height() const override;
			size_t depth()  const override;

			Image2D& operator=(const Image2D& other);
			Image2D& operator=(Image2D&& other);
	};

	class Image3D : public Image {
		private:
			size_t m_componentCount;
			std::vector<fpSingle> m_imageData;
			size_t m_w, m_h, m_d;

		public:
			Image3D(const fpSingle* data, size_t components, size_t width, size_t height, size_t depth);
			Image3D(const Image3D& other);
			Image3D(Image3D&& other);
			~Image3D() override;

			void set(uint32 x, uint32 y, uint32 z, fpSingle value);
			void set(uint32 x, uint32 y, uint32 z, akm::Vec2 value);
			void set(uint32 x, uint32 y, uint32 z, akm::Vec3 value);
			void set(uint32 x, uint32 y, uint32 z, akm::Vec4 value);

			fpSingle asR(uint32 x, uint32 y, uint32 z) const;
			akm::Vec2 asRG(uint32 x, uint32 y, uint32 z) const;
			akm::Vec3 asRGB(uint32 x, uint32 y, uint32 z) const;
			akm::Vec4 asRGBA(uint32 x, uint32 y, uint32 z) const;

			size_t componentCount() const override;
			void setComponentCount(size_t targetFormat) override;
			const fpSingle* data() const override;

			size_t width()  const override;
			size_t height() const override;
			size_t depth()  const override;

			Image3D& operator=(const Image3D& other);
			Image3D& operator=(Image3D&& other);
	};
}

#endif
