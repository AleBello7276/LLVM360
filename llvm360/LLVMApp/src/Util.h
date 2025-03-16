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
bool printINST = true;
bool printFile = false;  // Set this flag to true or false based on your preference
bool genLLVMIR = true;
bool isUnitTesting = false;
bool doOverride = false; // if it should override the endAddress to debug
bool dbCallBack = true; // enables debug callbacks, break points etc
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

struct EXPMD_Section
{
	uint32_t dat_offset;
    uint32_t size;
	uint32_t flags;
	uint32_t nameLength;
	std::string sec_name;
	uint8_t* data;
};

struct EXPMD_Header
{
    uint32_t magic;
	uint32_t version;
	uint32_t flags;
	uint32_t baseAddress;
	uint32_t numSections;
	EXPMD_Section** sections;
};

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
        std::vector<pDataInfo> infoList;

        for (uint32_t i = 0; i < size; i++)
        {
            pDataInfo info;
            info.funcAddr = SwapInstrBytes(*(uint32_t*)(stride + (i * 8)));
            info.info = SwapInstrBytes(*(uint32_t*)(stride + (i * 8) + 4));

            IRFunc* func = g_irGen->getCreateFuncInMap(info.funcAddr);
            func->end_address = (info.funcAddr + (info.funcLength * 4) - 4);
            infoList.push_back(info);
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

void flow_stackInitProl(uint32_t start, uint32_t end)
{
    bool first = true;
    while (start < end)
    {
        if (start == end - 4) break;
        Instruction instr = g_irGen->instrsList.at(start);
        Instruction nextInstr = g_irGen->instrsList.at(start + 4);;
        Instruction prevInstr;
        if(first)
        {
            prevInstr = Instruction{ start, 0x0, std::string{"nop"}, std::vector<uint32_t>{0, 0, 0} };
            first = false;
        }
        else
        {
            prevInstr = g_irGen->instrsList.at(start - 4);
        }
        if (strcmp(instr.opcName.c_str(), "stw") == 0 && instr.ops[2] == 1)
        {
            if((strcmp(prevInstr.opcName.c_str(), "bclr") == 0 && instr.ops[2] == 1) ||
                (strcmp(prevInstr.opcName.c_str(), "nop") == 0))
            {
                if (!g_irGen->isIRFuncinMap(start))
                {
                    printf("{flow_stackInitProl} Found new start of function bounds at: %08X\n", start);
                    g_irGen->getCreateFuncInMap(start);
                }

                if ((strcmp(nextInstr.opcName.c_str(), "stw") == 0 && instr.ops[2] == 1))
                {
                    start += 8;
                    continue;
                }
            }
            start += 4;
            continue;
        }

        start += 4;
    }
}

void flow_aftBclrProl(uint32_t start, uint32_t end)
{
    while (start < end)
    {
        if (start == end - 4) break;
        Instruction instr = g_irGen->instrsList.at(start);
        Instruction instrAfter;
        if (strcmp(instr.opcName.c_str(), "bclr") == 0)
        {
            instrAfter = g_irGen->instrsList.at(start + 4);
            if(strcmp(instrAfter.opcName.c_str(), "stw") == 0 && instrAfter.ops[2] == 1)
            {
                if (g_irGen->isIRFuncinMap(instrAfter.address))
                {
                    start += 4;
                    continue;
                }

                printf("{flow_aftBclrProl} Found new start of function bounds at: %08X\n", instrAfter.address);
                g_irGen->getCreateFuncInMap(instrAfter.address);
            }


            uint32_t off = start + 4;
            do
            {
                instrAfter = g_irGen->instrsList.at(off);
                off += 4;
            } while (strcmp(instrAfter.opcName.c_str(), "nop") == 0);

            if (g_irGen->isIRFuncinMap(instrAfter.address))
            {
                start += 4;
                continue;
            }

            printf("{flow_aftBclrProl} Found new start of function bounds at: %08X\n", instrAfter.address);
            g_irGen->getCreateFuncInMap(instrAfter.address);
        }

        start += 4;
    }
}

void flow_promoteTailProl(uint32_t start, uint32_t end)
{
    while (start < end)
    {
        if (start == end - 4) break;
        Instruction instr = g_irGen->instrsList.at(start);
        Instruction instrAfter = g_irGen->instrsList.at(start + 4);
        if (strcmp(instr.opcName.c_str(), "b") == 0)
        {
            uint32_t target = instr.address + signExtend(instr.ops[0], 24);
            /*if (strcmp(instrAfter.opcName.c_str(), "nop") == 0)
            {
                // promote branched address to function if not in map
                if (!g_irGen->isIRFuncinMap(target))
                {
                    printf("{flow_bclrAndTailEpil} Promoted new function at: %08X\n", target);
                    g_irGen->getCreateFuncInMap(target);
                }
                start += 4;
                continue;
            }*/

            // it's a tail call 100%
            if (g_irGen->isIRFuncinMap(start + 4))
            {
                // promote branched address to function if not in map
                if (!g_irGen->isIRFuncinMap(target)) // probably the actual end
                {
                    printf("{flow_bclrAndTailEpil} Promoted new function at: %08X\n", target);
                    g_irGen->getCreateFuncInMap(target);
                }
                start += 4;
                continue;
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
                
                

                if (start == end - 4)
                {
                    func->end_address = start;
                    printf("{flow_bclrAndTailEpil} Found new end of function bounds at: %08X\n", func->end_address);
                    break;
                }

                Instruction instr = g_irGen->instrsList.at(start);
                Instruction instrAfter = g_irGen->instrsList.at(start + 4);
                if (strcmp(instr.opcName.c_str(), "b") == 0)
                {

					
                    



                    uint32_t target = instr.address + signExtend(instr.ops[0], 24);
                    if (strcmp(instrAfter.opcName.c_str(), "nop") == 0)
                    {
                        uint32_t offf = start + 4;
                        while (!g_irGen->isIRFuncinMap(offf))
                        {
                            offf += 4;
                        }
                        func->end_address = offf - 4;
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
                            start += 4;
                            continue;
                        }
                    }
                }

                start += 4;
            }
        }
    }
}

void serializeDBMapData(const std::unordered_map<uint32_t, Instruction>& map, const std::string& filename)
{
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs.is_open()) {
        printf("Error opening file for writing\n");
        return;
    }

    // sort by address
    std::vector<Instruction> sorted_instructions;
    for (const auto& entry : map) {
        sorted_instructions.push_back(entry.second);
    }
    std::sort(sorted_instructions.begin(), sorted_instructions.end(),
        [](const Instruction& a, const Instruction& b) { return a.address < b.address; });

    // serialize
    for (const Instruction& inst : sorted_instructions) {
        // address, instrWord
        ofs.write(reinterpret_cast<const char*>(&inst.address), sizeof(inst.address));
        ofs.write(reinterpret_cast<const char*>(&inst.instrWord), sizeof(inst.instrWord));
        // opcName
        uint32_t nameLength = inst.opcName.size();
        ofs.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength)); 
        ofs.write(inst.opcName.c_str(), nameLength + 1); 
        // operands 
        uint32_t opsSize = inst.ops.size();
        ofs.write(reinterpret_cast<const char*>(&opsSize), sizeof(opsSize));
        if (opsSize > 0) {
            ofs.write(reinterpret_cast<const char*>(inst.ops.data()), opsSize * sizeof(uint32_t));
        }
    }

    ofs.close();
    printf("Serialization completed successfully.\n");
}