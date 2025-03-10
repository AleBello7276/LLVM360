#pragma once
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include "misc/Utils.h"
#include "Decoder/Instruction.h"
#include "Decoder/InstructionDecoder.h"
#include <chrono>
#include "IR/IRGenerator.h"
#include <Xex/XexLoader.h>
#include <conio.h>  // for _kbhit
#include <IR/Unit/UnitTesting.h>
#include <IR/IRFunc.h>

// Debug
bool printINST = false;
bool printFile = false;  // Set this flag to true or false based on your preference
bool genLLVMIR = true;
bool isUnitTesting = false;
bool doOverride = false; // if it should override the endAddress to debug
uint32_t overAddr = 0x82060150;

// Benchmark / static analysis
uint32_t instCount = 0;

// Naive+ stuff
XexImage* loadedXex;
IRGenerator* g_irGen;
llvm::LLVMContext cxt;
llvm::Module* mod = new llvm::Module("Xenon", cxt);
llvm::IRBuilder<llvm::NoFolder> builder(cxt);

Section* findSection(std::string name);

struct pDataInfo
{
    uint32_t funcAddr;
    union
    {
        uint32_t info;
        struct
        {
            uint32_t length : 8;
            uint32_t funcLength : 22;
            uint32_t flag1 : 1;
            uint32_t flag2 : 1;
        };
    };
};

void flow_pData()
{
    Section* pData = findSection(".pdata");

    if (pData)
    {
        const auto baseAddress = loadedXex->GetBaseAddress();
        const auto sectionBaseAddress = baseAddress + pData->GetVirtualOffset();
        const auto endAddress = sectionBaseAddress + pData->GetVirtualSize();
        const auto address = sectionBaseAddress;
        const uint8_t* m_imageDataPtr = pData->GetImage()->GetMemory() +
            (pData->GetVirtualOffset() - pData->GetImage()->GetBaseAddress());
        const uint64_t offset = address - pData->GetVirtualOffset();
        const uint8_t* stride = m_imageDataPtr + offset;

        const uint32_t size = pData->GetVirtualSize() / 8;

        for (uint32_t i = 0; i < size; i++)
        {
            pDataInfo info;
            info.funcAddr = SwapInstrBytes(*(uint32_t*)(stride + (i * 8)));
            info.info = SwapInstrBytes(*(uint32_t*)(stride + (i * 8) + 4));

            IRFunc* func = g_irGen->getCreateFuncInMap(info.funcAddr);
            func->end_address = (info.funcAddr + (info.funcLength * 4) - 4);
        }


        printf("%u Function bounds found in .pData\n", size);

    }
}


//
// find and promote save/rest to functions
//
void flow_promoteSaveRest(uint32_t start, uint32_t end)
{
    while (start < end)
    {
        if (start == end - 4) break;
        Instruction instr = g_irGen->instrsList.at(start);
        Instruction instrAfter = g_irGen->instrsList.at(start + 4);

        //
        // savegprlr_14
        //
        if (instr.instrWord == 0xf9c1ff68)
        {
            printf("savegprlr_14 Found at: %08X\n", instr.address);
            for (size_t i = 14; i < 32; i++)
            {
                IRFunc* func = g_irGen->getCreateFuncInMap(instr.address + (i - 14) * 4);
                func->end_address = func->start_address + ((32 - i) * 4) + 4;
            }
        }

        //
        // restgprlr_14
        //
        if (instr.instrWord == 0xe9c1ff68)
        {
            printf("restgprlr_14 Found at: %08X\n", instr.address);
            for (size_t i = 14; i < 32; i++)
            {
                IRFunc* func = g_irGen->getCreateFuncInMap(instr.address + (i - 14) * 4);
                func->end_address = func->start_address + ((32 - i) * 4) + 8;
            }
        }

        //
        // savefpr_14
        //
        if (instr.instrWord == 0xd9ccff70)
        {
            printf("savefpr_14 Found at: %08X\n", instr.address);
            DebugBreak();
        }
       //
       // restfpr_14
       //
        if (instr.instrWord == 0xc9ccff70)
        {
            printf("restfpr_14 Found at: %08X\n", instr.address);
            DebugBreak();
        }

        //
        // savevmx_14
        //
        if (instr.instrWord == 0x3960fee0)
        {
            if (instrAfter.instrWord == 0x7dcb61ce)
            {
                printf("savevmx_14 Found at: %08X\n", instr.address);
                DebugBreak();
            }
        }
        //
        // restvmx_14
        //
        if (instr.instrWord == 0x3960fee0)
        {
            if (instrAfter.instrWord == 0x7dcb60ce)
            {
                printf("restvmx_14 Found at: %08X\n", instr.address);
                DebugBreak();
            }
        }

        //
        // savevmx_64
        //
        if (instr.instrWord == 0x3960fc00)
        {
            if (instrAfter.instrWord == 0x100b61cb)
            {
                printf("savevmx_64 Found at: %08X\n", instr.address);
                DebugBreak();
            }
        }
        //
        // restvmx_64
        //
        if (instr.instrWord == 0x3960fc00)
        {
            if (instrAfter.instrWord == 0x100b60cb)
            {
                printf("restvmx_64 Found at: %08X\n", instr.address);
                DebugBreak();
            }
        }


        start += 4;
    }
}

//
// this flow pass find every function prologue that are jumped from BL instructions
//
void flow_blJumps(uint32_t start, uint32_t end)
{
    while (start < end)
    {
        Instruction instr = g_irGen->instrsList.at(start);
        if (strcmp(instr.opcName.c_str(), "bl") == 0)
        {
            uint32_t target = instr.address + signExtend(instr.ops[0], 24);
            if (!g_irGen->isIRFuncinMap(target))
            {
                printf("{flow_blJumps} Found new start of function bounds at: %08X\n", target);
                g_irGen->getCreateFuncInMap(target);
            }
        }

        start += 4;
    }
}

//
// this flow pass search every prolouge with mfspr R12 LR 
// and since i know that if it save the LR the epilogue will restore it with
// mtspr (check epilogue passes) so i set a metadata flag
//
void flow_mfsprProl(uint32_t start, uint32_t end)
{
    while (start < end)
    {
        Instruction instr = g_irGen->instrsList.at(start);
        if (instr.instrWord == 0x7d8802a6)              // mfspr r12, LR
        {
            if (!g_irGen->isIRFuncinMap(start))
                printf("{flow_mfsprProl} Found new start of function bounds at: %08X\n", start);
            IRFunc* func = g_irGen->getCreateFuncInMap(start);
            func->startW_MFSPR_LR = true;
        }

        start += 4;
    }
}
void flow_aftBclrProl(uint32_t start, uint32_t end)
{
    while (start < end)
    {
        Instruction instr = g_irGen->instrsList.at(start);
        if (strcmp(instr.opcName.c_str(), "bclr") == 0)
        {
            if (!g_irGen->isIRFuncinMap(start + 4))
            {
                printf("{flow_aftBclrProl} Found new start of function bounds at: %08X\n", start + 4);
                g_irGen->getCreateFuncInMap(start + 4);
            }
        }

        start += 4;
    }
}

void flow_promoteTailProl(uint32_t start, uint32_t end)
{
    while (start < end)
    {
      
        Instruction instr = g_irGen->instrsList.at(start);
        Instruction instrAfter = g_irGen->instrsList.at(start + 4);
        if (strcmp(instr.opcName.c_str(), "b") == 0)
        {
            uint32_t target = instr.address + signExtend(instr.ops[0], 24);
            if (strcmp(instrAfter.opcName.c_str(), "nop") == 0)
            {
                // promote branched address to function if not in map
                if (!g_irGen->isIRFuncinMap(target))
                {
                    printf("{flow_bclrAndTailEpil} Promoted new function at: %08X\n", target);
                    g_irGen->getCreateFuncInMap(target);
                }
                break;
            }

            // it's a tail call 100%
            if (g_irGen->isIRFuncinMap(start + 4))
            {
                // promote branched address to function if not in map
                if (!g_irGen->isIRFuncinMap(target)) // probably the actual end
                {
                    printf("{flow_bclrAndTailEpil} Promoted new function at: %08X\n", target);
                    g_irGen->getCreateFuncInMap(target);
                }
                break;
            }
        }



        start += 4;
    }
}

void flow_mtsprEpil(uint32_t start, uint32_t end)
{
    for (const auto& pair : g_irGen->m_function_map)
    {
        IRFunc* func = pair.second;
        if (func->startW_MFSPR_LR && func->end_address == 0)
        {
            start = func->start_address;
            while (start < end)
            {
                Instruction instr = g_irGen->instrsList.at(start);
                if (instr.instrWord == 0x7d8803a6)            // mtspr LR, r12
                {

                    // this account for cases where there's some LD instructions
                    uint32_t off = start + 4;
                    do
                    {
                        instr = g_irGen->instrsList.at(off);
                        off += 4;
                    } while (strcmp(instr.opcName.c_str(), "bclr") != 0);

                    func->end_address = off - 4;
                    printf("{flow_mtsprEpil} Found new end of function bounds at: %08X\n", func->end_address);
                    break;
                }

                start += 4;
            }
        }
    }
}


void flow_bclrAndTailEpil(uint32_t start, uint32_t end)
{
    for (const auto& pair : g_irGen->m_function_map)
    {
        IRFunc* func = pair.second;
        if (func->end_address == 0)
        {
            start = func->start_address;
            while (start < end)
            {

                Instruction instr = g_irGen->instrsList.at(start);
                Instruction instrAfter = g_irGen->instrsList.at(start + 4);
                if (strcmp(instr.opcName.c_str(), "b") == 0)
                {
                    uint32_t target = instr.address + signExtend(instr.ops[0], 24);
                    if (strcmp(instrAfter.opcName.c_str(), "nop") == 0)
                    {
                        func->end_address = start;
                        printf("{flow_bclrAndTailEpil} Found new end of function bounds at: %08X\n", func->end_address);
                        break;
                    }

                    if (g_irGen->isIRFuncinMap(target))
                    {
                        func->end_address = start;
                        if (g_irGen->isIRFuncinMap(start + 4)) // probably the actual end
                        {
                            printf("{flow_bclrAndTailEpil} Found new end of function bounds at: %08X\n", func->end_address);
                            break;
                        }
                    }

                    // if it detects no function in map, it checks if the next address is a function, if it is
                    // then it's the epilogue of the current one, and is indeed a tail call
                    if (g_irGen->isIRFuncinMap(start + 4))
                    {
                        func->end_address = start;
                        printf("{flow_bclrAndTailEpil} Found new end of function bounds at: %08X\n", func->end_address);
                        break;
                    }
                }

                if (strcmp(instr.opcName.c_str(), "bclr") == 0)
                {
                    func->end_address = start;
                    if (strcmp(instrAfter.opcName.c_str(), "nop") == 0)
                    {
                        func->end_address = start;
                        printf("{flow_bclrAndTailEpil} Found new end of function bounds at: %08X\n", func->end_address);
                        break;
                    }
                    
                    if (g_irGen->isIRFuncinMap(start + 4))
                    {
                        func->end_address = start;
                        printf("{flow_bclrAndTailEpil} Found new end of function bounds at: %08X\n", func->end_address);
                        break;
                    }
                }

                start += 4;
            }
        }
    }
}

//
// Detect "standart" tail calls, it checks if the function is in the map
//
void flow_stdTailDEpil(uint32_t start, uint32_t end)
{
    for (const auto& pair : g_irGen->m_function_map)
    {
        IRFunc* func = pair.second;
        if (func->end_address == 0)
        {
            start = func->start_address;
            while (start < end)
            {
                Instruction instr = g_irGen->instrsList.at(start);
                if (strcmp(instr.opcName.c_str(), "b") == 0)
                {
                    uint32_t target = instr.address + signExtend(instr.ops[0], 24);
                    if(g_irGen->isIRFuncinMap(target))
                    {
                        func->end_address = start;
                        if (g_irGen->isIRFuncinMap(start + 4)) // probably the actual end
                        {
                            printf("{flow_stdTailDEpil} Found new end of function bounds at: %08X\n", func->end_address);
                            break;
                        }
                    }
                }

                start += 4;
            }
        }
    }
}