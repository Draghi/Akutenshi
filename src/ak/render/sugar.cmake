# Include Guard
if(NOT DEFINED GUARD_AK_SRC_AK_RENDER)
set(GUARD_AK_SRC_AK_RENDER 1)

sugar_files(AK_SOURCE Pipeline.cpp Buffer.cpp VertexMapping.cpp Draw.cpp Primitives.cpp Texture.cpp)

endif()
