# Include Guard
if(NOT DEFINED GUARD_AK_SRC_AK_SOUND)
set(GUARD_AK_SRC_AK_SOUND 1)

sugar_files(AK_SOURCE Util.cpp Decode.cpp)

sugar_include(backend)

endif()
