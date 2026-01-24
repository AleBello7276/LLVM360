#include "PPCModule.h"

#include "Logger.h"
#include "Loader/ImageLoader.h"
#include <Loader/XEXImage.h>
#include <Loader/PEImage.h>

PPCModule::PPCModule(std::string path, bool useCache, bool isKernel) {
    mImagePath = path;
    m_type = isKernel ? BIN_KERNEL : BIN_UNKNOWN;
    mID = -1;


    if(useCache == false) {
        // load and decode instructions
        LoadBinary();

        // recompile into dym lib
        RecompileBinary();
    }

    LOG_ERROR("TranslateBinary {}", "Caching NYI");

    // the cache is very simple in practice, it's just a way to store already recompiled modules, 
    // it doesn't matter where they are located 
    // so all cached binaries will be located in ./cache/<hash>
}

void PPCModule::LoadBinary() {
    auto bin = XLoader::ImageLoader::load(mImagePath);
    if(bin == nullptr) {
        LOG_ERROR("PBinaryHandle::LoadBinary -> Failed to load binary image");
        return;
    }
    if(m_type == BIN_UNKNOWN) {
        if(dynamic_cast<XLoader::XEXImage*>(bin.get()) != nullptr) {
            m_type = BIN_XEX;
        }
        else if(dynamic_cast<XLoader::PEImage*>(bin.get()) != nullptr) {
            m_type = BIN_PE;
        }
        else {
            LOG_ERROR("PBinaryHandle::LoadBinary -> Unknown binary type");
            return;
        }
    }


    for(const auto& sec : bin->getSections()) {

        if(sec->getName() != ".text") {
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


        LOG_DEBUG("PBinaryHandle::LoadBinary Found executable section: {}", sec->getName().c_str());


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

void PPCModule::RecompileBinary() {

}
