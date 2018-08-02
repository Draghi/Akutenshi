# Include Guard
if(NOT DEFINED GUARD_AK_SRC_AK_DATA)
set(GUARD_AK_SRC_AK_DATA 1)

sugar_files(AK_SOURCE PVPath.cpp PValue.cpp Json.cpp MsgPack.cpp Brotli.cpp)

endif()
