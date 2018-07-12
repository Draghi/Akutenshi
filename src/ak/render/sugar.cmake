# Include Guard
if(NOT DEFINED GUARD_AK_SRC_AK_RENDER)
set(GUARD_AK_SRC_AK_RENDER 1)

 sugar_files(AK_SOURCE 
 	DebugDraw.cpp
 	SceneRendererDefault.cpp
)

sugar_include(gl)

endif()
