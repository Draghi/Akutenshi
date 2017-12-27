# Include Guard
if(NOT DEFINED GUARD_AK_SRC_AK_DATA)
set(GUARD_AK_SRC_AK_DATA 1)

sugar_files(AK_SOURCE Path.cpp PValue.cpp Json.cpp MPac.cpp Image.cpp BrotliCompression.cpp)

endif()
