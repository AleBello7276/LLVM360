#pragma once 
#include <vector>
#include <string>
#include <stdint.h>
#include "Codec/ppc_codec.h"

#ifdef NAIVE_EXPORT
#define NAIVE_EXPORT __declspec(dllexport)
#else
#define NAIVE_EXPORT __declspec(dllimport)
#endif


enum BinaryType
{
	BIN_XEX,
	BIN_PE,
	BIN_KERNEL,
	BIN_UNKNOWN
};

// Precompiled Binary Handle
// it's used to as communication between Naive+ and emulator to define a loaded binary
struct PBinaryHandle
{
	std::wstring m_imagePath;
	BinaryType m_type;
	uint32_t m_ID;
	std::unordered_map<uint32_t, codec::DecodedInst> m_binInstr;

	void LoadBinary();
	void RecompileBinary();
};



	// Translate a binary located at <path> and return a generated handle describing it
	// used also to load cached binaries <useCache> (true)
	// the PBinaryHandle is generated everytime the binary it's translated or reloaded from cache
	//NAIVE_EXPORT PBinaryHandle* TranslateBinary(std::wstring path, bool useCache = false, bool isKernel = false);
