cmake_minimum_required(VERSION 3.14)

if(NOT ${CMAKE_ANDROID_ARCH_ABI} STREQUAL arm64-v8a)
	message(FATAL_ERROR "Unsupported target architecture: ${CMAKE_ANDROID_ARCH_ABI}. Please make an issue on the repo!")
endif()

project(adrenotools LANGUAGES CXX C)

add_subdirectory(lib/linkernsbypass)

set(LIB_SOURCES src/bcenabler.cpp
                src/driver.cpp
                include/adrenotools/bcenabler.h
                include/adrenotools/driver.h
                include/adrenotools/priv.h)

add_library(adrenotools ${LIB_SOURCES})

target_include_directories(adrenotools PUBLIC include)
target_include_directories(adrenotools PRIVATE .)
target_compile_options(adrenotools PRIVATE -Wall -Wextra)
target_link_libraries(adrenotools vulkan android linkernsbypass)

add_subdirectory(src/hook)
