# Include Guard
if(NOT DEFINED GUARD_AK_SRC_AK_ENGINE)
set(GUARD_AK_SRC_AK_ENGINE 1)

sugar_files(AK_SOURCE Config.cpp EntityManager.cpp Scene.cpp SceneManager.cpp) 

sugar_include(components)
sugar_include(internal)

endif()
