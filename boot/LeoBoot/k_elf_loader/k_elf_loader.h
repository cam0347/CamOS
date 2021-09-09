#include "../types.h"
#define ELF_HSIZE_32 52
#define ELF_HSIZE_64 64
#define CR_TEXT 0
#define CR_DATA 1
#define CR_RODATA 2
#define CR_BSS 4

//section load addresses (set by the parser)
const void *kmin_address = (void *) 0x28600;
void *ktext_address = (void *) 0;
void *kdata_address = (void *) 0;
void *krodata_address = (void *) 0;
void *kbss_address = (void *) 0;

enum elf_type {
    none =   0x0000,
    rel =    0x0001,
    exec =   0x0002,
    dyn =    0x0003,
    core =   0x0004,
    loos =   0xFE00,
    hios =   0xFEFF,
    loproc = 0xFF00,
    hiproc = 0xFFFF
};

struct elf_header_32 {
    uint8_t e_ident[16];  //some other things
    uint16_t e_type;      //type of elf file
    uint16_t e_machine;   //machine type
    uint32_t e_version;   //elf version
    uint32_t e_entry;     //entry offset
    uint32_t e_phoff;     //program header offset
    uint32_t e_shoff;     //section header offset
    uint32_t e_flags;     //flags
    uint16_t e_hsize;     //header size
    uint16_t e_phentsize; //program header entry size
    uint16_t e_phnum;     //program header entry number
    uint16_t e_shentsize; //section header entry size
    uint16_t e_shnum;     //section header entry number
    uint16_t e_shstrndx;  //section header entry that contains sections name
};

struct ph_entry_32 {
    uint32_t type;
    uint32_t file_offset;
    uint32_t virtual_address;
    uint32_t physical_address;
    uint32_t file_size;
    uint32_t mem_size;
    uint32_t flags;
    uint32_t alignment;
};

struct sh_entry_32 {
    uint32_t name_offset;
    uint32_t type;
    uint32_t flags;
    uint32_t load_address;
    uint32_t file_offset;
    uint32_t size;
    uint32_t section_index;
    uint32_t info;
    uint32_t alignment;
    uint32_t entry_size;
};

bool validate_elf(const void *file);
void reverse_endianess(void *ptr, uint32_t length);