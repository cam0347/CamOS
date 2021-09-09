#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "elf_parser.h"
#define ELF_FILE_PATH "bin/bootm.bin"
#define DISK_PATH "disk.iso"
#define COMPILING_LOG_PATH "../etc/compiling_log.log"
#define VERBOSE 0

/*
Some definitions:
The program header tells the system how to create the process image (gives some informations about the program itself)
The section header tells the system the information of the sections of the programs (text, data, bss, ...), where and how locate them in memory
*/

FILE *file, *disk;
long file_size;
int elf_type;
unsigned char elf_ident[16];

void reverse_endianess(void *ptr, unsigned int length);

int main() {
    printf("loading boot manager into disk...\n");
    file = fopen(ELF_FILE_PATH, "r+");
    disk = fopen(DISK_PATH, "r+");

    if (!file) {
        printf("Failed to open file: %s\n", ELF_FILE_PATH);
        return 0;
    }

    if (!disk) {
        printf("Failed to open disk file: %s\n", DISK_PATH);
        return 0;
    }

    struct stat inode;
    if (stat(ELF_FILE_PATH, &inode) != 0) {
        printf("Failed to retrieve file inode\n");
        fclose(file);
        return 0;
    }

    file_size = inode.st_size;

    //read the elf identificator
    rewind(file);
    if (fread(elf_ident, 16, 1, file) != 1) {
        printf("Failed to read ELF identificator\n");
        fclose(file);
        return 0;
    }

    //check the magic number
    unsigned char magic[] = {0x7F, 'E', 'L', 'F'};
    for (int i = 0; i < 4; i++) {
        if (elf_ident[i] != magic[i]) {
            printf("Not an elf file\n");
            fclose(file);
            return 0;
        }
    }

    printf("Bits: ");
    if (elf_ident[4] == 1) {
        elf_type = 0; //32 bit
        printf("32\n");
    } else if (elf_ident[4] == 2) {
        elf_type = 1; //64 bit
        printf("64\n");
    } else {
        printf("unknown\n");
        printf("Failed to determine file type\n");
        fclose(file);
        return 0;
    }

    printf("Endianess: ");
    if ((elf_ident[EI_ENDIANESS]) == 1) {
        printf("little endian\n");
    } else if (elf_ident[EI_ENDIANESS] == 2) {
        printf("big endian\n");
    } else {
        printf("unknown\n");
    }

    rewind(file);

    unsigned int entry_loc, ph_offset, sh_offset, ph_entnum, sh_entnum, ph_entsize, sh_entsize, e_type, sh_name_entry_index, sections_name_area_size;
    void *section_names;

    //read th whole header and store the informations in global variables (out of the if scope)
    if (elf_type == 0) { //32 bit
        struct elf_header_32 header;
        fread(&header, ELF_HSIZE_32, 1, file); //read the header

        //store the informations
        entry_loc = header.e_entry;
        ph_offset = header.e_phoff;
        sh_offset = header.e_shoff;
        ph_entnum = header.e_phnum;
        sh_entnum = header.e_shnum;
        ph_entsize = header.e_phentsize;
        sh_entsize = header.e_shentsize;
        e_type = header.e_type;
        sh_name_entry_index = header.e_shstrndx;

        //read the section names entry
        fseek(file, sh_offset + sh_name_entry_index * sh_entsize, SEEK_SET);
        struct sh_entry_32 name_entry;
        fread(&name_entry, sh_entsize, 1, file); //reads the entry
        fseek(file, name_entry.file_offset, SEEK_SET); //moves to the data area of the section
        section_names = malloc(name_entry.size); //allocates the required amount of memory
        sections_name_area_size = name_entry.size;
    } else {
        struct elf_header_64 header;
        fread(&header, ELF_HSIZE_64, 1, file);

        //store the informations
        entry_loc = header.e_entry;
        ph_offset = header.e_phoff;
        sh_offset = header.e_shoff;
        ph_entnum = header.e_phnum;
        sh_entnum = header.e_shnum;
        ph_entsize = header.e_phentsize;
        sh_entsize = header.e_shentsize;
        e_type = header.e_type;
        sh_name_entry_index = header.e_shstrndx;

        //read the section names entry
        fseek(file, sh_offset + sh_name_entry_index * sh_entsize, SEEK_SET);
        struct sh_entry_32 name_entry;
        fread(&name_entry, sh_entsize, 1, file); //reads the entry
        fseek(file, name_entry.file_offset, SEEK_SET); //moves to the data area of the section
        section_names = malloc(name_entry.size); //allocates the required amount of memory
        sections_name_area_size = name_entry.size;
    }

    if (!section_names) {
        printf("Buffer error\n");
        fclose(file);
        return 0;
    }

    fread(section_names, sections_name_area_size, 1, file); //reads the section names section data

    if (VERBOSE) {
        printf("ELF type: ");
        switch(e_type) {
            case 0x00:
                printf("none\n");
                break;

            case 0x01:
                printf("relocatable\n");
                break;

            case 0x02:
                printf("executable\n");
                break;

            case 0x03:
                printf("dynamic link\n");
                break;

            default:
                printf("unknown\n");
                break;
        }

        printf("ABI: ");
        switch(elf_ident[EI_OSABI]) {
            case 0x00:
                printf("System V\n");
                break;

            case 0x01:
                printf("HP-UX\n");
                break;

            case 0x02:
                printf("NetBSD\n");
                break;

            case 0x03:
                printf("Linux\n");
                break;

            case 0x04:
                printf("GNU Hurd\n");
                break;

            case 0x06:
                printf("Solaris\n");
                break;

            case 0x07:
                printf("AIX\n");
                break;

            case 0x08:
                printf("IRIX\n");
                break;

            case 0x09:
                printf("FreeBSD\n");
                break;

            case 0x0A:
                printf("Tru64\n");
                break;

            case 0x0B:
                printf("Novell Modesto\n");
                break;

            case 0x0C:
                printf("OpenBSD\n");
                break;

            case 0x0D:
                printf("OpenVMS\n");
                break;

            case 0x0E:
                printf("NonStop Kernel\n");
                break;

            case 0x0F:
                printf("AROS\n");
                break;
        
            case 0x10:
                printf("Fenix OS\n");
                break;

            case 0x11:
                printf("CloudABI\n");
                break;

            case 0x12:
                printf("Stratus Technologies OpenVOS\n");
                break;

            default:
                printf("unknown\n");
                break;
        }

        printf("ABI version: %d\n", elf_ident[EI_ABIVERSION]);
        printf("Program header offset: %u\n", ph_offset);
        printf("Section header offset: %u\n", sh_offset);
        printf("Program header entry number: %u\n", ph_entnum);
        printf("Program header entry size: %u\n", ph_entsize);
        printf("Section header entry number: %u\n", sh_entnum);
        printf("Section header entry size: %u\n", sh_entsize);
        printf("Section header names entry index: %u\n", sh_name_entry_index);
    }

    printf("Program entry location: %u\n", entry_loc);

    fseek(file, sh_offset, SEEK_SET); //move to section header
    char critical_sections[4][8] = {".text", ".data", ".rodata", ".bss"};
    unsigned long cr_sections_off[4] = {0, 0, 0, 0};
    unsigned long cr_sections_len[4] = {0, 0, 0, 0};

    if (elf_type == 0) {
        struct sh_entry_32 sh_entries[sh_entnum];
        fread(sh_entries, sh_entsize, sh_entnum, file); //reads the 32-bit entries

        for (int i = 0; i < sh_entnum; i++) {
            for (int j = 0; j < 4; j++) {
                if (strcmp((char *) (section_names + sh_entries[i].name_offset), critical_sections[j]) == 0) {
                    printf("%10s (of: %u la: %u sz: %u)\n", (char *)(section_names + sh_entries[i].name_offset), sh_entries[i].file_offset, sh_entries[i].load_address, sh_entries[i].size);
                    cr_sections_off[j] = sh_entries[i].file_offset;
                    cr_sections_len[j] = sh_entries[i].size;
                }
            }
        }
    } else {
        struct sh_entry_64 sh_entries[sh_entnum];
        fread(sh_entries, sh_entsize, sh_entnum, file);

        for (int i = 0; i < sh_entnum; i++) {
            for (int j = 0; j < 4; j++) {
                if (strcmp((char *) (section_names + sh_entries[i].name_offset), critical_sections[j]) == 0) {
                    printf("%10s (of: %lu la: %lu sz: %lu)\n", (char *)(section_names + sh_entries[i].name_offset), sh_entries[i].file_offset, sh_entries[i].load_address, sh_entries[i].size);
                    cr_sections_off[j] = sh_entries[i].file_offset;
                    cr_sections_len[j] = sh_entries[i].size;
                }
            }
        }
    }

    //check bounds
    if (cr_sections_len[0] > 512 * (TEXT_MAX_SEC - TEXT_MIN_SEC + 1)) {
        printf("Warning: code section exceeds disk bound\n");
        fclose(file);
        fclose(disk);
        return 0;
    }

    if (cr_sections_len[1] > 512 * (DATA_MAX_SEC - DATA_MIN_SEC + 1)) {
        printf("Warning: data section exceeds disk bound\n");
        fclose(file);
        fclose(disk);
        return 0;
    }

    if (cr_sections_len[2] > 512 * (RODATA_MAX_SEC - RODATA_MIN_SEC + 1)) {
        printf("Warning: rodata section exceeds disk bound\n");
        fclose(file);
        fclose(disk);
        return 0;
    }

    void *data;
    
    //text section [0]
    data = calloc(1, cr_sections_len[0]);
    fseek(file, cr_sections_off[0], SEEK_SET);
    fread(data, cr_sections_len[0], 1, file);
    fseek(disk, TEXT_MIN_SEC * 512, SEEK_SET);
    fwrite(data, cr_sections_len[0], 1, disk);

    //data section [1]
    data = realloc(data, cr_sections_len[1]);
    fseek(file, cr_sections_off[1], SEEK_SET);
    fread(data, cr_sections_len[1], 1, file);
    fseek(disk, DATA_MIN_SEC * 512, SEEK_SET);
    fwrite(data, cr_sections_len[1], 1, disk);

    //rodata section [2]
    data = realloc(data, cr_sections_len[2]);
    fseek(file, cr_sections_off[2], SEEK_SET);
    fread(data, cr_sections_len[2], 1, file);
    fseek(disk, RODATA_MIN_SEC * 512, SEEK_SET);
    fwrite(data, cr_sections_len[2], 1, disk);

    //write boot manager entry address into the last 4 bytes of the boot loader st. 2 code
    //invert endianess
    unsigned int be_entry = entry_loc;
    //reverse_endianess(&be_entry, 4);

    //write to the disk the boot manager entry point (when executed, the boot loader st. 2 will jump to that location)
    fseek(disk, 512 * BOOTLOADER_ST2_LBA + 512 * BOOTLOADER_ST2_SIZE - 4, SEEK_SET);
    fwrite(&be_entry, 4, 1, disk);
    
    free(data);
    fclose(file);
    fclose(disk);

    printf("done\n");

    return 0;
}

void reverse_endianess(void *ptr, unsigned int length) {
    if (length <= 1) {
        return;
    }

    char tmp;

    for (int i = 0; i < length / 2; i++) {
        tmp = *(char *)(ptr + i);
        *(char *)(ptr + i) = *(char *)(ptr + (length - 1) - i);
        *(char *)(ptr + (length - 1) - i) = tmp;
    }
}