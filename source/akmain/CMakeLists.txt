cmake_minimum_required(VERSION 3.2)

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

project(akutenshi VERSION 1.0.0.0)

sugar_include("src")

add_executable(akutenshi ${AK_MAIN_SOURCE})

# ################### #
# # Compile Options # #
# ################### #
target_compile_options(akutenshi PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akutenshi PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})
target_compile_definitions(akutenshi PUBLIC -DGLM_FORCE_RADIANS=1 -DGLM_ENABLE_EXPERIMENTAL=1 -DGLM_FORCE_LEFT_HANDED=1 -DGLM_FORCE_SIZE_FUNC=1 -DGLM_FORCE_INLINE=1)
set_target_properties(akutenshi PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)

# ############ #
# # Internal # #
# ############ #
target_include_directories(akutenshi PUBLIC  inc)
target_include_directories(akutenshi PRIVATE src)

target_include_directories(akutenshi SYSTEM PUBLIC ${AK_ASSET_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${AK_COMMON_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${AK_ENGINE_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${AK_GAME_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${AK_INPUT_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${AK_MATH_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${AK_RENDER_INCLUDE_DIR})
target_include_directories(akutenshi SYSTEM PUBLIC ${AK_SOUND_INCLUDE_DIR})

target_link_libraries(akutenshi akasset)
target_link_libraries(akutenshi akcommon)
target_link_libraries(akutenshi akengine)
target_link_libraries(akutenshi akgame)
target_link_libraries(akutenshi akinput)
target_link_libraries(akutenshi akmath)
target_link_libraries(akutenshi akrender)
target_link_libraries(akutenshi aksound)

# ############ #
# # External # #
# ############ #
target_include_directories(akutenshi SYSTEM PUBLIC ${BACKWARD_INCLUDE_DIR})

if(NOT WIN32)
	target_link_libraries(akutenshi bfd)
endif()

