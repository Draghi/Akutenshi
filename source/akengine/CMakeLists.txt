cmake_minimum_required(VERSION 3.2) 

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

project(akengine VERSION 1.0.0.0)
sugar_include("src")
add_library(akengine STATIC ${AK_ENGINE_SOURCE} ${AK_COMMON_SOURCE})

# ################### #
# # Compile Options # #
# ################### #
set_target_properties(akengine PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)
target_compile_options(akengine PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akengine PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})
target_compile_definitions(akengine PUBLIC -DGLM_FORCE_RADIANS=1 -DGLM_ENABLE_EXPERIMENTAL=1 -DGLM_FORCE_LEFT_HANDED=1 -DGLM_FORCE_SIZE_FUNC=1 -DGLM_FORCE_INLINE=1)

# ##################### #
# # Internal Includes # #
# ##################### #
target_include_directories(akengine PUBLIC  inc)
target_include_directories(akengine PRIVATE src)

# ##################### #
# # External Includes # #
# ##################### #
target_include_directories(akengine SYSTEM PUBLIC  ${GLM_INCLUDE_DIR})
target_include_directories(akengine SYSTEM PRIVATE ${BROTLI_INCLUDE_DIR})
target_include_directories(akengine SYSTEM PRIVATE ${MSGPACK_INCLUDE_DIR})
target_include_directories(akengine SYSTEM PRIVATE ${RAPIDJSON_INCLUDE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(akengine PRIVATE Threads::Threads)
if(WIN32) 
	target_link_libraries(akengine PRIVATE pthread)
endif()

target_link_libraries(akengine PRIVATE brotlicommon-static)
target_link_libraries(akengine PRIVATE brotlidec-static)
target_link_libraries(akengine PRIVATE brotlienc-static)
