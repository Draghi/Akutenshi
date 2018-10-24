cmake_minimum_required(VERSION 3.2) 
project(akengine VERSION 1.0.0.0)

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

sugar_include("src")

add_library(akengine STATIC ${AK_ENGINE_SOURCE})

# ################### #
# # Compile Options # #
# ################### #
target_compile_options(akengine PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akengine PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})
set_target_properties(akengine PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)

# ##################### #
# # Internal Includes # #
# ##################### #
target_include_directories(akengine PUBLIC  inc)
target_include_directories(akengine PRIVATE src)

target_include_directories(akengine SYSTEM PUBLIC ${AK_COMMON_INCLUDE_DIR})
target_include_directories(akengine SYSTEM PUBLIC ${AK_MATH_INCLUDE_DIR})

target_link_libraries(akengine akcommon)
target_link_libraries(akengine akmath)

# ##################### #
# # External Includes # #
# ##################### #
target_include_directories(akengine SYSTEM PUBLIC ${BROTLI_INCLUDE_DIR})
target_include_directories(akengine SYSTEM PUBLIC ${MSGPACK_INCLUDE_DIR})
target_include_directories(akengine SYSTEM PUBLIC ${RAPIDJSON_INCLUDE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(akengine Threads::Threads)
if(WIN32) 
	target_link_libraries(akengine pthread) 
endif()

target_link_libraries(akengine brotlicommon-static)
target_link_libraries(akengine brotlidec-static)
target_link_libraries(akengine brotlienc-static)