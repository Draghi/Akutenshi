cmake_minimum_required(VERSION 3.2) 
project(akcommon VERSION 1.0.0.0)

if(NOT DEFINED AK_BUILD_ENV_SETUP)
	include("cmake/SetupBuildEnvironment.cmake")
endif()

sugar_include("src")

add_library(akcommon STATIC ${AK_COMMON_SOURCE})

# ##################### #
# # Internal Includes # #
# ##################### #
target_include_directories(akcommon PUBLIC  inc)
target_include_directories(akcommon PRIVATE src)

target_include_directories(akcommon SYSTEM PUBLIC ${AK_MATH_INCLUDE_DIR})
target_link_libraries(akcommon akmath)

# ##################### #
# # External Includes # #
# ##################### #

# ################### #
# # Compile Options # #
# ################### #
target_compile_options(akcommon PRIVATE ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akcommon PRIVATE ${AK_BUILD_CXX_COMPILER_FLAGS})
set_target_properties(akcommon PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)

# ############# #
# # Libraries # #
# ############# #

