cmake_minimum_required(VERSION 3.30)
project(external)


if(NOT LLVM_DIR)
    set(LLVM_DIR "${CMAKE_CURRENT_LIST_DIR}/llvm_build/lib/cmake/llvm" CACHE PATH "Path to precompiled LLVM")
endif()
find_package(LLVM REQUIRED CONFIG)

add_library(EXT_LLVM INTERFACE)
target_include_directories(EXT_LLVM INTERFACE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(EXT_LLVM INTERFACE ${LLVM_DEFINITIONS})



# List of LLVM libs you need:
llvm_map_components_to_libnames(LLVM_COMPONENTS
  Core
  Target
  IRReader

  OrcJIT
  Passes

  TableGen
  X86CodeGen
)

target_link_libraries(EXT_LLVM INTERFACE ${LLVM_COMPONENTS})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
