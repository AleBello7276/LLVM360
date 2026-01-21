#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>


using f_instrEmitter = void(*)(int, const std::string&);

enum FormType
{
	FORM_UNK,
	FORM_PADDING,
	FORM_I,
	FORM_B,
	FORM_SC,
	FORM_D,
	FORM_DS,
	FORM_X,
	FORM_XO,
	FORM_XL,
	FORM_XFX,
	FORM_M,
	FORM_MD,
	FORM_XS,
	FORM_XFL,
	FORM_VX,
};


union DEFAULTForm
{
	uint32_t raw;
	struct { uint32_t CCC : 26, OPCD : 6; };
};

union VXForm
{
	uint32_t raw;
	struct { uint32_t XO : 11, vB : 5, vA : 5, vD : 5, OPCD : 6; };
};

union IForm
{
	uint32_t raw;
	struct { uint32_t LK : 1, AA : 1, LI : 24, OPCD : 6; };
};

union BForm 
{ 
	uint32_t raw; 
	struct { uint32_t LK : 1, AA : 1, BO : 5, BI : 5, BD : 14, OPCD : 6; };
};

union SCForm
{
	uint32_t raw;
	struct { uint32_t ZERO_1 : 1, ONE : 1, ZERO_24 : 24, OPCD : 6; };
};

union DForm 
{ 
	uint32_t raw; 
	struct { uint32_t IMM : 16, A : 5, D : 5, OPCD : 6; } Base; 
	struct { uint32_t IMM : 16, A : 5, L : 1, ZERO : 1, crfD : 3, OPCD : 6; } CRF;
};

union DSForm
{
	uint32_t raw;
	struct { uint32_t XO : 2, ds : 14, A : 5, D : 5, OPCD : 6; } Base;
};

union MForm
{
	uint32_t raw;
	struct { uint32_t Rc : 1, ME : 5, MB : 5, SH : 5, A : 5, S : 5, OPCD : 6; } M1;
	struct { uint32_t Rc : 1, ME : 5, MB : 5, B : 5, A : 5, S : 5, OPCD : 6; } M2;
};

union MDForm
{
	uint32_t raw;
	struct { uint32_t Rc : 1, sh1 : 1, XO : 3, mb : 6, sh : 5, A : 5, S : 5, OPCD : 6; } MD1;
	struct { uint32_t Rc : 1, sh1 : 1, XO : 3, me : 6, sh : 5, A : 5, S : 5, OPCD : 6; } MD2;
};


// TODO: 
// those are a lot, finish and verify!
union XForm
{
	uint32_t raw;
	struct { uint32_t RC : 1, XO : 10, B : 5, A : 5, D : 5, OPCD : 6; } XA;
	struct { uint32_t RC : 1, XO : 10, B : 5, A : 4, ZERO1 : 1, D : 5, OPCD : 6; } XB;
	struct { uint32_t RC : 1, XO : 10, B : 5, A : 5, L : 1, ZERO1 : 1, crfD : 3, OPCD : 6; } XC;
	struct { uint32_t RC : 1, XO : 10, B : 5, A : 5, ZERO2 : 2, crfD : 3, OPCD : 6; } XD;
};

union XOForm
{
	uint32_t raw;
	struct { uint32_t Rc : 1, XO : 9, OE : 1, B : 5, A : 5, D : 5, OPCD : 6; } XO1;
	struct { uint32_t Rc : 1, XO : 9, ZERO1 : 1, B : 5, A : 5, D : 5, OPCD : 6; } XO2;
	struct { uint32_t Rc : 1, XO : 9, OE : 1, ZERO5 : 5, A : 5, D : 5, OPCD : 6; } XO3;
};

union XLForm
{
	uint32_t raw;
	struct { uint32_t LK : 1, XO : 10, ZERO5 : 5, BI : 5, BO : 5, OPCD : 6; } XL_1;
};

union XFLForm
{
	uint32_t raw;
	struct { uint32_t Rc : 1, XO : 10, B : 5, ZERO1 : 1, FM : 8, ZERO : 1, OPCD : 6; } XFL;
};

union XSForm
{
	uint32_t raw;
	struct { uint32_t Rc : 1, sh1 : 1, XO : 9, sh : 5, A : 5, S : 5, OPCD : 6; } XS;
};

union XFXForm
{
	uint32_t raw;
	struct { uint32_t ZERO1 : 1, XO : 10, spr : 10, D : 5, OPCD : 6; } XFX_1;
};


union InstrOperands
{
	InstrOperands(uint32_t data) : raw(data) {}
	uint32_t raw;
	DEFAULTForm DEF;
	IForm I;
	BForm B;
	SCForm SC;
	DForm D;
	DSForm DS;
	XForm X;
	XOForm XO;
	XLForm XL;
	XFXForm XFX;
	MForm M;
	XSForm XS;
	XFLForm XFL;
	VXForm VX;
};



struct InstructionDescriptor
{
	std::string mnemonic;
	FormType m_Type;
	f_instrEmitter emitter;
};

struct Instruction
{
	InstructionDescriptor desc;
	uint32_t address;

	// raw data and operands
	union
	{
		uint32_t m_rawData;
		InstrOperands m_operands;
	};

	Instruction() {}
};

static InstructionDescriptor unknownDesc = { "UNK", FormType::FORM_UNK, nullptr };

// responsible of indexing opcodes and (if any) extended opcodes
// also responsible of decoding an instruction 
// <data> into an <Instruction> using a <InstructionDescriptor>
struct InstructionRegistry
{
	struct OpcodeKey
	{
		uint32_t m_mainOP;
		uint32_t m_extMASK;
		std::unordered_map<uint32_t, InstructionDescriptor> m_descriptors;
	};
	std::unordered_map<uint8_t, OpcodeKey> m_mainOPs; // 0 ... 63


	InstructionRegistry();
	void registerInstructions();
	void InitialiseOpCodeKey(uint32_t m_mainOP, uint32_t m_extMASK);
	void AddDescriptorToKey(uint32_t mainOP, std::string mnemonic, FormType type, uint32_t extOP = 0);

	Instruction DecodeInstr(uint32_t data, uint32_t address)
	{
		InstrOperands operands = InstrOperands(data);
		Instruction instr;
		
		// get OpcodeKey by main Opcode
		auto it = m_mainOPs.find(operands.DEF.OPCD);
		if (it == m_mainOPs.end()) 
		{ 
			printf("Instruction::DecodeInstr MAIN OPCODE HAS NO KEY : %d\n", operands.DEF.OPCD);
			return makeUnkInstr(data, address);
		}
		
		OpcodeKey& key = it->second;
		uint32_t extOpcode = (data & key.m_extMASK);
		auto itDesc = key.m_descriptors.find(extOpcode);
		if (itDesc == key.m_descriptors.end()) 
		{ 
			printf("Instruction::DecodeInstr OpCodeKey HAS NO DESCRIPTOR FOR THIS EXTOP %d\n", extOpcode >> (__builtin_ctz(key.m_extMASK))); 
			return makeUnkInstr(data, address);
		}
		
		InstructionDescriptor& desc = itDesc->second;
		instr.address = address;
		instr.desc = desc;
		instr.m_rawData = data;
		return instr;
	}

	static Instruction makeUnkInstr(uint32_t data, uint32_t address){
		Instruction instr;
		instr.address = address;
		instr.m_rawData = data;
		instr.desc = unknownDesc;
		return instr;
	}
};



extern InstructionRegistry g_instrRegistry;