//e_ident field indexes in elf header structure
#define EI_BITS       4
#define EI_ENDIANESS  5
#define EI_VERSION    6
#define EI_OSABI      7
#define EI_ABIVERSION 8

//elf header sizes
#define ELF_HSIZE_32 52
#define ELF_HSIZE_64 64

//bounds 2 ~ 21
//disk sectors bound
#define TEXT_MIN_SEC   6
#define TEXT_MAX_SEC   133 //5Kb for code
#define DATA_MIN_SEC   134
#define DATA_MAX_SEC   197 //2.5Kb for data
#define RODATA_MIN_SEC 198
#define RODATA_MAX_SEC 261 //2.5Kb for rodata
//BSS segment is not loaded into the disk, the ram is already prepared to host bss segment

#define BOOTLOADER_ST2_LBA  2
#define BOOTLOADER_ST2_SIZE 4

#define TEXT 0
#define DATA 1
#define RODATA 2
#define BSS 3
#define SYMTAB 4
#define STRTAB 5

enum sh_type {
    null = 0x00,
    progbits = 0x01,
    symtab = 0x02,
    strtab = 0x03,
    rela = 0x04,
    hash = 0x05,
    dynamic = 0x06,
    note = 0x07,
    nobits = 0x08,
    rel = 0x09,
    shlib = 0x0A,
    dynsym = 0x0B,
    init_array = 0x0E,
    fini_array = 0x0F,
    preinit_array = 0x10,
    group = 0x11,
    symtab_shndx = 0x12,
    num = 0x13,
    loos = 0x60000000
};

//ELF header 32 bits
struct elf_header_32 {
    unsigned char e_ident[16];  //some other things
    unsigned short e_type;      //type of elf file
    unsigned short e_machine;   //machine type
    unsigned int e_version;     //elf version
    unsigned int e_entry;       //entry offset
    unsigned int e_phoff;       //program header offset
    unsigned int e_shoff;       //section header offset
    unsigned int e_flags;       //flags
    unsigned short e_hsize;     //header size
    unsigned short e_phentsize; //program header entry size
    unsigned short e_phnum;     //program header entry number
    unsigned short e_shentsize; //section header entry size
    unsigned short e_shnum;     //section header entry number
    unsigned short e_shstrndx;  //section header entry that contains sections name
};

//ELF header 64 bits
struct elf_header_64 {
    unsigned char e_ident[16];  //some other things
    unsigned short e_type;      //type of elf file
    unsigned short e_machine;   //machine type
    unsigned int e_version;     //elf version
    unsigned int e_entry;       //entry offset
    unsigned long e_phoff;      //program header offset
    unsigned long e_shoff;      //section header offset
    unsigned long e_flags;      //flags
    unsigned short e_hsize;     //header size
    unsigned short e_phentsize; //program header entry size
    unsigned short e_phnum;     //program header entry number
    unsigned short e_shentsize; //section header entry size
    unsigned short e_shnum;     //section header entry number
    unsigned short e_shstrndx;  //section header entry that contains sections name
};

//program entry 32 bits
struct ph_entry_32 {
    unsigned int type;
    unsigned int file_offset;
    unsigned int virtual_address;
    unsigned int physical_address;
    unsigned int file_size;
    unsigned int mem_size;
    unsigned int flags;
    unsigned int alignment;
};

//program header entry 64 bits
struct ph_entry_64 {
    unsigned int type;
    unsigned int flags;
    unsigned long file_offset;
    unsigned long virtual_address;
    unsigned long physical_address;
    unsigned long file_size;
    unsigned long mem_size;
    unsigned long alignment;
};

//section header entry 32 bits
struct sh_entry_32 {
    unsigned int name_offset;
    unsigned int type;
    unsigned int flags;
    unsigned int load_address;
    unsigned int file_offset;
    unsigned int size;
    unsigned int section_index;
    unsigned int info;
    unsigned int alignment;
    unsigned int entry_size;
};

//section header entry 64 bits
struct sh_entry_64 {
    unsigned int name_offset;
    unsigned int type;
    unsigned long flags;
    unsigned long load_address;
    unsigned long file_offset;
    unsigned long size;
    unsigned int section_index;
    unsigned int info;
    unsigned long alignment;
    unsigned long entry_size;
};

struct elf_reloc {
    unsigned int ptr;
    unsigned short info;
};

struct symtab_entry {
    unsigned int name_index;
    unsigned int value;
    unsigned int size;
    unsigned char info;
    unsigned char other;
    unsigned short shndx;
};