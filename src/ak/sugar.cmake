# Include Guard
if(NOT DEFINED GUARD_AK_SRC_AK)
set(GUARD_AK_SRC_AK 1)

sugar_files(AK_SOURCE PrimitiveTypes.cpp String.cpp)

sugar_include(data)
sugar_include(engine)
sugar_include(filesystem)
sugar_include(thread)
sugar_include(time)
sugar_include(log)

endif()
