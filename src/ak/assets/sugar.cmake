# Include Guard
if(NOT DEFINED GUARD_AK_SRC_AK_ASSETS)
set(GUARD_AK_SRC_AK_ASSETS 1)

sugar_files(AK_SOURCE Convert.cpp AssetRegistry.cpp)

sugar_include(gltf)

endif()
