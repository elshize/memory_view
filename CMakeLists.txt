cmake_minimum_required(VERSION 3.9)
project(memory_view VERSION 0.1 LANGUAGES CXX)

add_library(memory_view INTERFACE)
add_library(memory_view::memory_view ALIAS memory_view)

target_include_directories(memory_view INTERFACE
    $<BUILD_INTERFACE:${memory_view_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

find_package(GSL REQUIRED)
target_link_libraries(memory_view INTERFACE GSL::gsl)
install(FILES include/memory_view.hpp DESTINATION include)
export(PACKAGE memory_view)

enable_testing()
add_subdirectory(test)
