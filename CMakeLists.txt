cmake_minimum_required(VERSION 3.10)
project(TSAPP_test VERSION 0.1.0 LANGUAGES C CXX)

# Use vcpkg include/lib paths directly
find_path(RAYLIB_INCLUDE_DIR raylib.h
  PATHS "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/include"
        "D:/Software/vcpkg/installed/x64-windows/include"
)

find_library(RAYLIB_LIBRARY raylib
  PATHS "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/lib"
        "D:/Software/vcpkg/installed/x64-windows/lib"
)

if (NOT RAYLIB_INCLUDE_DIR OR NOT RAYLIB_LIBRARY)
  message(FATAL_ERROR "Failed to find raylib manually.")
endif()

add_executable(TSAPP_test main.cpp)

target_include_directories(TSAPP_test PRIVATE ${RAYLIB_INCLUDE_DIR})
target_link_libraries(TSAPP_test PRIVATE ${RAYLIB_LIBRARY})
