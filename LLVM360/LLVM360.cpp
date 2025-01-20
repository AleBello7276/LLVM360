#include <iostream>
#include <fstream> 
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include "Utils.h"
#include "XexLoader.h"
#include "Instruction.h"
#include "InstructionDecoder.h"




int main()
{
	// Parse the XEX file. and use as file path a xex file in a folder relative to the exetubable
	XexImage* xex = new XexImage(L"MinecraftX360.xex");
    xex->LoadXex();
	


	for (size_t i = 0; i < xex->GetNumSections(); i++)
	{

		const Section* section = xex->GetSection(i);

		if (!section->CanExecute())
		{
			continue;
		}

		printf("Processing section %s\n", section->GetName().c_str());

		// compute code range
		const auto baseAddress = xex->GetBaseAddress();
		const auto sectionBaseAddress = baseAddress + section->GetVirtualOffset();
		const auto endAddress = baseAddress + section->GetVirtualOffset() + section->GetVirtualSize();

		// process all instructions in the code range
		auto address = sectionBaseAddress;
		bool forceBlockStart = false;
		int lastCount = -1;

		InstructionDecoder decoder(section);

		printf("\n\n\n");

		while (address < endAddress)
		{
			Instruction instruction;
			const auto instructionSize = decoder.GetInstructionAt(address, instruction);
			if (instructionSize == 0)
			{
				printf("Failed to decode instruction at %08X\n", address);
				break;
			}

			// decode instruction bytes to Intruction struct for later use
			// use the instruction and emit LLVM IR code



			// print the instruction + operands


			
			std::ostringstream oss;
			oss << std::hex << std::uppercase << std::setfill('0');
			oss << std::setw(8) << address << ":   " << instruction.opcName;

			for (size_t i = 0; i < instruction.ops.size(); ++i) {
				oss << " " << std::setw(2) << static_cast<int>(instruction.ops.at(i));
			}

			std::string output = oss.str();
			printf("%s\n", output.c_str());
			// move to the next instruction
			address += instructionSize;
		}
	}

	

    printf("Hello, World!\n");
}