#include <unordered_map>
#include "Codec/ppc_codec.h"

enum BinaryType : uint8_t {
    BIN_XEX,
    BIN_PE,
    BIN_KERNEL,
    BIN_UNKNOWN
};

class PPCModule {
public:
    std::string mImagePath;
    BinaryType m_type;
    uint32_t mID;
    std::unordered_map<uint32_t, codec::DecodedInst> mInstrMap; // address -> instruction
    
    PPCModule(std::string path, bool useCache = false, bool isKernel = false);
private:
    void LoadBinary();
    void RecompileBinary();
};