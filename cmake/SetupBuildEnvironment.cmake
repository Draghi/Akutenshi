###################
# Akutenshi Setup # 
###################

# Setup Internal Variables
set(AK_OUTPUT_ROOT "${CMAKE_SOURCE_DIR}/dist" CACHE string "The root directory of where the built project will be deployed")
set(AK_OUTPUT_BIN  "${AK_OUTPUT_ROOT}/bin")
set(AK_OUTPUT_LIB  "${AK_OUTPUT_ROOT}/lib")
set(AK_OUTPUT_DATA "${AK_OUTPUT_ROOT}/dat")

# Setup Build Flags
set(AK_BUILD_CXX_WARNING_FLAGS "")
set(AK_BUILD_CXX_COMPILER_FLAGS "")

if ("${CMAKE_CXX_COMPILER}" MATCHES ".*clang")
	list(APPEND AK_BUILD_CXX_WARNING_FLAGS
		"-Weverything"
		"-Wno-c++11-compat" "-Wno-c++98-c++11-compat" "-Wno-c++11-compat-pedantic"
		"-Wno-c++98-compat-pedantic" "-Wno-c99-compat" "-Wno-c++98-compat"
		"-Wno-padded" "-Wno-disabled-macro-expansion" "-Wno-covered-switch-default"
		"-Wno-weak-vtables" "-Wno-exit-time-destructors" "-Wno-global-constructors")
    
    SET(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")
		
elseif(CMAKE_COMPILER_IS_GNUCXX)
	list(APPEND AK_BUILD_CXX_WARNING_FLAGS 
		"-Wall" "-Wextra" "-Wpedantic"
		"-Wno-c++11-compat" "-Wno-padded")
elseif(WIN32)
	list(APPEND AK_BUILD_CXX_COMPILER_FLAGS "/std:c++17" "/EHsc")
else()
	message(WARNING "Unsupported compiler" ${CMAKE_CXX_COMPILER})
endif()

###############
# Build Setup # 
###############

# Setup Sugar
set(SUGAR_ROOT "${CMAKE_SOURCE_DIR}/cmake/sugar/")
include("${SUGAR_ROOT}/cmake/Sugar")
include(sugar_include)
include(sugar_files)

# Hide sugar variables
set(SUGAR_STATUS_PRINT OFF CACHE INTERNAL "" FORCE)
set(SUGAR_STATUS_DEBUG OFF CACHE INTERNAL "" FORCE)

###############
# Build GLFW  # 
###############

set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(GLFW_DOCUMENT_INTERNALS OFF CACHE INTERNAL "" FORCE)
set(GLFW_INSTALL OFF CACHE INTERNAL "" FORCE)
set(LIB_SUFFIX "" CACHE INTERNAL "" FORCE)

add_subdirectory("${CMAKE_SOURCE_DIR}/cmake/glfw")

##############
# Build GL3  # 
##############

add_subdirectory("${CMAKE_SOURCE_DIR}/cmake/GL4")

#################
# Library Paths # 
#################

set(GLM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/glm")
set(ORDERED_MAP_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/ordered-map/src")
set(RAPIDJSON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/rapidjson/include")
set(BACKWARD_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/backward-cpp")
set(GLFW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/glfw/include")
set(GL4_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/GL4/inc")
set(STB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/stb")

###############
# CMake Setup #
###############

# Setup CMake variables
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${AK_OUTPUT_BIN}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${AK_OUTPUT_LIB}")

# Set defaults for tweakble cmake variables
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Hide variables
set(CMAKE_INSTALL_PREFIX "install/" CACHE INTERNAL "${AK_OUTPUT_ROOT}" FORCE)



