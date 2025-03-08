#pragma once
#include "IRGenerator.h"
#include "IRFunc.h"


//
// Helpers
//

#define BUILD  func->m_irGen->m_builder
#define i64Const(x) BUILD->getInt64(x)
#define i32Const(x) BUILD->getInt32(x)
#define i16Const(x) BUILD->getInt16(x)
#define i1Const(x) BUILD->getInt1(x)

#define i64_T BUILD->getInt64Ty()
#define i32_T BUILD->getInt32Ty()
#define i16_T BUILD->getInt16Ty()
#define i1_T BUILD->getInt1Ty()

#define trcTo16(x) BUILD->CreateTrunc(x, BUILD->getInt16Ty(), "trc16")
#define trcTo32(x) BUILD->CreateTrunc(x, BUILD->getInt32Ty(), "trc32")
#define trcTo64(x) BUILD->CreateTrunc(x, BUILD->getInt64Ty(), "trc64")

// register value access / store
#define gprVal(x) BUILD->CreateLoad(BUILD->getInt64Ty(), func->getRegister("RR", x), "rrV")
#define crVal() BUILD->CreateLoad(BUILD->getInt32Ty(), func->getRegister("CR"), "crV")
// ext
#define sExt64(x) BUILD->CreateSExt(x, BUILD->getInt64Ty(), "sEx64")
#define sExt32(x) BUILD->CreateSExt(x, BUILD->getInt32Ty(), "sEx32")

#define zExt64(x) BUILD->CreateZExt(x, BUILD->getInt64Ty(), "zEx64")
#define zExt32(x) BUILD->CreateZExt(x, BUILD->getInt32Ty(), "zEx32")

#define sign64(x)  llvm::ConstantInt::getSigned(i32_T, x)
#define sign32(x)  llvm::ConstantInt::getSigned(i64_T, x)
#define sign16(x)  llvm::ConstantInt::getSigned(i16_T, x)

inline uint32_t signExtend(uint32_t value, int size)
{
    if (value & (1 << (size - 1))) {
        return value | (~0 << size); 
    }
    return value; 
}

inline bool isBoBit(uint32_t value, uint32_t idx) {
    return (value >> idx) & 0b1;
}

inline llvm::Value* getBOOperation(IRFunc* func, Instruction instr, llvm::Value* bi)
{
    llvm::Value* should_branch{};

    /*0000y Decrement the CTR, then branch if the decremented CTR[M�63] is not 0 and the condition is FALSE.
      0001y Decrement the CTR, then branch if the decremented CTR[M�63] = 0 and the condition is FALSE.
      001zy Branch if the condition is FALSE.
      0100y Decrement the CTR, then branch if the decremented CTR[M�63] is not 0 and the condition is TRUE.
      0101y Decrement the CTR, then branch if the decremented CTR[M�63] = 0 and the condition is TRUE.
      011zy Branch if the condition is TRUE.
      1z00y Decrement the CTR, then branch if the decremented CTR[M�63] is not 0.*/


      // NOTE, remember to cast "bools" with int1Ty, cause if i do CreateNot with an 32 bit value it will mess up the cmp result

    if (!isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        !isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), i32Const(1));
        llvm::Value* isCTRnz = BUILD->CreateICmpNE(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRnz, BUILD->CreateNot(bi), "shBr");
    }
    // 0001y
    if (isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        !isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), i32Const(1));
        llvm::Value* isCTRz = BUILD->CreateICmpEQ(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRz, BUILD->CreateNot(bi), "shBr");
    }
    // 001zy
    if (isBoBit(instr.ops[0], 2) &&
        !isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
		// TODO: optmize this
        should_branch = BUILD->CreateAnd(BUILD->CreateNot(bi, "not"), i1Const(1), "shBr");
    }
    // 0100y
    if (!isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), i32Const(1));
        llvm::Value* isCTRnz = BUILD->CreateICmpNE(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRnz, bi, "shBr");
    }
    // 0101y
    if (isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) &&
        isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), i32Const(1));
        llvm::Value* isCTRnz = BUILD->CreateICmpEQ(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRnz, bi, "shBr");
    }
    // 0b011zy (Branch if condition is TRUE)
    if (isBoBit(instr.ops[0], 2) && isBoBit(instr.ops[0], 3) && !isBoBit(instr.ops[0], 4))
    {
        should_branch = BUILD->CreateAnd(bi, i1Const(1), "shBr");
    }
    // 0b1z00y
    if (!isBoBit(instr.ops[0], 1) && !isBoBit(instr.ops[0], 2) && isBoBit(instr.ops[0], 4))
    {
        BUILD->CreateSub(func->getRegister("CTR"), i32Const(1));
        llvm::Value* isCTRnz = BUILD->CreateICmpNE(func->getRegister("CTR"), 0, "ctrnz");
        should_branch = BUILD->CreateAnd(isCTRnz, i1Const(1), "shBr");
    }

    return should_branch;
}


// TODO refactor
inline void setCRField(IRFunc* func, uint32_t index, llvm::Value* field)
{
    int bitOffset = index * 4;
    llvm::Value* shiftedMask = BUILD->CreateShl(i32Const(0b1111), bitOffset, "shMsk");

    // clear the already stored bits
    llvm::Value* invertedMask = BUILD->CreateNot(shiftedMask, "inMsk");
    llvm::Value* clearedCR = BUILD->CreateAnd(crVal(), invertedMask, "zCR");
    llvm::Value* fieldExtended = zExt32(field); // Cast 'field' to i32 before shifting

    // update bits
    llvm::Value* shiftedFieldValue = BUILD->CreateShl(fieldExtended, bitOffset, "shVal");
    llvm::Value* updatedCR = BUILD->CreateOr(clearedCR, shiftedFieldValue, "uCR");

    BUILD->CreateStore(updatedCR, func->getRegister("CR"));
}

inline llvm::Value* extractCRBit(IRFunc* func, uint32_t BI) {
    // create mask at bit pos
    llvm::Value* mask = i32Const(1);
    llvm::Value* shiftAmount = i32Const(BI);
    llvm::Value* shiftedMask = BUILD->CreateShl(mask, shiftAmount, "shm");

    // mask the bit
    llvm::Value* isolatedBit = BUILD->CreateAnd(crVal(), shiftedMask, "ib");
    return BUILD->CreateIntCast(BUILD->CreateLShr(isolatedBit, shiftAmount, "bi"), BUILD->getInt1Ty(), false,"cast1");
}

inline llvm::Value* updateCRWithValue(IRFunc* func, llvm::Value* result, llvm::Value* value)
{
    // lt gt eq
    llvm::Value* ltBit = zExt32(BUILD->CreateICmpSLT(value, result, "isLT"));
    llvm::Value* gtBit = zExt32(BUILD->CreateICmpSGT(value, result, "isGT"));
    llvm::Value* eqBit = zExt32(BUILD->CreateICmpEQ(value, result, "isEQ"));

    // so // TODO
    //llvm::Value* soBit = xerSO;

    // build cr0 field 
    llvm::Value* crField = BUILD->CreateOr(
        BUILD->CreateOr(ltBit, BUILD->CreateShl(gtBit, 1)),
        BUILD->CreateOr(BUILD->CreateShl(eqBit, 2),
            BUILD->CreateShl(i32Const(0), 3)), // so BIT TODO
        "crField"
    );

    return crField;
}

inline llvm::Value* updateCRWithZero(IRFunc* func, llvm::Value* result)
{
    llvm::Value* zero = i32Const(0);
    // lt gt eq
    llvm::Value* ltBit = zExt32(BUILD->CreateICmpSLT(result, zero, "isLT"));
    llvm::Value* gtBit = zExt32(BUILD->CreateICmpSGT(result, zero, "isGT"));
    llvm::Value* eqBit = zExt32(BUILD->CreateICmpEQ(result, zero, "isEQ"));

    // so // TODO
    //llvm::Value* soBit = xerSO;

    // build cr0 field 
    llvm::Value* crField = BUILD->CreateOr(
        BUILD->CreateOr(ltBit, BUILD->CreateShl(gtBit, 1)),
        BUILD->CreateOr(BUILD->CreateShl(eqBit, 2), 
        BUILD->CreateShl(i32Const(0), 3)), // so BIT TODO
        "cr0Field"
    );

    return crField;
}

inline llvm::Value* getEA(IRFunc* func, uint32_t displ, uint32_t gpr)
{
    llvm::Value* extendedDisplacement = sExt64(llvm::ConstantInt::get(i32_T, llvm::APInt(16, displ, true)));
    llvm::Value* regValue = gprVal(gpr);
    llvm::Value* ea = BUILD->CreateAdd(regValue, extendedDisplacement, "ea");
    return BUILD->CreateIntToPtr(ea, i32_T->getPointerTo(), "addrPtr");
}

//
// INSTRUCTIONS Emitters
//

inline void nop_e(Instruction instr, IRFunc* func)
{
	// best instruction ever
    return;
}

inline void twi_e(Instruction instr, IRFunc* func)
{
    // temp stub
    return;
}

inline void bcctr_e(Instruction instr, IRFunc* func)
{
    // temp stub
    return;
}

inline void mfspr_e(Instruction instr, IRFunc* func)
{
    auto lrValue = BUILD->CreateLoad(BUILD->getInt64Ty(), func->getSPR(instr.ops[1]), "load_spr");
    BUILD->CreateStore(lrValue, func->getRegister("RR", instr.ops[0]));
}


inline void mtspr_e(Instruction instr, IRFunc* func)
{
    auto rrValue = trcTo32(gprVal(instr.ops[1]));
    BUILD->CreateStore(rrValue, func->getSPR(instr.ops[0]));
}

inline void bl_e(Instruction instr, IRFunc* func)
{
    uint32_t target = instr.address + signExtend(instr.ops[0], 24);
	IRFunc* targetFunc = func->m_irGen->getCreateFuncInMap(target);
    func->m_irGen->initFuncBody(targetFunc);

    // outdated:
    // 
    // update link register with the llvm return address of the next ir instruction
    // this is an interesting one, since i really don't have a good way to "store the instr.address + 4" in LR and make it work,
    // here is what i do, i create a new basic block for the address of the next instruction (so instr.address + 4 bytes) and store it
    // in LR, so when LR is used to return, it branch to the basic block so the next instruction
    // i think there is a better way to handle this but.. it should work fine for now :}
    // llvm::BlockAddress* lr_BB = func->getCreateBBinMap(instr.address + 4); fix it


    auto argIter = func->m_irFunc->arg_begin();
    llvm::Argument* arg1 = &*argIter;
    llvm::Argument* arg2 = &*(++argIter);

    BUILD->CreateStore(i32Const(instr.address + 4), func->getRegister("LR"));
	BUILD->CreateCall(targetFunc->m_irFunc, {arg1, i32Const(instr.address + 4)});
}

inline void b_e(Instruction instr, IRFunc* func)
{
    uint32_t target = instr.address + signExtend(instr.ops[0], 24);
    llvm::BasicBlock* target_BB = func->getCreateBBinMap(target);

    BUILD->CreateBr(target_BB);
}

inline void bclr_e(Instruction instr, IRFunc* func)
{
	BUILD->CreateRetVoid();
}

inline void stfd_e(Instruction instr, IRFunc* func)
{
    auto frValue = BUILD->CreateLoad(BUILD->getDoubleTy(), func->getRegister("FR", instr.ops[0]), "load_fr");
    BUILD->CreateStore(frValue, getEA(func, instr.ops[1], instr.ops[2])); // address needs to be a pointer (pointer to an address in memory)
}

inline void stw_e(Instruction instr, IRFunc* func)
{
	// truncate the value to 32 bits
    auto rrValue32 = trcTo32(gprVal(instr.ops[0]));
    BUILD->CreateStore(rrValue32, getEA(func, instr.ops[1], instr.ops[2]));
}

// store word with update, update means that the address register is updated with the new address
// so rA = rA + displacement after the store
inline void stwu_e(Instruction instr, IRFunc* func)
{
	llvm::Value* eaVal = getEA(func, instr.ops[1], instr.ops[2]);
    auto rrValue32 = trcTo32(gprVal(instr.ops[0]));
    BUILD->CreateStore(rrValue32, eaVal);
	BUILD->CreateStore(eaVal, func->getRegister("RR", instr.ops[2])); // update rA
}

inline void addi_e(Instruction instr, IRFunc* func)
{
    llvm::Value* im = sExt64(BUILD->getInt16(instr.ops[2]));
    llvm::Value* rrValue;
    if (instr.ops[1] != 0)
    {
		rrValue = gprVal(instr.ops[1]);
	}
	else
	{
		rrValue = i64Const(0);
	}
    llvm::Value* val = instr.ops[1] ? BUILD->CreateAdd(rrValue, im, "val") : im;
    BUILD->CreateStore(val, func->getRegister("RR", instr.ops[0]));
}



// add immediate shifted
// if rA = 0 then it will use value 0 and not the content of rA (because lis use 0)
inline void addis_e(Instruction instr, IRFunc* func)
{
    llvm::Value* shift = sExt64(i32Const(instr.ops[2] << 16));
    llvm::Value* rrValue;
    if (instr.ops[1] != 0)
    {
        rrValue = gprVal(instr.ops[1]);
    }
    else
    {
        rrValue = i64Const(0);
    }
	llvm::Value* val = instr.ops[1] ? BUILD->CreateAdd(rrValue, shift, "val") : shift;
    BUILD->CreateStore(val, func->getRegister("RR", instr.ops[0]));
}

inline void add_e(Instruction instr, IRFunc* func)
{
    llvm::Value* val = BUILD->CreateAdd(gprVal(instr.ops[1]), gprVal(instr.ops[2]), "val");
    BUILD->CreateStore(val, func->getRegister("RR", instr.ops[0]));
}

inline void bcx_e(Instruction instr, IRFunc* func)
{
    // first check how to manage the branch condition
    // if "should_branch" == True then
    llvm::Value* bi = BUILD->CreateTrunc(extractCRBit(func, instr.ops[1]), BUILD->getInt1Ty(), "tr");
    llvm::Value* should_branch = getBOOperation(func, instr, bi);


    // compute condition BBs
    llvm::BasicBlock* b_true = func->getCreateBBinMap(instr.address + (instr.ops[2] << 2));
    llvm::BasicBlock* b_false = func->getCreateBBinMap(instr.address + 4);

    BUILD->CreateCondBr(should_branch, b_true, b_false);
}

inline void cmpli_e(Instruction instr, IRFunc* func)
{
    printf("------------- FIXXXXX ---------\n");
    llvm::Value* a;
    if(instr.ops[1] = 0)
    {
        llvm::Value* low32Bits = BUILD->CreateTrunc(func->getRegister("RR", instr.ops[2]), llvm::Type::getInt32Ty(func->m_irGen->m_module->getContext()), "low32");
        a = BUILD->CreateZExt(low32Bits, BUILD->getInt64Ty(), "ext");
    } 
    else
    {
        a = func->getRegister("RR", instr.ops[2]);
    }

    llvm::Value* imm = zExt64(i32Const(instr.ops[3]));
    setCRField(func, instr.ops[0], updateCRWithValue(func, imm, a));
}


inline void cmpw_e(Instruction instr, IRFunc* func)
{
    llvm::Value* rA = gprVal(instr.ops[2]);
    llvm::Value* rB = gprVal(instr.ops[3]);
    if(instr.ops[1] != 1)
    {
        rA = sExt64(trcTo32(rA));
        rB = sExt64(trcTo32(rB));
    }
  
    llvm::Value* LT = zExt32(BUILD->CreateICmpSLT(rA, rB, "lt"));
    llvm::Value* GT = zExt32(BUILD->CreateICmpSGT(rA, rB, "gt"));
    llvm::Value* EQ = zExt32(BUILD->CreateICmpEQ(rA, rB, "eq"));
    // TODO
    llvm::Value* SO_bit = i32Const(0);
	llvm::Value* field = zExt32(BUILD->CreateOr(BUILD->CreateOr(BUILD->CreateOr(LT, BUILD->CreateShl(GT, 1, "sh"), "or"), BUILD->CreateShl(EQ, 2, "sh"), "or"), BUILD->CreateShl(SO_bit, 3, "sh"), "or"));

	setCRField(func, instr.ops[0], field);
}

inline void cmpwi_e(Instruction instr, IRFunc* func)
{
    llvm::Value* rA = gprVal(instr.ops[2]);
    llvm::Value* imm = sign16(instr.ops[3]);
    if (instr.ops[1] != 1)
    {
        rA = sExt64(trcTo32(rA));
        imm = sExt64(imm);
    }

    llvm::Value* LT = zExt32(BUILD->CreateICmpSLT(rA, imm, "lt"));
    llvm::Value* GT = zExt32(BUILD->CreateICmpSGT(rA, imm, "gt"));
    llvm::Value* EQ = zExt32(BUILD->CreateICmpEQ(rA, imm, "eq"));

    // TODO
    llvm::Value* SO_bit = i32Const(0);
    llvm::Value* field = zExt32(BUILD->CreateOr(BUILD->CreateOr(BUILD->CreateOr(LT, BUILD->CreateShl(GT, 1, "sh"), "or"), BUILD->CreateShl(EQ, 2, "sh"), "or"), BUILD->CreateShl(SO_bit, 3, "sh"), "or"));

    setCRField(func, instr.ops[0], field);
}


inline void lwz_e(Instruction instr, IRFunc* func)
{
    // EA is the sum(rA | 0) + d.The word in memory addressed by EA is loaded into the low - order 32 bits of rD.The
    // high - order 32 bits of rD are cleared.
    llvm::Value* loadedValue = BUILD->CreateLoad(i32_T, getEA(func, instr.ops[1], instr.ops[2]), "loaded_value");
    BUILD->CreateStore(zExt64(loadedValue), func->getRegister("RR", instr.ops[0]));
}

inline void lwzx_e(Instruction instr, IRFunc* func)
{
    llvm::Value* ea = BUILD->CreateAdd(gprVal(instr.ops[1]), gprVal(instr.ops[2]), "ea");
    llvm::Value* loadedValue = BUILD->CreateLoad(i32_T, BUILD->CreateIntToPtr(ea, i32_T->getPointerTo(), "addrPtr"), "loaded_value");
    BUILD->CreateStore(zExt64(loadedValue), func->getRegister("RR", instr.ops[0]));
}

// could be better
inline void lhz_e(Instruction instr, IRFunc* func)
{
    //EA is the sum(rA | 0) + d.The half word in memory addressed by EA is loaded into the low - order 16 bits of rD.
    //The remaining bits in rD are cleared.
	llvm::Value* ea_val = BUILD->CreateLoad(i16_T, getEA(func, instr.ops[1], instr.ops[2]), "eaV");
    BUILD->CreateStore(zExt64(trcTo16(ea_val)), func->getRegister("RR", instr.ops[0]));
}


inline void sth_e(Instruction instr, IRFunc* func)
{
    //EA is the sum (rA|0) + d. The contents of the low-order 16 bits of rS are stored into the half word in memory
    //addressed by EA.
    llvm::Value* low16 = trcTo16(gprVal(instr.ops[0]));
    BUILD->CreateStore(low16, getEA(func, instr.ops[1], instr.ops[2]));
}

#define DMASK(b, e) (((0xFFFFFFFF << ((31 + (b)) - (e))) >> (b)))
// please optimize this
inline void rlwinm_e(Instruction instr, IRFunc* func)
{
    uint32_t mask = (instr.ops[3] <= instr.ops[4]) ? (DMASK(instr.ops[3], instr.ops[4])) : (DMASK(0, instr.ops[4]) | DMASK(3, 31));

    uint32_t width = 32 - instr.ops[2];
    llvm::Value* lhs = BUILD->CreateShl(gprVal(instr.ops[1]), instr.ops[2], "lhs");
    llvm::Value* rhs = BUILD->CreateLShr(gprVal(instr.ops[1]), width, "rhs");
    llvm::Value* rotl = BUILD->CreateOr(lhs, rhs, "rotl");
    
    auto masked = trcTo32(BUILD->CreateAnd(rotl, i64Const(mask), "and"));
    BUILD->CreateStore(zExt64(masked), func->getRegister("RR", instr.ops[0]));
    
    // RC
    if(strcmp(instr.opcName.c_str(), "rlwinmRC") == 0)
    {
        
        llvm::Value* LT = zExt32(BUILD->CreateICmpSLT(masked, i32Const(0), "lt"));
        llvm::Value* GT = zExt32(BUILD->CreateICmpSGT(masked, i32Const(0), "gt"));
        llvm::Value* EQ = zExt32(BUILD->CreateICmpEQ(masked, i32Const(0), "eq"));
        // TODO
        llvm::Value* SO_bit = i32Const(0);
        llvm::Value* field = zExt32(BUILD->CreateOr(BUILD->CreateOr(BUILD->CreateOr(LT, BUILD->CreateShl(GT, 1, "sh"), "or"), BUILD->CreateShl(EQ, 2, "sh"), "or"), BUILD->CreateShl(SO_bit, 3, "sh"), "or"));

        setCRField(func, 0, field);
    }
}

#define BMSK(w, i) (((u64)(1)) << ((w) - (i) - (1)))

#define BGET(dw, w, i) (((dw) & BMSK(w, i)) ? 1 : 0)
#define QMASK(b, e) ((0xFFFFFFFFFFFFFFFF << ((63 + (b)) - (e))) >> (b))


// fix this shis
inline void srawi_e(Instruction instr, IRFunc* func)
{
    BUILD->CreateStore(gprVal(instr.ops[0]), func->getRegister("RR", 31));

    // Extract the low 32 bits of rS as X.
    llvm::Value* X = BUILD->CreateTrunc(gprVal(instr.ops[1]), BUILD->getInt32Ty(), "X");

    // Compute the sign bit S = (X >> 31) & 1.
    llvm::Value* S = BUILD->CreateAShr(X, BUILD->getInt32(31), "S"); // arithmetic shift
    S = BUILD->CreateTrunc(S, BUILD->getInt1Ty(), "S_bit");    // now S is an i1

    // Let n = SH.
    // Compute (32 - n). Note: We must ensure n is in range; assume SH is 0..31.
    llvm::Value* const32 = BUILD->getInt32(32);
    llvm::Value* n32 = i32Const(instr.ops[2]);
    llvm::Value* rotAmount = BUILD->CreateSub(const32, n32, "rotAmount"); // rotAmount = 32 - n

    // Compute the 32-bit rotate left on X by rotAmount.
    // A rotate left by k bits is: (X << k) | (X >> (32 - k)).
    // Here, we want: r = ROTL32(X, 32 - n) which is equivalent to a right shift by n bits,
    // but we compute it via a rotate.
    llvm::Value* shl_part = BUILD->CreateShl(X, rotAmount, "shl_part");
    llvm::Value* lshr_part = BUILD->CreateLShr(X, n32, "lshr_part");
    llvm::Value* r32 = BUILD->CreateOr(shl_part, lshr_part, "rotated32");

    // Extend the rotated result to 64 bits (zero-extend).
    llvm::Value* r = BUILD->CreateZExt(r32, BUILD->getInt64Ty(), "r64");

    // Now, create the 64-bit mask m = MASK(n + 32, 63).
    // In a 64-bit value, bits 32..63 correspond to the low-order 32 bits.
    // We want a mask with ones from bit (n+32) to bit 63.
    // Let k = 32 - n. Then number of ones = k.
    // Compute: m = ((1 << (32 - n)) - 1) << (n + 32).
    llvm::Value* k = BUILD->CreateSub(const32, n32, "k"); // k = 32 - n
    // Compute (1 << k)
    llvm::Value* one64 = BUILD->getInt64(1);
    llvm::Value* shifted = BUILD->CreateShl(one64, zExt64(k), "one_shl_k");
    // Compute (1 << k) - 1
    llvm::Value* maskPart = BUILD->CreateSub(shifted, BUILD->getInt64(1), "maskPart");
    // Now shift left by (n + 32)
    llvm::Value* nPlus32 = BUILD->CreateAdd(zExt64(n32), BUILD->getInt64(32), "nPlus32");
    llvm::Value* m_mask = BUILD->CreateShl(maskPart, nPlus32, "m_mask");

    // Now compute the sign-extension part.
    // The documentation says: (64)S is S replicated to fill the high 32 bits.
    // If S is true (i.e. 1), then (64)S = 0xFFFFFFFF00000000; if false, it's 0.
    // We can compute this with a select.
    llvm::Value* highConst = BUILD->getInt64(0xFFFFFFFF00000000ULL);
    llvm::Value* zero64 = BUILD->getInt64(0);
    llvm::Value* signExt = BUILD->CreateSelect(S, highConst, zero64, "signExt");

    // Finally, compute rA = (r & m_mask) | (signExt & (~m_mask))
    llvm::Value* notMask = BUILD->CreateNot(m_mask, "notMask");
    llvm::Value* part1 = BUILD->CreateAnd(r, m_mask, "part1");
    llvm::Value* part2 = BUILD->CreateAnd(signExt, notMask, "part2");
    llvm::Value* rA = BUILD->CreateOr(part1, part2, "rA");

    BUILD->CreateStore(rA, func->getRegister("RR", instr.ops[0]));

    BUILD->CreateStore(gprVal(instr.ops[0]), func->getRegister("RR", 30));

}


//
//// bitwise operators
//

inline void orx_e(Instruction instr, IRFunc* func)
{
    // The contents of rS are ORed with the contents of rB and the result is placed into rA.
    llvm::Value* value = BUILD->CreateOr(gprVal(instr.ops[1]), gprVal(instr.ops[2]), "or");
    BUILD->CreateStore(value, func->getRegister("RR", instr.ops[0]));
}

inline void ori_e(Instruction instr, IRFunc* func)
{
    auto im64 = i64Const(instr.ops[2]);
    auto orResult = BUILD->CreateOr(gprVal(instr.ops[1]), im64, "or");
    BUILD->CreateStore(orResult, func->getRegister("RR", instr.ops[0]));
}

inline void and_e(Instruction instr, IRFunc* func)
{
    auto andResult = BUILD->CreateAnd(gprVal(instr.ops[1]), gprVal(instr.ops[2]), "and");
    BUILD->CreateStore(andResult, func->getRegister("RR", instr.ops[0]));
}

inline void xor_e(Instruction instr, IRFunc* func)
{
    auto xorResult = BUILD->CreateXor(gprVal(instr.ops[1]), gprVal(instr.ops[2]), "xor");
    BUILD->CreateStore(xorResult, func->getRegister("RR", instr.ops[0]));
}

inline void neg_e(Instruction instr, IRFunc* func)
{
    llvm::Value* negVal = BUILD->CreateNeg(gprVal(instr.ops[1]), "neg");

    // overflow detection: if (rA == MIN_INT) then OV = 1
    //llvm::Value* minInt = BUILD->getInt64(0x8000000000000000);
    //llvm::Value* isOverflow = BUILD->CreateICmpEQ(rA, minInt, "overflow_check");
    //llvm::Value* OV = BUILD->CreateSelect(OE, isOverflow, Builder.getInt1(false), "OV_flag");
    BUILD->CreateStore(negVal, func->getRegister("RR", instr.ops[0]));
}

//
//// MATH
//

inline void mullw_e(Instruction instr, IRFunc* func)
{
    auto mulResult = trcTo32(BUILD->CreateMul(gprVal(instr.ops[1]), gprVal(instr.ops[2]), "Mul"));
    BUILD->CreateStore(mulResult, func->getRegister("RR", instr.ops[0]));
}