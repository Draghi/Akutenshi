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

#ifndef AK_RENDER_RENDERTARGET_HPP_
#define AK_RENDER_RENDERTARGET_HPP_

#include <vector>

#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/gl/Textures.hpp>

namespace akr {
	namespace gl {

		class RenderBuffer final {
			private:
				uint32 m_id;

			public:
				RenderBuffer();
				RenderBuffer(RenderBuffer&&);
				~RenderBuffer();

				uint32 id() const;
				bool isValid() const;

				RenderBuffer& operator=(RenderBuffer&& other);
		};

		enum class DepthStencilStorage {
			SIZE_D24_S8,
			SIZE_D32F_S8
		};

		enum class DepthStorage {
			SIZE_16,
			SIZE_24,
			SIZE_32,
			SIZE_32F
		};

		enum class StencilStorage {
			SIZE_1,
			SIZE_4,
			SIZE_8,
			SIZE_16
		};

		void newRenderBufferColourStorage(RenderBuffer& renderBuffer, TexFormat format, TexStorage storage, akSize width, akSize height, akSize multisampling);
		void newRenderBufferDepthStorage(RenderBuffer& renderBuffer, DepthStorage depthStorage, akSize width, akSize height, akSize multisampling);
		void newRenderBufferStencilStorage(RenderBuffer& renderBuffer, StencilStorage stencilStorage, akSize width, akSize height, akSize multisampling);
		void newRenderBufferDepthStencilStorage(RenderBuffer& renderBuffer, DepthStencilStorage depthStencilStorage, akSize width, akSize height, akSize multisampling);

		class RenderTarget {
			private:
				uint32 m_id;

			public:
				RenderTarget();
				RenderTarget(RenderTarget&&);
				~RenderTarget();

				uint32 id() const;
				bool isValid() const;

				RenderTarget& operator=(RenderTarget&& other);
		};

		enum BlitMask : uint8 {
			Colour = 0x01,
			Depth = 0x02,
			Stencil = 0x04
		};


		void bindRenderTarget(const RenderTarget& target);
		void bindDisplayRenderTarget();

		void blitRenderTargets(const RenderTarget& src, const RenderTarget& dst, const akm::Vec2& srcStart, const akm::Vec2& srcEnd, const akm::Vec2& dstStart, const akm::Vec2& dstEnd, BlitMask mask, FilterType filter);
		void blitRenderTargetToDisplay(const RenderTarget& src, const akm::Vec2& srcStart, const akm::Vec2& srcEnd, const akm::Vec2& dstStart, const akm::Vec2& dstEnd, BlitMask mask, FilterType filter);
		void blitDisplayToRenderTarget(const RenderTarget& dst, const akm::Vec2& srcStart, const akm::Vec2& srcEnd, const akm::Vec2& dstStart, const akm::Vec2& dstEnd, BlitMask mask, FilterType filter);

		void invalidateColourAttachment(const RenderTarget& target, uint32 index);
		void invalidateColourAttachments(const RenderTarget& target, const std::vector<uint32>& colourIndexes);

		void invalidateDepthAttachment(const RenderTarget& target);
		void invalidateStencilAttachment(const RenderTarget& target);
		void invalidateDepthStencilAttachment(const RenderTarget& target);

		void attachColourBuffer(RenderTarget& target, uint32 index, RenderBuffer& renderBuffer);
		void attachDepthBuffer(RenderTarget& target, RenderBuffer& renderBuffer);
		void attachStencilBuffer(RenderTarget& target, RenderBuffer& renderBuffer);
		void attachDepthStencilBuffer(RenderTarget& target, RenderBuffer& renderBuffer);

		void attachColourTexture(RenderTarget& target, uint32 index, Texture& texture);
		void attachDepthTexture(RenderTarget& target, Texture& texture);
		void attachStencilTexture(RenderTarget& target, Texture& texture);
		void attachDepthStencilTexture(RenderTarget& target, Texture& texture);

		void attachColourTexture(RenderTarget& target, uint32 index, uint32 layer, Texture& texture);
		void attachDepthTexture(RenderTarget& target, uint32 layer, Texture& texture);
		void attachStencilTexture(RenderTarget& target, uint32 layer, Texture& texture);
		void attachDepthStencilTexture(RenderTarget& target, uint32 layer, Texture& texture);

		void detachColour(RenderTarget& target, uint32 index);
		void detachDepth(RenderTarget& target);
		void detachStencil(RenderTarget& target);
		void detachDepthStencil(RenderTarget& target);
	}
}

#endif /* AK_RENDER_RENDERTARGET_HPP_ */
