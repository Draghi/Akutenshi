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
		"-Wno-weak-vtables" "-Wno-exit-time-destructors" "-Wno-global-constructors"
		"-Wno-double-promotion" "-Wno-float-equal" 
		# Disable warnings about conversions between int sizes, but not between fp and int, or signs.
		"-Wno-conversion" "-Wfloat-conversion")
		
    SET(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")
		
elseif("${CMAKE_CXX_COMPILER}" MATCHES ".*g\\+\\+")
	list(APPEND AK_BUILD_CXX_WARNING_FLAGS 
		"-Wall" "-Wextra" "-Wpedantic"
		"-Wno-c++11-compat" "-Wno-padded")
else()
	message(WARNING "Unsupported compiler" ${CMAKE_CXX_COMPILER})
endif()

###############
# CMake Setup #
###############
# Setup CMake variables
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${AK_OUTPUT_BIN}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${AK_OUTPUT_LIB}")
set(BUILD_SHARED_LIBS ON CACHE INTERNAL "" FORCE)

# Set defaults for tweakble cmake variables
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Hide variables
set(CMAKE_INSTALL_PREFIX "install/" CACHE INTERNAL "${AK_OUTPUT_ROOT}" FORCE)

###############
# Build Setup #
###############
# Setup Sugar
set(SUGAR_ROOT "${CMAKE_SOURCE_DIR}/cmake/sugar")
list(APPEND CMAKE_MODULE_PATH 
	"${SUGAR_ROOT}/cmake/module/collecting"
	"${SUGAR_ROOT}/cmake/module/core"
	"${SUGAR_ROOT}/cmake/module/print"
	"${SUGAR_ROOT}/cmake/module/utility"
)
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
set(GLFW_USE_MIR OFF CACHE INTERNAL "" FORCE)
set(GLFW_USE_OSMESA OFF CACHE INTERNAL "" FORCE)
set(GLFW_USE_WAYLAND OFF CACHE INTERNAL "" FORCE)
set(GLFW_VULKAN_STATIC OFF CACHE INTERNAL "" FORCE)

add_subdirectory("${CMAKE_SOURCE_DIR}/cmake/glfw")

##############
# Build GL3  #
##############
add_subdirectory("${CMAKE_SOURCE_DIR}/cmake/GL4")

################
# Build Brotli #
################
set(BROTLI_BUNDLED_MODE ON CACHE INTERNAL "" FORCE)
# set(ENABLE_SANITIZER "undefined")
add_subdirectory("${CMAKE_SOURCE_DIR}/cmake/brotli")

##########################
# External Library Paths #
##########################
set(BACKWARD_INCLUDE_DIR  "${CMAKE_SOURCE_DIR}/cmake/backward-cpp")
set(BROTLI_INCLUDE_DIR    "${CMAKE_SOURCE_DIR}/cmake/brotli/c/include")
set(GLM_INCLUDE_DIR       "${CMAKE_SOURCE_DIR}/cmake/glm")
set(GLFW_INCLUDE_DIR      "${CMAKE_SOURCE_DIR}/cmake/glfw/include")
set(GL4_INCLUDE_DIR       "${CMAKE_SOURCE_DIR}/cmake/GL4/inc")
set(MSGPACK_INCLUDE_DIR   "${CMAKE_SOURCE_DIR}/cmake/msgpack-c/include")
set(RAPIDJSON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/rapidjson/include")
set(STB_INCLUDE_DIR       "${CMAKE_SOURCE_DIR}/cmake/stb")
set(MINI_AL_INCLUDE_DIR   "${CMAKE_SOURCE_DIR}/cmake/mini_al")

##########################
# Internal Library Paths #
##########################
set(AK_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/inc")

##########
# Finish #
##########
set(AK_BUILD_ENV_SETUP 1)