#include "InstructionRegistry.h"


InstructionRegistry g_instrRegistry;


InstructionRegistry::InstructionRegistry()
{
	registerInstructions();
}



void InstructionRegistry::InitialiseOpCodeKey(uint32_t mainOP, uint32_t extMASK)
{
	OpcodeKey val = { mainOP, extMASK };
	m_mainOPs.try_emplace(mainOP, val);
}

void InstructionRegistry::AddDescriptorToKey(uint32_t mainOP, std::string mnemonic, FormType type, uint32_t extOP)
{
	auto it = m_mainOPs.find(mainOP);
	if (it == m_mainOPs.end()) { printf("AddDescriptorToKey, Something is wrong :/, did you called InitialiseOpCodeKey for this main Opcode? "); _CrtDbgBreak(); }
	OpcodeKey& key = it->second;
	uint32_t shift = __builtin_ctz(key.m_extMASK);
	InstructionDescriptor desc = { mnemonic, type };

	key.m_descriptors.try_emplace(extOP << shift, desc);
}


// search "MAIN OP: <n>"
void InstructionRegistry::registerInstructions()
{
	// MAIN OP: 0
	InitialiseOpCodeKey(0, 0);
	AddDescriptorToKey(0, "PADDING", FormType::FORM_PADDING);


	// MAIN OP: 4
	InitialiseOpCodeKey(4, 0x7FF);
	AddDescriptorToKey(4, "mtvscr", FormType::FORM_VX, 1604);
	AddDescriptorToKey(4, "mfvscr", FormType::FORM_VX, 1540);
	AddDescriptorToKey(4, "stvlxl128", FormType::FORM_VX, 1799);
	AddDescriptorToKey(4, "stvlxl128", FormType::FORM_VX, 1803);
	AddDescriptorToKey(4, "stvlxl128", FormType::FORM_VX, 1807);


	// MAIN OP: 7
	InitialiseOpCodeKey(7, 0);
	AddDescriptorToKey(7, "mulli", FormType::FORM_D);
	
	// MAIN OP: 8
	InitialiseOpCodeKey(8, 0);
	AddDescriptorToKey(8, "subfic", FormType::FORM_D);

	// MAIN OP: 10
	InitialiseOpCodeKey(10, 0);
	AddDescriptorToKey(10, "cmpli", FormType::FORM_D);
	// MAIN OP: 11
	InitialiseOpCodeKey(11, 0);
	AddDescriptorToKey(11, "cmpi", FormType::FORM_D);

	// MAIN OP: 13
	InitialiseOpCodeKey(13, 0);
	AddDescriptorToKey(13, "addic.", FormType::FORM_D);


	// MAIN OP: 14
	// MAIN OP: 15
	InitialiseOpCodeKey(14, 0);
	AddDescriptorToKey(14, "addi", FormType::FORM_D);
	InitialiseOpCodeKey(15, 0);
	AddDescriptorToKey(15, "addi", FormType::FORM_D);


	// MAIN OP: 16
	InitialiseOpCodeKey(16, 0);
	AddDescriptorToKey(16, "bcx", FormType::FORM_B);

	// MAIN OP: 17
	InitialiseOpCodeKey(17, 0);
	AddDescriptorToKey(17, "sc", FormType::FORM_SC);
	
	// MAIN OP: 18
	InitialiseOpCodeKey(18, 0);
	AddDescriptorToKey(18, "bx", FormType::FORM_I);

	// MAIN OP: 19
	InitialiseOpCodeKey(19, 0x7FE);
	AddDescriptorToKey(19, "bclrx", FormType::FORM_XL, 16);
	AddDescriptorToKey(19, "bcctrx", FormType::FORM_XL, 528);
	AddDescriptorToKey(19, "isync", FormType::FORM_XL, 150);

	// MAIN OP: 20
	InitialiseOpCodeKey(20, 0);
	AddDescriptorToKey(20, "rlwimix", FormType::FORM_M);

	// MAIN OP: 21
	InitialiseOpCodeKey(21, 0);
	AddDescriptorToKey(21, "rlwinmx", FormType::FORM_M);

	// MAIN OP: 24
	InitialiseOpCodeKey(24, 0);
	AddDescriptorToKey(24, "ori", FormType::FORM_D);

	// MAIN OP: 25
	InitialiseOpCodeKey(25, 0);
	AddDescriptorToKey(25, "oris", FormType::FORM_D);

	// MAIN OP: 26
	InitialiseOpCodeKey(26, 0);
	AddDescriptorToKey(26, "xori", FormType::FORM_D);

	// MAIN OP: 28
	InitialiseOpCodeKey(28, 0);
	AddDescriptorToKey(28, "andi.", FormType::FORM_D);

	// MAIN OP: 30
	InitialiseOpCodeKey(30, 0xC);
	AddDescriptorToKey(30, "rldiclx", FormType::FORM_MD, 0);
	AddDescriptorToKey(30, "rldicrx", FormType::FORM_MD, 1);
	AddDescriptorToKey(30, "rldimix", FormType::FORM_MD, 3);

	// MAIN OP: 31
	InitialiseOpCodeKey(31, 0x7FE);
	AddDescriptorToKey(31, "mfspr", FormType::FORM_XFX, 339);
	AddDescriptorToKey(31, "mtspr", FormType::FORM_XFX, 467);
	AddDescriptorToKey(31, "orx", FormType::FORM_X, 444);
	AddDescriptorToKey(31, "cmpl", FormType::FORM_X, 32);
	AddDescriptorToKey(31, "cntlzwx", FormType::FORM_X, 26);
	AddDescriptorToKey(31, "mfmsr", FormType::FORM_X, 83);
	AddDescriptorToKey(31, "mtmsrd", FormType::FORM_X, 178);
	AddDescriptorToKey(31, "lwarx", FormType::FORM_X, 20);
	AddDescriptorToKey(31, "stwcx.", FormType::FORM_X, 150);
	AddDescriptorToKey(31, "addx", FormType::FORM_XO, 266);
	AddDescriptorToKey(31, "subfx", FormType::FORM_XO, 40);
	AddDescriptorToKey(31, "cmp", FormType::FORM_X, 0);
	AddDescriptorToKey(31, "lbzx", FormType::FORM_X, 87);
	AddDescriptorToKey(31, "extshx", FormType::FORM_X, 922);
	AddDescriptorToKey(31, "sync", FormType::FORM_X, 598);
	AddDescriptorToKey(31, "sld", FormType::FORM_X, 27);
	AddDescriptorToKey(31, "andc", FormType::FORM_X, 60);
	AddDescriptorToKey(31, "mftb", FormType::FORM_XFX, 371);
	AddDescriptorToKey(31, "extswx", FormType::FORM_X, 986);
	AddDescriptorToKey(31, "negx", FormType::FORM_XO, 104);
	AddDescriptorToKey(31, "lwzx", FormType::FORM_X, 23);
	AddDescriptorToKey(31, "lhzx", FormType::FORM_X, 279);
	AddDescriptorToKey(31, "srawix", FormType::FORM_X, 824);
	AddDescriptorToKey(31, "addze", FormType::FORM_XO, 202);
	AddDescriptorToKey(31, "stbx", FormType::FORM_X, 215);
	AddDescriptorToKey(31, "sthx", FormType::FORM_X, 407);
	AddDescriptorToKey(31, "andx", FormType::FORM_X, 28);
	AddDescriptorToKey(31, "ldarx", FormType::FORM_X, 84);
	AddDescriptorToKey(31, "stdcx.", FormType::FORM_X, 214);
	AddDescriptorToKey(31, "stwx", FormType::FORM_X, 151);
	AddDescriptorToKey(31, "eieio", FormType::FORM_X, 854);
	AddDescriptorToKey(31, "lwbrx", FormType::FORM_X, 534);
	AddDescriptorToKey(31, "stwbrx", FormType::FORM_X, 662);
	AddDescriptorToKey(31, "subfex", FormType::FORM_XO, 136);
	AddDescriptorToKey(31, "xorx", FormType::FORM_X, 316);
	AddDescriptorToKey(31, "divdux", FormType::FORM_XO, 457);
	AddDescriptorToKey(31, "sradix", FormType::FORM_XS, 826); // shifted extOP
	AddDescriptorToKey(31, "sradix", FormType::FORM_XS, 827); // shifted extOP
	AddDescriptorToKey(31, "subfcx", FormType::FORM_XO, 8);
	AddDescriptorToKey(31, "slwx", FormType::FORM_X, 24);
	AddDescriptorToKey(31, "extsbx", FormType::FORM_X, 954);
	AddDescriptorToKey(31, "norx", FormType::FORM_X, 124);
	AddDescriptorToKey(31, "mulhdux", FormType::FORM_XO, 9);
	AddDescriptorToKey(31, "mfcr", FormType::FORM_X, 19);
	AddDescriptorToKey(31, "mfcrf", FormType::FORM_XFX, 144);
	AddDescriptorToKey(31, "dcbf", FormType::FORM_X, 86);
	AddDescriptorToKey(31, "icbi", FormType::FORM_X, 982);
	AddDescriptorToKey(31, "dcbst", FormType::FORM_X, 54);
	AddDescriptorToKey(31, "mtmsr", FormType::FORM_X, 146);
	AddDescriptorToKey(31, "stwux", FormType::FORM_X, 183);
	AddDescriptorToKey(31, "lvxl", FormType::FORM_X, 359);
	AddDescriptorToKey(31, "stvxl", FormType::FORM_X, 487);




	
	//int32_t ext21_11 = ibf->GetAt(21, 11);
	//        switch (ext21_11) {
	//            // VMX LOADS
	//        case 12:
	//            INST("lvsl", b6_5, b11_5, b16_5)
	//        case 1038:
	//            INST("lvlx", S, b11_5, b16_5)
	//        case 1550:
	//            INST("lvlxl", S, b11_5, b16_5)
	//        case 1102:
	//            INST("lvrx", S, b11_5, b16_5)
	//        case 1614:
	//            INST("lvrxl", S, b11_5, b16_5)
	//        case 206:
	//            INST("lvx", S, b11_5, b16_5)
	//        case 718:
	//            INST("lvxl", S, b11_5, b16_5)
	//        case 76:
	//            INST("lvsr", S, b11_5, b16_5)
	//
	//                // STORE
	//
	//        case 270:
	//            INST("stvebx", b6_5, b11_5, b16_5)
	//        case 334:
	//            INST("stvehx", b6_5, b11_5, b16_5)
	//        case 398:
	//            INST("stvewx", b6_5, b11_5, b16_5)
	//        case 1294:
	//            INST("stvlx", b6_5, b11_5, b16_5)
	//        case 1806:
	//            INST("stvlxl", b6_5, b11_5, b16_5)
	//        case 1358:
	//            INST("stvrx", b6_5, b11_5, b16_5)
	//        case 1870:
	//            INST("stvrxl", b6_5, b11_5, b16_5)
	//        case 462:
	//            INST("stvx", b6_5, b11_5, b16_5)
	//        case 974:
	//            INST("stvxl", b6_5, b11_5, b16_5)
	//        }


	// MAIN OP: 32
	InitialiseOpCodeKey(32, 0);
	AddDescriptorToKey(32, "lwz", FormType::FORM_D);

	// MAIN OP: 34
	InitialiseOpCodeKey(34, 0);
	AddDescriptorToKey(34, "lbz", FormType::FORM_D);


	// MAIN OP: 36
	// MAIN OP: 37
	InitialiseOpCodeKey(36, 0);
	AddDescriptorToKey(36, "stw", FormType::FORM_D);
	InitialiseOpCodeKey(37, 0);
	AddDescriptorToKey(37, "stwu", FormType::FORM_D);

	// MAIN OP: 38
	InitialiseOpCodeKey(38, 0);
	AddDescriptorToKey(38, "stb", FormType::FORM_D);

	// MAIN OP: 40
	InitialiseOpCodeKey(40, 0);
	AddDescriptorToKey(40, "lhz", FormType::FORM_D);

	// MAIN OP: 42
	InitialiseOpCodeKey(42, 0);
	AddDescriptorToKey(42, "lha", FormType::FORM_D);

	// MAIN OP: 44
	InitialiseOpCodeKey(44, 0);
	AddDescriptorToKey(44, "sth", FormType::FORM_D);

	// MAIN OP: 50
	InitialiseOpCodeKey(50, 0);
	AddDescriptorToKey(50, "lfd", FormType::FORM_D);

	// MAIN OP: 54
	InitialiseOpCodeKey(54, 0);
	AddDescriptorToKey(54, "stfd", FormType::FORM_D);

	// MAIN OP: 58
	InitialiseOpCodeKey(58, 0x2);
	AddDescriptorToKey(58, "ld", FormType::FORM_DS, 0);
	AddDescriptorToKey(58, "ldu", FormType::FORM_DS, 1);
	AddDescriptorToKey(58, "lwa", FormType::FORM_DS, 2);

	// MAIN OP: 62
	InitialiseOpCodeKey(62, 0x2);
	AddDescriptorToKey(62, "std", FormType::FORM_DS, 0);
	AddDescriptorToKey(62, "stdu", FormType::FORM_DS, 1);

	// MAIN OP: 63
	InitialiseOpCodeKey(63, 0x7FE);
	AddDescriptorToKey(63, "mtfsfx", FormType::FORM_XFL, 711);
	AddDescriptorToKey(63, "mffsx", FormType::FORM_X, 583);

}

