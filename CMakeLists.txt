cmake_minimum_required(VERSION 4.0)
project(demix)

if (NOT WIN32 OR NOT MSVC)
    message(FATAL_ERROR "Not support: ${CMAKE_SYSTEM_NAME} ${CMAKE_CXX_COMPILER_ID}")
endif ()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    add_compile_options(/utf-8 /EHsc)
    add_link_options(/SUBSYSTEM:CONSOLE /ENTRY:wmainCRTStartup)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(
                /O2
                /GL
                /GS-
                /DNDEBUG
        )
        add_link_options(
                /BREPRO
                /NOCOFFGRPINFO
                /EMITTOOLVERSIONINFO:NO
                /DEBUG:NONE
                /PDB:NONE
                /LTCG
                /OPT:REF,ICF
                /INCREMENTAL:NO
                /DYNAMICBASE:NO
        )
    endif ()
endif ()

file(GLOB_RECURSE DEMIX_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_executable(${PROJECT_NAME} ${DEMIX_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fftw/include")
target_link_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/fftw/lib")

set(OpenVINO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/openvino/runtime/cmake")
find_package(OpenVINO REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE openvino::runtime fftw3f)
