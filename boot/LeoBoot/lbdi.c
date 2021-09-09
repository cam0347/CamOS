/*
This module defines the interface layer of the boot manager to the file system drivers.
Each driver must provide a set of routines to interface with the boot manager:
- initialization function
- file loading function
- file size function

The boot manager then has to call the file size function in order to allocate enugh space to read the file.
Once allocated the boot manager has to call kload to load the kernel executable.
The boot manager calls kload and pass the entry struct of the selected partition, the path to the kernel binary file and the address to the driver's interface routines.
Returns the length of the file or -1 (0xFFFFFFFF) if error.
The kernel binary is loaded at 0x28600.
*/

#include "lbdi.h"
#include "types.h"
#include "ptb.h"
#include "string.h"
#include "abort.h"

void *kelf_load = (void *) 0x28600;

uint32_t kload(struct pt_entry pt_entry, const char *k_path, bool init(uint32_t, uint32_t), uint32_t load_file(const char*, void *)) {
    uint32_t start = pt_entry.start_sector;
    uint32_t end = pt_entry.end_sector;

    //make some checks
    if (start > end) {
        abort("invalid sectors range", null);
    }

    if (strcmp(k_path, "") == STR_EQUAL) {
        abort("invalid kernel path", null);
    }

    if (init == null || load_file == null) {
        abort("null LBDI pointers", null);
    }

    //init the driver
    if (!init(start, end)) {
        abort("driver init failed", null);
    }

    //loads the binary and returns the size
    return load_file(k_path, kelf_load);
}