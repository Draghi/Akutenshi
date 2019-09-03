cmake_minimum_required(VERSION 3.2) 

set(CMAKE_CXX_FLAGS "-O3")

include("${CMAKE_SOURCE_DIR}/cmake/SetupBuildEnvironment.cmake")

project(akutenshi VERSION 1.0.0.0)
sugar_include("${CMAKE_SOURCE_DIR}/src")
add_executable(akutenshi 
	${AK_COMMON_SOURCE} ${AK_ENGINE_SOURCE} 
	${AK_INPUT_SOURCE}
	${AK_RENDER_SOURCE} ${AK_SOUND_SOURCE} 
	${AK_ASSET_SOURCE}
	${AK_GAME_SOURCE}
	${AK_MAIN_SOURCE}
)

# ########### #
# # Compile # #
# ########### #
set_target_properties(akutenshi PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)
target_compile_options(akutenshi PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akutenshi PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})
target_compile_definitions(akutenshi PUBLIC -DGLM_FORCE_RADIANS=1 -DGLM_ENABLE_EXPERIMENTAL=1 -DGLM_FORCE_LEFT_HANDED=1 -DGLM_FORCE_SIZE_FUNC=1 -DGLM_FORCE_INLINE=1)

# ############ #
# # Internal # #
# ############ #
target_include_directories(akutenshi PUBLIC "${CMAKE_SOURCE_DIR}/inc/")
target_include_directories(akutenshi PRIVATE "${CMAKE_SOURCE_DIR}/src/")

# ############ #
# # External # #
# ############ #
target_include_directories(akutenshi SYSTEM PUBLIC  ${STB_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC  ${GLM_INCLUDE_DIR})

target_include_directories(akutenshi SYSTEM PRIVATE ${GLFW_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PRIVATE ${GL4_INCLUDE_DIR})

target_include_directories(akutenshi SYSTEM PRIVATE ${MINI_AL_INCLUDE_DIR})

target_include_directories(akutenshi SYSTEM PRIVATE ${BROTLI_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PRIVATE ${MSGPACK_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PRIVATE ${RAPIDJSON_INCLUDE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(akutenshi PRIVATE Threads::Threads)
if(WIN32) 
	target_link_libraries(akutenshi PRIVATE pthread)
else()
	target_link_libraries(akutenshi PRIVATE bfd)
endif()

target_link_libraries(akutenshi PRIVATE glfw)
target_link_libraries(akutenshi PRIVATE GL4)
target_link_libraries(akutenshi PRIVATE brotlicommon-static)
target_link_libraries(akutenshi PRIVATE brotlidec-static)
target_link_libraries(akutenshi PRIVATE brotlienc-static)