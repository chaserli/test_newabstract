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

include_directories(${PROJECT_SOURCE_DIR}/YRpp)

include_directories(${PROJECT_SOURCE_DIR}/src)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    message(FATAL_ERROR "FUCK FUCK FUCK FUCK! CYKA BLYAT!")
endif()

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
    SYR_VER=2
)

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
    /wd26495
    /wd26812
    /arch:IA32
    /fp:fast
    /Qfast_transcendentals
    /Gs-
)

target_link_libraries(${PROJECT_NAME} PRIVATE
     kernel32
     user32
     gdi32
     winspool
     comdlg32
     advapi32
     shell32
     ole32
     oleaut32
     uuid
     odbc32
     odbccp32
     dbghelp
     onecore
)
