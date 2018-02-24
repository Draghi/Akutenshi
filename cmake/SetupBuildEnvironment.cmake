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
set(GLFW_USE_MIR OFF CACHE INTERNAL "" FORCE)
set(GLFW_USE_OSMESA OFF CACHE INTERNAL "" FORCE)
set(GLFW_USE_WAYLAND OFF CACHE INTERNAL "" FORCE)
set(GLFW_VULKAN_STATIC OFF CACHE INTERNAL "" FORCE)

add_subdirectory("${CMAKE_SOURCE_DIR}/cmake/glfw")

##############
# Build GL3  #
##############

add_subdirectory("${CMAKE_SOURCE_DIR}/cmake/GL4")

#################
# Build Brotli  #
#################

set(BROTLI_BUNDLED_MODE ON CACHE INTERNAL "" FORCE)
# set(ENABLE_SANITIZER "undefined")
add_subdirectory("${CMAKE_SOURCE_DIR}/cmake/brotli")

#################
# Build ASSIMP  #
#################

set(ASSIMP_ANDROID_JNIIOSYSTEM OFF CACHE INTERNAL "" FORCE)

set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE INTERNAL "" FORCE)

set(ASSIMP_BUILD_ASSIMP_TOOLS     ON CACHE INTERNAL "" FORCE)

set(ASSIMP_BUILD_OBJ_IMPORTER     ON CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_COLLADA_IMPORTER ON CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER     ON CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_MD5_IMPORTER     ON CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_OPENGEX_IMPORTER ON CACHE INTERNAL "" FORCE)

set(ASSIMP_BUILD_3DS_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_3D_IMPORTER       OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_3MF_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_AMF_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_AC_IMPORTER       OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASE_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASSBIN_IMPORTER   OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASSXML_IMPORTER   OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_BLEND_IMPORTER    OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_B3D_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_BVH_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_COB_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_CSM_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_DXF_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER     OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_HMP_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_IFC_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_IRRMESH_IMPORTER  OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_IRR_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_LWO_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_LWS_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_MD2_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_MD3_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_MDC_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_MDL_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_MMD_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_MS3D_IMPORTER     OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_NDO_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_NFF_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_OFF_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_OGRE_IMPORTER     OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_PLY_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_Q3BSP_IMPORTER    OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_Q3D_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_RAW_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_SIB_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_SMD_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_STL_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TERRAGEN_IMPORTER OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_X3D_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_XGL_IMPORTER      OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_X_IMPORTER        OFF CACHE INTERNAL "" FORCE)

set(ASSIMP_BUILD_NONFREE_C4D_IMPORTER OFF CACHE INTERNAL "" FORCE)

set(ASSIMP_BUILD_ZLIB          OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS  OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_SAMPLES       OFF CACHE INTERNAL "" FORCE)

set(ASSIMP_ASAN               OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_COVERALLS          OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_DOUBLE_PRECISION   OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_HEADERCHECK        OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_NO_EXPORT          ON  CACHE INTERNAL "" FORCE)
set(ASSIMP_OPT_BUILD_PACKAGES OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_UBSAN              OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_WERROR             OFF CACHE INTERNAL "" FORCE)

set(ASSIMP_BIN_INSTALL_DIR     "bin"     CACHE INTERNAL "" FORCE)
set(ASSIMP_INCLUDE_INSTALL_DIR "include" CACHE INTERNAL "" FORCE)
set(ASSIMP_LIB_INSTALL_DIR     "lib"     CACHE INTERNAL "" FORCE)
set(ASSIMP_PACKAGE_VERSION     0         CACHE INTERNAL "" FORCE)

set(SYSTEM_IRRXML             OFF CACHE INTERNAL "" FORCE)

add_subdirectory("${CMAKE_SOURCE_DIR}/cmake/assimp")

#################
# Library Paths #
#################

set(GLM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/glm")
set(ORDERED_MAP_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/ordered-map")
set(RAPIDJSON_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/rapidjson/include")
set(BACKWARD_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/backward-cpp")
set(GLFW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/glfw/include")
set(GL4_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/GL4/inc")
set(STB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/stb")
set(SMILE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/libsmile/include")
set(MSGPACK_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/msgpack-c/include")
set(BROTLI_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/brotli/c/include")
set(CXXOPTS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/cxxopts/include")
set(CPPCODEC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/cppcodec")
set(ASSIMP_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/assimp/Include" "${CMAKE_BINARY_DIR}/cmake/assimp/Include")
set(SPARSEPP_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/cmake/sparsepp")

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



