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

#ifndef AK_RENDER_DRAW_HPP_
#define AK_RENDER_DRAW_HPP_

#include <ak/math/Types.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/gl/Types.hpp>

namespace akr {
	namespace gl {
		class Buffer;
	}
}

namespace akr {
	namespace gl {

		void init();

		void setViewport(const akm::Vec2& offset, const akm::Vec2& size);

		void draw(DrawType mode, uint32 vertexCount, uint32 offset = 0);
		void drawIndexed(DrawType mode, IDataType indexType, uint32 vertexCount, uint32 offset = 0);

		void clear(ClearMode clearMode = ClearMode::All);
		void setClearColour(fpSingle red, fpSingle green, fpSingle blue, fpSingle alpha = 0);
		void setClearDepth(fpSingle depth);
		void setClearStencil(int32 stencil);

		void setFillMode(FillMode fillMode, Face face = Face::FrontAndBack);

		void enableDepthTest(bool state);
		void enableCullFace(bool state);

		void setDepthTestMode(DepthMode depthMode);
		void setCullFaceMode(CullMode cullMode);
	}
}

#endif
