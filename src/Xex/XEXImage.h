#pragma once
#include <cstdint>
#include <stdio.h>
#include <vcruntime_string.h>
#include <type_traits>
#include <vector>

//-----------------------------------------------------------------------------

#pragma pack(push)
#pragma pack(1)

struct DOSHeader {
  char signature[2]; // MZ
  short lastsize;
  short nblocks;
  short nreloc;
  short hdrsize;
  short minalloc;
  short maxalloc;
  uint16_t ss;
  uint16_t sp;
  short checksum;
  uint16_t ip;
  uint16_t cs;
  short relocpos;
  short noverlay;
  short reserved1[4];
  short oem_id;
  short oem_info;
  short reserved2[10];
  long e_lfanew;

  inline bool Validate() const {
    if (signature[0] != 'M' || signature[1] != 'Z') {
      printf("PE: Invalid file signature, should be MZ, found '%c%c'\n",
        signature[0] > 32 ? signature[0] : ' ',
        signature[1] > 32 ? signature[1] : ' ');
      return false;
    }

    return true;
  }
};

#pragma pack(pop)

struct COFFHeader {
  short Machine;
  short NumberOfSections;
  long TimeDateStamp;
  long PointerToSymbolTable;
  long NumberOfSymbols;
  short SizeOfOptionalHeader;
  short Characteristics;
};

struct COFFSection {
  char Name[8];
  uint32_t VirtualSize;
  uint32_t VirtualAddress;
  uint32_t SizeOfRawData;
  uint32_t PointerToRawData;
  uint32_t PointerToRelocations;
  uint32_t PointerToLinenumbers;
  uint16_t NumberOfRelocations;
  uint16_t NumberOfLinenumbers;
  uint32_t Flags;
};

struct PEDataDirectory {
  long VirtualAddress;
  long Size;
};

struct PEOptHeader {
  short signature; // decimal number 267.
  char MajorLinkerVersion;
  char MinorLinkerVersion;
  long SizeOfCode;
  long SizeOfInitializedData;
  long SizeOfUninitializedData;
  long AddressOfEntryPoint; // The RVA of the code entry point
  long BaseOfCode;
  long BaseOfData;
  long ImageBase;
  long SectionAlignment;
  long FileAlignment;
  short MajorOSVersion;
  short MinorOSVersion;
  short MajorImageVersion;
  short MinorImageVersion;
  short MajorSubsystemVersion;
  short MinorSubsystemVersion;
  long Reserved;
  long SizeOfImage;
  long SizeOfHeaders;
  long Checksum;
  short Subsystem;
  short DLLCharacteristics;
  long SizeOfStackReserve;
  long SizeOfStackCommit;
  long SizeOfHeapReserve;
  long SizeOfHeapCommit;
  long LoaderFlags;
  long NumberOfRvaAndSizes;
  PEDataDirectory DataDirectory[16]; // Can have any number of elements, matching the number in
                                     // NumberOfRvaAndSizes.

  bool Validate() const {
    if (signature != 267) {
      printf("Invalid optional PE signature, should be 267, found %d\n", signature);
      return false;
    }

    return true;
  }
};

//-----------------------------------------------------------------------------

class ImageByteReaderXEX {
private:
  const uint8_t *m_data;
  uint32_t m_size;
  uint32_t m_offset;

public:
  ImageByteReaderXEX(const uint8_t *data, const uint32_t size)
    : m_data(data)
    , m_size(size)
    , m_offset(0) {
  }

  inline bool Read(void *dest, uint32_t size) {
    if (m_offset + size > m_size) {
      printf("IO: Reading outside file size (offset: %d, size:%d, file size: %d)\n",
        m_offset,
        size,
        m_size);
      return false;
    }

    memcpy(dest, m_data + m_offset, size);
    m_offset += size;
    return true;
  }

  inline bool Seek(const uint32_t offset) {
    if (offset >= m_size) {
      printf("IO: Seeking to position outside file size (offset: %d, file size: %d)\n",
        offset,
        m_size);
      return false;
    }

    m_offset = offset;
    return true;
  }

  inline const uint8_t *GetData() const {
    return m_data;
  }

  inline const uint32_t GetSize() const {
    return m_size;
  }

  inline const uint32_t GetOffset() const {
    return m_offset;
  }
};

//-----------------------------------------------------------------------------

inline static void Swap64(void *ptr) {
  uint8_t *data = (uint8_t *)ptr;
  std::swap(data[0], data[7]);
  std::swap(data[1], data[6]);
  std::swap(data[2], data[5]);
  std::swap(data[3], data[4]);
}

inline static void Swap32(void *ptr) {
  uint8_t *data = (uint8_t *)ptr;
  std::swap(data[0], data[3]);
  std::swap(data[1], data[2]);
}

inline static void Swap16(void *ptr) {
  uint8_t *data = (uint8_t *)ptr;
  std::swap(data[0], data[1]);
}

//-----------------------------------------------------------------------------

#define XEX2_SECTION_LENGTH 0x00010000

//-----------------------------------------------------------------------------

enum XEXHeaderKeys {
  XEX_HEADER_RESOURCE_INFO = 0x000002FF,
  XEX_HEADER_FILE_FORMAT_INFO = 0x000003FF,
  XEX_HEADER_DELTA_PATCH_DESCRIPTOR = 0x000005FF,
  XEX_HEADER_BASE_REFERENCE = 0x00000405,
  XEX_HEADER_BOUNDING_PATH = 0x000080FF,
  XEX_HEADER_DEVICE_ID = 0x00008105,
  XEX_HEADER_ORIGINAL_BASE_ADDRESS = 0x00010001,
  XEX_HEADER_ENTRY_POINT = 0x00010100,
  XEX_HEADER_IMAGE_BASE_ADDRESS = 0x00010201,
  XEX_HEADER_IMPORT_LIBRARIES = 0x000103FF,
  XEX_HEADER_CHECKSUM_TIMESTAMP = 0x00018002,
  XEX_HEADER_ENABLED_FOR_CALLCAP = 0x00018102,
  XEX_HEADER_ENABLED_FOR_FASTCAP = 0x00018200,
  XEX_HEADER_ORIGINAL_PE_NAME = 0x000183FF,
  XEX_HEADER_STATIC_LIBRARIES = 0x000200FF,
  XEX_HEADER_TLS_INFO = 0x00020104,
  XEX_HEADER_DEFAULT_STACK_SIZE = 0x00020200,
  XEX_HEADER_DEFAULT_FILESYSTEM_CACHE_SIZE = 0x00020301,
  XEX_HEADER_DEFAULT_HEAP_SIZE = 0x00020401,
  XEX_HEADER_PAGE_HEAP_SIZE_AND_FLAGS = 0x00028002,
  XEX_HEADER_SYSTEM_FLAGS = 0x00030000,
  XEX_HEADER_EXECUTION_INFO = 0x00040006,
  XEX_HEADER_TITLE_WORKSPACE_SIZE = 0x00040201,
  XEX_HEADER_GAME_RATINGS = 0x00040310,
  XEX_HEADER_LAN_KEY = 0x00040404,
  XEX_HEADER_XBOX360_LOGO = 0x000405FF,
  XEX_HEADER_MULTIDISC_MEDIA_IDS = 0x000406FF,
  XEX_HEADER_ALTERNATE_TITLE_IDS = 0x000407FF,
  XEX_HEADER_ADDITIONAL_TITLE_MEMORY = 0x00040801,
  XEX_HEADER_EXPORTS_BY_NAME = 0x00E10402,
};

enum XEXModuleFlags {
  XEX_MODULE_TITLE = 0x00000001,
  XEX_MODULE_EXPORTS_TO_TITLE = 0x00000002,
  XEX_MODULE_SYSTEM_DEBUGGER = 0x00000004,
  XEX_MODULE_DLL_MODULE = 0x00000008,
  XEX_MODULE_MODULE_PATCH = 0x00000010,
  XEX_MODULE_PATCH_FULL = 0x00000020,
  XEX_MODULE_PATCH_DELTA = 0x00000040,
  XEX_MODULE_USER_MODE = 0x00000080,
};

enum XEXSystemFlags {
  XEX_SYSTEM_NO_FORCED_REBOOT = 0x00000001,
  XEX_SYSTEM_FOREGROUND_TASKS = 0x00000002,
  XEX_SYSTEM_NO_ODD_MAPPING = 0x00000004,
  XEX_SYSTEM_HANDLE_MCE_INPUT = 0x00000008,
  XEX_SYSTEM_RESTRICTED_HUD_FEATURES = 0x00000010,
  XEX_SYSTEM_HANDLE_GAMEPAD_DISCONNECT = 0x00000020,
  XEX_SYSTEM_INSECURE_SOCKETS = 0x00000040,
  XEX_SYSTEM_XBOX1_INTEROPERABILITY = 0x00000080,
  XEX_SYSTEM_DASH_CONTEXT = 0x00000100,
  XEX_SYSTEM_USES_GAME_VOICE_CHANNEL = 0x00000200,
  XEX_SYSTEM_PAL50_INCOMPATIBLE = 0x00000400,
  XEX_SYSTEM_INSECURE_UTILITY_DRIVE = 0x00000800,
  XEX_SYSTEM_XAM_HOOKS = 0x00001000,
  XEX_SYSTEM_ACCESS_PII = 0x00002000,
  XEX_SYSTEM_CROSS_PLATFORM_SYSTEM_LINK = 0x00004000,
  XEX_SYSTEM_MULTIDISC_SWAP = 0x00008000,
  XEX_SYSTEM_MULTIDISC_INSECURE_MEDIA = 0x00010000,
  XEX_SYSTEM_AP25_MEDIA = 0x00020000,
  XEX_SYSTEM_NO_CONFIRM_EXIT = 0x00040000,
  XEX_SYSTEM_ALLOW_BACKGROUND_DOWNLOAD = 0x00080000,
  XEX_SYSTEM_CREATE_PERSISTABLE_RAMDRIVE = 0x00100000,
  XEX_SYSTEM_INHERIT_PERSISTENT_RAMDRIVE = 0x00200000,
  XEX_SYSTEM_ALLOW_HUD_VIBRATION = 0x00400000,
  XEX_SYSTEM_ACCESS_UTILITY_PARTITIONS = 0x00800000,
  XEX_SYSTEM_IPTV_INPUT_SUPPORTED = 0x01000000,
  XEX_SYSTEM_PREFER_BIG_BUTTON_INPUT = 0x02000000,
  XEX_SYSTEM_ALLOW_EXTENDED_SYSTEM_RESERVATION = 0x04000000,
  XEX_SYSTEM_MULTIDISC_CROSS_TITLE = 0x08000000,
  XEX_SYSTEM_INSTALL_INCOMPATIBLE = 0x10000000,
  XEX_SYSTEM_ALLOW_AVATAR_GET_METADATA_BY_XUID = 0x20000000,
  XEX_SYSTEM_ALLOW_CONTROLLER_SWAPPING = 0x40000000,
  XEX_SYSTEM_DASH_EXTENSIBILITY_MODULE = 0x80000000,
  // TODO: figure out how stored
  /*XEX_SYSTEM_ALLOW_NETWORK_READ_CANCEL            = 0x0,
  XEX_SYSTEM_UNINTERRUPTABLE_READS                = 0x0,
  XEX_SYSTEM_REQUIRE_FULL_EXPERIENCE              = 0x0,
  XEX_SYSTEM_GAME_VOICE_REQUIRED_UI               = 0x0,
  XEX_SYSTEM_CAMERA_ANGLE                         = 0x0,
  XEX_SYSTEM_SKELETAL_TRACKING_REQUIRED           = 0x0,
  XEX_SYSTEM_SKELETAL_TRACKING_SUPPORTED          = 0x0,*/
};

enum XEXAprovalType {
  XEX_APPROVAL_UNAPPROVED = 0,
  XEX_APPROVAL_POSSIBLE = 1,
  XEX_APPROVAL_APPROVED = 2,
  XEX_APPROVAL_EXPIRED = 3,
};

enum XEXEncryptionType {
  XEX_ENCRYPTION_NONE = 0,
  XEX_ENCRYPTION_NORMAL = 1,
};

enum XEXCompressionType {
  XEX_COMPRESSION_NONE = 0,
  XEX_COMPRESSION_BASIC = 1,
  XEX_COMPRESSION_NORMAL = 2,
  XEX_COMPRESSION_DELTA = 3,
};

enum XEXImageFlags {
  XEX_IMAGE_MANUFACTURING_UTILITY = 0x00000002,
  XEX_IMAGE_MANUFACTURING_SUPPORT_TOOLS = 0x00000004,
  XEX_IMAGE_XGD2_MEDIA_ONLY = 0x00000008,
  XEX_IMAGE_CARDEA_KEY = 0x00000100,
  XEX_IMAGE_XEIKA_KEY = 0x00000200,
  XEX_IMAGE_USERMODE_TITLE = 0x00000400,
  XEX_IMAGE_USERMODE_SYSTEM = 0x00000800,
  XEX_IMAGE_ORANGE0 = 0x00001000,
  XEX_IMAGE_ORANGE1 = 0x00002000,
  XEX_IMAGE_ORANGE2 = 0x00004000,
  XEX_IMAGE_IPTV_SIGNUP_APPLICATION = 0x00010000,
  XEX_IMAGE_IPTV_TITLE_APPLICATION = 0x00020000,
  XEX_IMAGE_KEYVAULT_PRIVILEGES_REQUIRED = 0x04000000,
  XEX_IMAGE_ONLINE_ACTIVATION_REQUIRED = 0x08000000,
  XEX_IMAGE_PAGE_SIZE_4KB = 0x10000000, // else 64KB
  XEX_IMAGE_REGION_FREE = 0x20000000,
  XEX_IMAGE_REVOCATION_CHECK_OPTIONAL = 0x40000000,
  XEX_IMAGE_REVOCATION_CHECK_REQUIRED = 0x80000000,
};

enum XEXMediaFlags {
  XEX_MEDIA_HARDDISK = 0x00000001,
  XEX_MEDIA_DVD_X2 = 0x00000002,
  XEX_MEDIA_DVD_CD = 0x00000004,
  XEX_MEDIA_DVD_5 = 0x00000008,
  XEX_MEDIA_DVD_9 = 0x00000010,
  XEX_MEDIA_SYSTEM_FLASH = 0x00000020,
  XEX_MEDIA_MEMORY_UNIT = 0x00000080,
  XEX_MEDIA_USB_MASS_STORAGE_DEVICE = 0x00000100,
  XEX_MEDIA_NETWORK = 0x00000200,
  XEX_MEDIA_DIRECT_FROM_MEMORY = 0x00000400,
  XEX_MEDIA_RAM_DRIVE = 0x00000800,
  XEX_MEDIA_SVOD = 0x00001000,
  XEX_MEDIA_INSECURE_PACKAGE = 0x01000000,
  XEX_MEDIA_SAVEGAME_PACKAGE = 0x02000000,
  XEX_MEDIA_LOCALLY_SIGNED_PACKAGE = 0x04000000,
  XEX_MEDIA_LIVE_SIGNED_PACKAGE = 0x08000000,
  XEX_MEDIA_XBOX_PACKAGE = 0x10000000,
};

enum XEXRegion {
  XEX_REGION_NTSCU = 0x000000FF,
  XEX_REGION_NTSCJ = 0x0000FF00,
  XEX_REGION_NTSCJ_JAPAN = 0x00000100,
  XEX_REGION_NTSCJ_CHINA = 0x00000200,
  XEX_REGION_PAL = 0x00FF0000,
  XEX_REGION_PAL_AU_NZ = 0x00010000,
  XEX_REGION_OTHER = 0xFF000000,
  XEX_REGION_ALL = 0xFFFFFFFF,
};

enum XEXSectionType {
  XEX_SECTION_CODE = 1,
  XEX_SECTION_DATA = 2,
  XEX_SECTION_READONLY_DATA = 3,
};

//-----------------------------------------------------------------------------

struct XEXRatings {
  uint8_t rating_esrb;
  uint8_t rating_pegi;
  uint8_t rating_pegifi;
  uint8_t rating_pegipt;
  uint8_t rating_bbfc;
  uint8_t rating_cero;
  uint8_t rating_usk;
  uint8_t rating_oflcau;
  uint8_t rating_oflcnz;
  uint8_t rating_kmrb;
  uint8_t rating_brazil;
  uint8_t rating_fpb;
};

union XEXVersion {
  uint32_t value;

  struct {
    uint32_t major : 4;
    uint32_t minor : 4;
    uint32_t build : 16;
    uint32_t qfe : 8;
  };
};

struct XEXOptionalHeader {
  uint32_t offset;
  uint32_t length;
  uint32_t value;
  uint32_t key;

  bool Read(ImageByteReaderXEX &reader) {
    if (!reader.Read(&key, sizeof(key)))
      return false;
    if (!reader.Read(&offset, sizeof(offset)))
      return false;
    Swap32(&key);
    Swap32(&offset);
    length = 0;
    value = 0;
    return true;
  }
};

struct XEXResourceInfo {
  char name[9];
  uint32_t address;
  uint32_t size;

  bool Read(ImageByteReaderXEX &reader) {
    if (!reader.Read(&name, 8))
      return false;
    if (!reader.Read(&address, 4))
      return false;
    if (!reader.Read(&size, 4))
      return false;
    Swap32(&address);
    Swap32(&size);
    return true;
  }
};

struct XEXExecutionInfo {
  uint32_t media_id;
  XEXVersion version;
  XEXVersion base_version;
  uint32_t title_id;
  uint8_t platform;
  uint8_t executable_table;
  uint8_t disc_number;
  uint8_t disc_count;
  uint32_t savegame_id;

  bool Read(ImageByteReaderXEX &reader) {
    if (!reader.Read(this, sizeof(XEXExecutionInfo)))
      return false;
    Swap32(&media_id);
    Swap32(&version.value);
    Swap32(&base_version.value);
    Swap32(&title_id);
    Swap32(&savegame_id);
    return true;
  }
};

struct XEXTLSInfo {
  uint32_t slot_count;
  uint32_t raw_data_address;
  uint32_t data_size;
  uint32_t raw_data_size;

  bool Read(ImageByteReaderXEX &reader) {
    if (!reader.Read(this, sizeof(XEXTLSInfo)))
      return false;
    Swap32(&slot_count);
    Swap32(&raw_data_address);
    Swap32(&data_size);
    Swap32(&raw_data_size);
    return true;
  }
};

struct XEXImportLibraryBlockHeader {
  uint32_t string_table_size;
  uint32_t count;

  bool Read(ImageByteReaderXEX &reader) {
    if (!reader.Read(this, sizeof(XEXImportLibraryBlockHeader)))
      return false;
    Swap32(&count);
    Swap32(&string_table_size);
    return true;
  }
};

#pragma pack(push)
#pragma pack(2)

struct XEXImportLibraryHeader {
  uint32_t unknown;
  uint8_t digest[20];
  uint32_t import_id;
  XEXVersion version;
  XEXVersion min_version;
  uint16_t name_index;
  uint16_t record_count;

  bool Read(ImageByteReaderXEX &reader) {
    if (!reader.Read(this, sizeof(XEXImportLibraryHeader)))
      return false;
    Swap32(&unknown);
    Swap32(&import_id);
    Swap32(&version.value);
    Swap32(&min_version.value);
    Swap16(&name_index);
    Swap16(&record_count);
    return true;
  }
};

#pragma pack(pop)

struct XEXStaticLibrary {
  char name[9]; // 8 + 1 for \0
  uint16_t major;
  uint16_t minor;
  uint16_t build;
  uint16_t qfe;
  XEXAprovalType approval;
};

struct XEXFileBasicCompressionBlock {
  uint32_t data_size;
  uint32_t zero_size;
};

struct XEXFileNormalCompressionInfo {
  uint32_t window_size;
  uint32_t window_bits;
  uint32_t block_size;
  uint8_t block_hash[20];

  bool Read(ImageByteReaderXEX &reader) {
    if (!reader.Read(&window_size, sizeof(window_size)))
      return false;
    if (!reader.Read(&block_size, sizeof(block_size)))
      return false;
    if (!reader.Read(&block_hash, sizeof(block_hash)))
      return false;
    Swap32(&window_size);
    Swap32(&block_size);

    window_bits = 0;
    uint32_t temp = window_size;
    for (uint32_t m = 0; m < 32; m++, window_bits++) {
      temp <<= 1;
      if (temp == 0x80000000) {
        break;
      }
    }

    return true;
  }
};

struct XEXEncryptionHeader {
  uint16_t encryption_type;
  uint16_t compression_type;

  bool Read(ImageByteReaderXEX &reader) {
    if (!reader.Read(this, sizeof(XEXEncryptionHeader)))
      return false;
    Swap16(&compression_type);
    Swap16(&encryption_type);
    return true;
  }
};

struct XEXFileFormat {
  XEXEncryptionType encryption_type;
  XEXCompressionType compression_type;

  // basic compression blocks (in case of basic compression)
  std::vector<XEXFileBasicCompressionBlock> basic_blocks;

  // normal compression
  XEXFileNormalCompressionInfo normal;
};

struct XEXLoaderInfo {
  uint32_t header_size;
  uint32_t image_size;
  uint8_t rsa_signature[256];
  uint32_t unklength;
  uint32_t image_flags;
  uint32_t load_address;
  uint8_t section_digest[20];
  uint32_t import_table_count;
  uint8_t import_table_digest[20];
  uint8_t media_id[16];
  uint8_t file_key[16];
  uint32_t export_table;
  uint8_t header_digest[20];
  uint32_t game_regions;
  uint32_t media_flags;

  bool Read(ImageByteReaderXEX &reader) {
    if (!reader.Read(this, sizeof(XEXLoaderInfo)))
      return false;
    Swap32(&header_size);
    Swap32(&image_size);
    Swap32(&unklength);
    Swap32(&image_flags);
    Swap32(&load_address);
    Swap32(&import_table_count);
    Swap32(&export_table);
    Swap32(&game_regions);
    Swap32(&media_flags);
    return true;
  }
};

struct XEXSection {
  union {
    struct {
      uint32_t type : 4;        // XEXSectionType
      uint32_t page_count : 28; // # of 64kb pages
    };

    uint32_t value; // To make uint8_t_t swapping easier
  } info;

  uint8_t digest[20];
};

struct XEXHeader {
  uint32_t xex2;
  uint32_t module_flags;
  uint32_t exe_offset;
  uint32_t unknown0;
  uint32_t certificate_offset;
  uint32_t header_count;

  void Swap() {
    Swap32(&xex2);
    Swap32(&module_flags);
    Swap32(&exe_offset);
    Swap32(&unknown0);
    Swap32(&certificate_offset);
    Swap32(&header_count);
  }

  bool Validate() const {
    if (xex2 != 0x58455832) {
      printf("Loaded file is not a XEX2 file. Signature is 0x%08X\n", xex2);
      return false;
    }

    return true;
  }
};

struct XEXImageData {
  // header stuff
  XEXHeader header;
  XEXSystemFlags system_flags;
  XEXExecutionInfo execution_info;
  XEXRatings game_ratings;
  XEXTLSInfo tls_info;
  XEXFileFormat file_format_info;
  XEXLoaderInfo loader_info;
  uint8_t session_key[16];

  // executable info
  uint32_t exe_address;
  uint32_t exe_entry_point;
  uint32_t exe_stack_size;
  uint32_t exe_heap_size;

  // embedded resources
  typedef std::vector<XEXResourceInfo> TResourceInfos;
  TResourceInfos resources;

  // file optional headers
  typedef std::vector<XEXOptionalHeader> TOptionalHeaders;
  TOptionalHeaders optional_headers;

  // data sections
  typedef std::vector<XEXSection> TSections;
  TSections sections;

  // import records
  typedef std::vector<uint32_t> TImportRecords;
  TImportRecords import_records;
};

//-----------------------------------------------------------------------------
