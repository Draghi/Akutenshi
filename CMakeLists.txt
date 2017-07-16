cmake_minimum_required(VERSION 3.2) 

include("${CMAKE_SOURCE_DIR}/cmake/SetupBuildEnvironment.cmake")

project(akutenshi VERSION 1.0.0.0)

sugar_include("src")
add_executable(akutenshi ${AK_SOURCE})

target_include_directories(akutenshi PUBLIC inc)
target_include_directories(akutenshi PRIVATE src)

target_compile_options(akutenshi PUBLIC ${AK_BUILD_CXX_WARNING_FLAGS})
target_compile_options(akutenshi PUBLIC ${AK_BUILD_CXX_COMPILER_FLAGS})
set_target_properties(akutenshi PROPERTIES CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON CXX_STANDARD 17)

find_package(Threads REQUIRED)
target_link_libraries(akutenshi Threads::Threads)
