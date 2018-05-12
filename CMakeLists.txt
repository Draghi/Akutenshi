cmake_minimum_required(VERSION 3.2) 

include("${CMAKE_SOURCE_DIR}/cmake/SetupBuildEnvironment.cmake")

project(akutenshi VERSION 1.0.0.0)

sugar_include("src")
add_executable(akutenshi ${AK_SOURCE})

target_include_directories(akutenshi PUBLIC  inc)
target_include_directories(akutenshi PRIVATE src)

target_include_directories(akutenshi SYSTEM PUBLIC ${GLM_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${ORDERED_MAP_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${RAPIDJSON_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${BACKWARD_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${GLFW_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${GL4_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${STB_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${SMILE_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${MSGPACK_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${BROTLI_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${CXXOPTS_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${CPPCODEC_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${ASSIMP_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${VFSPP_INCLUDE_DIR})

target_compile_options(akutenshi PUBLIC ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akutenshi PUBLIC ${AK_BUILD_CXX_COMPILER_FLAGS})

target_compile_definitions(akutenshi PUBLIC -DGLM_FORCE_RADIANS=1 -DGLM_ENABLE_EXPERIMENTAL=1 -DGLM_FORCE_LEFT_HANDED=1 -DGLM_FORCE_SIZE_FUNC=1 -DGLM_FORCE_INLINE=1)

set_target_properties(akutenshi PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)

find_package(Threads REQUIRED)
target_link_libraries(akutenshi Threads::Threads)

target_link_libraries(akutenshi assimp)
target_link_libraries(akutenshi brotlicommon-static)
target_link_libraries(akutenshi brotlidec-static)
target_link_libraries(akutenshi brotlienc-static)
target_link_libraries(akutenshi glfw)
target_link_libraries(akutenshi GL4)


if(WIN32)
	target_link_libraries(akutenshi pthread)
else()
	target_link_libraries(akutenshi bfd)
endif()
