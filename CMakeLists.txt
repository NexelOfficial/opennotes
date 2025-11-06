cmake_minimum_required(VERSION 3.16)
project(OPENNOTES)
set(CMAKE_CXX_STANDARD 20)
add_compile_options("/utf-8")

# VCPKG directories
set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/x86-windows")
set(VCPKG_INCLUDE "${VCPKG_ROOT}/include")
set(VCPKG_LIB "${VCPKG_ROOT}/lib")
set(VCPKG_BIN "${VCPKG_ROOT}/bin")

# Libs directores
set(LIBS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs")
set(LIBS_INCLUDE "${LIBS_ROOT}/include")
set(LIBS_LIB "${LIBS_ROOT}/lib")


# Create a variable containing all the source files and one with all libraries
file(GLOB_RECURSE ALL_SOURCES src/*.cpp)
file(GLOB ALL_LIBRARIES ${VCPKG_BIN}/*.dll)

# Create the library using the source files
add_compile_definitions(W32)
add_executable(OPENNOTES ${ALL_SOURCES})
set_target_properties(OPENNOTES PROPERTIES OUTPUT_NAME "onotes")

# Include Libs folder
target_include_directories(OPENNOTES PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(OPENNOTES PRIVATE ${VCPKG_INCLUDE})
target_include_directories(OPENNOTES PRIVATE ${LIBS_INCLUDE})

target_link_libraries(OPENNOTES ${VCPKG_LIB}/*.lib ${LIBS_LIB}/*.lib crypt32 bcrypt)