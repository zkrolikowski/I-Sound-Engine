cmake_minimum_required(VERSION 3.16)
project(I_Sound_Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

#-g -fsanitize=address
# -Ofast -ffast-math -mavx -mtune=intel
# define the macro DEBUG if compiling in that configuration
# -fno-inline-small-functions
#/Ox /GF /Gy /arch:AVX512 /arch:AVX /arch:AVX2

add_compile_definitions("$<$<CONFIG:DEBUG>:DEBUG>")

if (NOT (OS_Windows OR OS_Linux OR OS_Horizon))
# define OS macros
if (WIN32)
    add_compile_definitions("$<${WIN32}:OS_Windows>")
    set(OS_Windows TRUE)
elseif(UNIX)
    add_compile_definitions("$<$<PLATFORM_ID:Linux>:OS_Linux>")
    set(OS_Linux $<PLATFORM_ID:Linux>)
endif ()

endif()

add_subdirectory(Libraries)
enable_testing()
add_subdirectory(Source)
add_subdirectory(Testing)
add_subdirectory(Tools)
