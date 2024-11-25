cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

set(CMAKE_SUPPRESS_REGENERATION true)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(MSVC_INCREMENTAL_DEFAULT OFF)

project(essayer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER cl)
set(CMAKE_VS_PLATFORM_NAME "x86")

set(CMAKE_SHARED_LIBRARY_PREFIX "")

if(NOT MSVC)
    message(FATAL_ERROR "Must use Visual Studio!")
endif()

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    message(FATAL_ERROR "Must use 32-bit")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

include_directories(${PROJECT_SOURCE_DIR}/YRpp)
include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB_RECURSE MY_SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_library(${PROJECT_NAME} SHARED
${MY_SRC_FILES}
${PROJECT_SOURCE_DIR}/dllmain.cpp
${PROJECT_SOURCE_DIR}/YRpp/StaticInits.cpp
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _WIN32_WINNT=0x0601
    _WINDLL
    HAS_EXCEPTIONS=0
    _CRT_SECURE_NO_WARNINGS
    NTDDI_VERSION=0x06010000
    _ALLOW_RTCc_IN_STL
    __STDC_FORMAT_MACROS
    __STDC_LIMIT_MACROS
    _CRT_SECURE_NO_DEPRECATE
    _CRT_NONSTDC_NO_DEPRECATE
    _WINSOCK_DEPRECATED_NO_WARNINGS
    _USE_32BIT_TIME_T

    SYR_VER=2
    I_BORROW_CONSOLE
)

foreach(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_MINSIZEREL
				 CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
	if(${flag_var} MATCHES "/RTC")
		string(REGEX REPLACE "/RTC[^ ]*" "" ${flag_var} "${${flag_var}}")
	endif()
endforeach(flag_var)

target_compile_options(${PROJECT_NAME} PUBLIC
    /W4
    /GF
    /Gz
    /Zc:rvalueCast
    /Zc:forScope
    /analyze-
    /Gy
    /Zc:wchar_t
    /EHsc
    /wd4100
    /wd4201
    /wd4530
    /wd4731
    /wd4740
    /wd4458
    /wd4819
    /wd5103
    /wd5105
    /wd26812
    /arch:IA32
    /fp:fast
    /Qfast_transcendentals
    /Gs-
)

target_link_options(${PROJECT_NAME} PRIVATE
    $<$<NOT:$<CONFIG:Debug>>:/OPT:REF /OPT:ICF>
)
