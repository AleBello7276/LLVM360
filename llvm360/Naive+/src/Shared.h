#pragma once


#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#ifdef _WIN32
#include <conio.h>
#include <Windows.h>
#else
#include <cstdarg>
#endif
#include <chrono>
#include <functional>


#include "Decoder/InstructionRegistry.h"

//#include "IR/IRGenerator.h"
//#include <IR/Unit/UnitTesting.h>
//#include <IR/IRFunc.h>
//
#include "Naive+/Naive+.h"
//
//// Naive+ stuff
//IRGenerator* g_irGen;
//llvm::LLVMContext cxt;
//llvm::Module* mod = new llvm::Module("Xenon", cxt);
//llvm::IRBuilder<llvm::NoFolder> builder(cxt);
//
//uint32_t Swap32(uint32_t val) {
//    return ((((val) & 0xff000000) >> 24) | (((val) & 0x00ff0000) >> 8) | (((val) & 0x0000ff00) << 8) |
//        (((val) & 0x000000ff) << 24));
//}
