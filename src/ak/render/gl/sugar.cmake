# Include Guard
if(NOT DEFINED GUARD_AK_SRC_AK_GL_RENDER)
set(GUARD_AK_SRC_AK_GL_RENDER 1)

sugar_files(AK_SOURCE 
	Buffers.cpp 
	Draw.cpp 
	Shaders.cpp
	Textures.cpp
	VertexArrays.cpp 
	RenderTarget.cpp
)

endif()
