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

	/**
	 * Base image class to facilitate generic handlers
	 */
	class Image {
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		public:
			Image() = default;
			virtual ~Image() = default;

			/**
			 * Returns the number of colour components in the image (1 - R, 2, RG, 3 RGB, 4 - RGBA etc.)
			 * @return the number of colour components in the image
			 */
			virtual size_t componentCount() const = 0;

			/**
			 * Changes the number of components in the image
			 * @param targetFormat The target number of components for the image
			 * @remarks Setting to a target with 4 or more components will default the 4th component to 1
			 */
			virtual void setComponentCount(size_t targetFormat) = 0;

			/**
			 * Returns the underlying data
			 * @return The underlying data
			 * @remarks Stored left-right, bottom-top, front-back
			 */
			virtual const fpSingle* data() const = 0;

			/**
			 * The width of the image.
			 * @return The width of the image.
			 */
			virtual size_t width()  const = 0;

			/**
			 * The height of the image.
			 * @return The height of the image.
			 * @remarks 1D images will return 0.
			 */
			virtual size_t height() const = 0;

			/**
			 * The depth of the image.
			 * @return The depth of the image.
			 * @remarks 1D and 2D images will return 0.
			 */
			virtual size_t depth()  const = 0;
	};

	/**
	 * 1D Image
	 */
	class Image1D final : public Image {
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

			/**
			 * Returns the number of colour components in the image (1 - R, 2, RG, 3 RGB, 4 - RGBA etc.)
			 * @return the number of colour components in the image
			 */
			size_t componentCount() const override;

			/**
			 * Changes the number of components in the image
			 * @param targetFormat The target number of components for the image
			 * @remarks Setting to a target with 4 or more components will default the 4th component to 1
			 */
			void setComponentCount(size_t targetFormat) override;

			/**
			 * Returns the underlying data
			 * @return The underlying data
			 * @remarks Stored left-right, bottom-top, front-back
			 */
			const fpSingle* data() const override;

			/**
			 * The width of the image.
			 * @return The width of the image.
			 */
			size_t width()  const override;

			/**
			 * The height of the image.
			 * @return The height of the image.
			 * @remarks 1D images will return 0.
			 */
			size_t height() const override;

			/**
			 * The depth of the image.
			 * @return The depth of the image.
			 * @remarks 1D and 2D images will return 0.
			 */
			size_t depth()  const override;

			Image1D& operator=(const Image1D& other);
			Image1D& operator=(Image1D&& other);
	};

	class Image2D final : public Image {
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

			/**
			 * Returns the number of colour components in the image (1 - R, 2, RG, 3 RGB, 4 - RGBA etc.)
			 * @return the number of colour components in the image
			 */
			size_t componentCount() const override;

			/**
			 * Changes the number of components in the image
			 * @param targetFormat The target number of components for the image
			 * @remarks Setting to a target with 4 or more components will default the 4th component to 1
			 */
			void setComponentCount(size_t targetFormat) override;

			/**
			 * Returns the underlying data
			 * @return The underlying data
			 * @remarks Stored left-right, bottom-top, front-back
			 */
			const fpSingle* data() const override;

			/**
			 * The width of the image.
			 * @return The width of the image.
			 */
			size_t width()  const override;

			/**
			 * The height of the image.
			 * @return The height of the image.
			 * @remarks 1D images will return 0.
			 */
			size_t height() const override;

			/**
			 * The depth of the image.
			 * @return The depth of the image.
			 * @remarks 1D and 2D images will return 0.
			 */
			size_t depth()  const override;

			Image2D& operator=(const Image2D& other);
			Image2D& operator=(Image2D&& other);
	};

	class Image3D final : public Image {
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

			/**
			 * Returns the number of colour components in the image (1 - R, 2, RG, 3 RGB, 4 - RGBA etc.)
			 * @return the number of colour components in the image
			 */
			size_t componentCount() const override;

			/**
			 * Changes the number of components in the image
			 * @param targetFormat The target number of components for the image
			 * @remarks Setting to a target with 4 or more components will default the 4th component to 1
			 */
			void setComponentCount(size_t targetFormat) override;

			/**
			 * Returns the underlying data
			 * @return The underlying data
			 * @remarks Stored left-right, bottom-top, front-back
			 */
			const fpSingle* data() const override;

			/**
			 * The width of the image.
			 * @return The width of the image.
			 */
			size_t width()  const override;

			/**
			 * The height of the image.
			 * @return The height of the image.
			 * @remarks 1D images will return 0.
			 */
			size_t height() const override;

			/**
			 * The depth of the image.
			 * @return The depth of the image.
			 * @remarks 1D and 2D images will return 0.
			 */
			size_t depth()  const override;

			Image3D& operator=(const Image3D& other);
			Image3D& operator=(Image3D&& other);
	};
}

#endif
