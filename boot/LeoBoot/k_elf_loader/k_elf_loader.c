#include "../types.h"
#include "../tty.h"
#include "../string.h"
#include "../mem.h"
#include "../abort.h"
#include "k_elf_loader.h"
#define VERBOSE false

//parses the kernel's elf file passed as a void pointer
bool k_parse(void *elf) {
    if (elf == null) {
        return false;
    }

    if (!validate_elf(elf)) {
        abort("invalid elf signature", null);
    }

    struct elf_header_32 header;
    memcpy(&header, elf, ELF_HSIZE_32);

    if (VERBOSE) {
        print("Kernel entry point: ", VGA_COLOR_WHITE);
        println_hex(header.e_entry, VGA_COLOR_WHITE);
    }

    struct sh_entry_32 names_sec;
    memcpy(&names_sec, (elf + header.e_shoff + header.e_shstrndx * header.e_shentsize), header.e_shentsize);
    void *section_names = elf + names_sec.file_offset;

    //if is relocatable, relocate it and continue
    if ((enum elf_type) header.e_type == rel) {
        //load_rel(elf, (struct sh_entry_32 *) header.e_shoff, names_sec);
    }

    //seek the sections and save absolute load location and size
    const char cr_sec[4][8] = {".text", ".data", ".rodata", ".bss"};
    uint32_t cr_off[4] = {0, 0, 0, 0};
    uint32_t cr_len[4] = {0, 0, 0, 0};
    bool cr_found[4] = {false, false, false, false};

    struct sh_entry_32 *sh_entries = (struct sh_entry_32 *)(elf + header.e_shoff);
    for (uint16_t i = 0; i < header.e_shnum; i++) {
        for (uint8_t j = 0; j < 4; j++) {
            if (strcmp((char *) (section_names + sh_entries[i].name_offset), cr_sec[j]) == STR_EQUAL) {
                cr_off[j] = (uint32_t) elf + sh_entries[i].file_offset;
                cr_len[j] = sh_entries[i].size;
                cr_found[j] = true;
            }
        }
    }

    if (!cr_len[CR_TEXT] || !cr_found[CR_DATA]) {
        abort("kernel critical sections not found, kernel binary could be corrupted", null);
    }

    ktext_address = (void *) kmin_address;
    kdata_address = ktext_address + cr_len[CR_TEXT];
    krodata_address = kdata_address + cr_len[CR_DATA];
    kbss_address = krodata_address + cr_len[CR_RODATA];

    //loads sections in memory
    memcpy(ktext_address, (void *) cr_off[CR_TEXT], cr_len[CR_TEXT]);
    memcpy(kdata_address, (void *) cr_off[CR_DATA], cr_len[CR_DATA]);

    if (cr_found[CR_RODATA]) {
        memcpy(krodata_address, (void *) cr_off[CR_RODATA], cr_len[CR_RODATA]);
    }

    if (cr_found[CR_BSS]) {
        memcpy(kbss_address, (void *) cr_off[CR_BSS], cr_len[CR_BSS]);
    }

    return true;
}

bool validate_elf(const void *file) {
    const char magic[] = {0x7F, 'E', 'L', 'F'};
    for (uint8_t i = 0; i < 4; i++) {
        if (*(char *)(file + i) != magic[i]) {
            return false;
        }
    }

    return true;
}

bool load_rel(void *elf, void *names_section, struct sh_entry_32 sh[], uint32_t sh_length) {
    uint32_t text_rel_index = -1, data_rel_index = -1;

    for (uint32_t i = 0; i < sh_length; i++) {
        if (strcmp(names_section + sh[i].name_offset, ".rel.text")) {
            text_rel_index = i;
        } else if (strcmp(names_section + sh[i].name_offset, ".rel.data")) {
            data_rel_index = i;
        }
    }

    if (text_rel_index != -1) {

    }

    return true;
}