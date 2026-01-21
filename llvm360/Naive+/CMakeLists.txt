cmake_minimum_required(VERSION 3.30)

project(Naive+)


set(Loader 
    src/Loader/PEImage.h
    src/Loader/XexImage.h
    src/Loader/ImageLoader.cpp
    src/Loader/ImageLoader.h
    src/Loader/table/ImportTable.h
    src/Loader/AES/AES.h
    src/Loader/AES/AES.cpp
)


set(DECODER
    src/Decoder/InstructionRegistry.cpp
    src/Decoder/InstructionRegistry.h
)

set(IR
    src/IR/IRGen.h
    src/IR/IRGen.cpp

    #src/IR/IRFunc.cpp
    #src/IR/IRFunc.h
    #src/IR/IRGenerator.cpp
    #src/IR/IRGenerator.h
)

set(SRC
    src/LLVM360.cpp
    src/Shared.h
    src/Logger.h 
    src/Naive+/Naive+.h

    ${Loader}
    ${DECODER}
    ${IR}
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/external/llvm_build/include)
add_executable(Naive+ ${CMAKE_CURRENT_SOURCE_DIR}/${SRC})
target_link_libraries(Naive+ PRIVATE EXT_LLVM)