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

#ifndef AK_RENDER_DRAW_HPP_
#define AK_RENDER_DRAW_HPP_

#include <ak/math/Matrix.hpp>
#include <ak/PrimitiveTypes.hpp>
#include <ak/render/Types.hpp>

namespace akr {

	void init();

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

	void setUniform(uint32 bindingLocation, uint32 x);
	void setUniform(uint32 bindingLocation, uint32 x, uint32 y);
	void setUniform(uint32 bindingLocation, uint32 x, uint32 y, uint32 z);
	void setUniform(uint32 bindingLocation, uint32 x, uint32 y, uint32 z, uint32 w);

	void setUniform(uint32 bindingLocation, int32 x);
	void setUniform(uint32 bindingLocation, int32 x, int32 y);
	void setUniform(uint32 bindingLocation, int32 x, int32 y, int32 z);
	void setUniform(uint32 bindingLocation, int32 x, int32 y, int32 z, int32 w);

	void setUniform(uint32 bindingLocation, fpSingle x);
	void setUniform(uint32 bindingLocation, fpSingle x, fpSingle y);
	void setUniform(uint32 bindingLocation, fpSingle x, fpSingle y, fpSingle z);
	void setUniform(uint32 bindingLocation, fpSingle x, fpSingle y, fpSingle z, fpSingle w);

	void setUniform(uint32 bindingLocation, akm::Vec4 vec);
	void setUniform(uint32 bindingLocation, akm::Vec3 vec);
	void setUniform(uint32 bindingLocation, akm::Vec2 vec);

	void setUniform(uint32 bindingLocation, akm::Mat4 matrix);
	void setUniform(uint32 bindingLocation, akm::Mat3 matrix);
	void setUniform(uint32 bindingLocation, akm::Mat2 matrix);
}

#endif
