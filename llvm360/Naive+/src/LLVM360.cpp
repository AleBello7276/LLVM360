#include "Shared.h"
#include "Logger.h"
#include "Loader/ImageLoader.h"
#include <Loader/XEXImage.h>
#include <Loader/PEImage.h>
#include "IR/IRGen.h"

void PBinaryHandle::RecompileBinary()
{
    //IRGen gen = IRGen(this->m_binInstr);
}

// loads in PBinaryHandle instructions and other stuff
void PBinaryHandle::LoadBinary()
{
    auto bin = XLoader::ImageLoader::load(this->m_imagePath);
    if (bin == nullptr)
    {
        LOG_ERROR("PBinaryHandle::LoadBinary", "Failed to load binary image");
        return;
    }
    if (this->m_type == BIN_UNKNOWN) {
        if (dynamic_cast<XLoader::XEXImage*>(bin.get()) != nullptr){
            this->m_type = BIN_XEX;
        }
        else if (dynamic_cast<XLoader::PEImage*>(bin.get()) != nullptr){
            this->m_type = BIN_PE;
        }
        else{
            LOG_ERROR("PBinaryHandle::LoadBinary", "Unknown binary type");
            return;
        }
    }


    for(const auto& sec : bin->getSections())
    {
        
        if (sec->getName() != ".text")
        {
            continue;
        }


        //uint32_t secVirtBase = 0;
        //uint32_t secVirtSize = 0;
        //// relocation for kernel, idk why it's offsetted
        //if (this->m_type == BIN_KERNEL)
        //{
        //    secVirtBase = 0x80065c00; // .text real base
        //    secVirtSize = 0x10A400; // .text real size
        //}

        
	    LOG_DEBUG("PBinaryHandle::LoadBinary", "Found executable section: %s", sec->getName().c_str());
        

        //uint32_t virtualAddr = sec->getVirtualAddress();
        //uint32_t virtualSize = sec->getVirtualSize();
        //
        //
        //const auto base = bin->getBaseAddress();
        //const auto start = base + virtualAddr;
        //const auto end = base + virtualAddr + virtualSize;
        //
        //
        //
        //const uint8_t* secDataPtr = (const uint8_t*)bin->getMemoryData() + (virtualAddr);
	    //uint32_t address = start;
        //
        //InstructionRegistry& registry = g_instrRegistry;
        //while (address <= end)
        //{
        //    // get and byteswap
        //    uint32_t data = __bswapd( (uint32_t) * (uint32_t*)(secDataPtr + (address - start)) );
        //    Instruction instruction = registry.DecodeInstr(data, address);
        //    this->m_binInstr.try_emplace(address, instruction);
        //     
        //    address += 4;
        //}
    }
}



PBinaryHandle* TranslateBinary(std::wstring path, bool useCache, bool isKernel)
{
	
	PBinaryHandle* handle = new PBinaryHandle();
	handle->m_imagePath = path;
	handle->m_type = isKernel ? BIN_KERNEL : BIN_UNKNOWN;
	handle->m_ID = -1;
    

	if (useCache == false)
	{
        // load and decode instructions
        handle->LoadBinary();

        // recompile into dym lib
		handle->RecompileBinary();
		
        return handle;
    }

	LOG_ERROR("TranslateBinary", "Caching NYI");
    // the cache is very simple in practice, it's just a way to store already recompiled modules, 
    // it doesn't matter where they are located 
	// so all cached binaries will be located in ./cache/<hash>

	return handle;
}

#include "Codec/ppc_codec.h"

int main(int argc, char* argv[])
{
   

    //loadedXex = new XexImage(L"LLVMTest1.xex");
    //loadedXex->LoadXex();
    //g_irGen = new IRGenerator(loadedXex, mod, &builder);
    //g_irGen->Initialize();
    
    // 394a0001

    const codec::DecodedInst instObj = codec::PPCCodec::decode(0xffff4A39);
    codec::PPCInstrType instrType = instObj.mInstTemplate.type();
    printf("Decoded instruction: %s\n", instObj.mInstTemplate.dump(instObj.mData).c_str());
    //codec::PPCCodec::decode(0x394a0001);
    uint32_t encodedAddi = codec::Addi::encode(10, 10, 1);

    // Splash texts
    printf("Hello, World!\n");
    printf("Say hi to the new galaxy note\n");

    // 19/02/2025
	printf("Trans rights!!!  @permdog99\n");
    printf("Live and learn  @ashrindy\n");
    printf("On dog  @.nover.\n");
	printf("Gotta Go Fast  @neoslyde\n");
}


