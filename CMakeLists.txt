cmake_minimum_required(VERSION 3.18)
project(class_inheritance)
find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)
set (CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}" "${CMAKE_MODULE_PATH}")
include(AddLLVM)

if (NOT CLANG_LINK_CLANG_DYLIB)
        set (CLANG_LINK_CLANG_DYLIB "ON")
endif()

function(clang_target_link_libraries target type)
  if (CLANG_LINK_CLANG_DYLIB)
    target_link_libraries(${target} ${type} clang-cpp)
  else()
    target_link_libraries(${target} ${type} ${ARGN})
  endif()
endfunction()

add_llvm_library(PrintFunctionNames MODULE class_inheritance.cpp PLUGIN_TOOL clang)
  clang_target_link_libraries(PrintFunctionNames PRIVATE
    clangAST
    clangBasic
    clangFrontend
    )
