#include "acpi.h"
#include "types.h"
#include "tty.h"
#include "mem.h"
#include "string.h"

/*
The ACPI system is made of several tables. The first one called the Root System Description Table is the hierarchy root.
To start scanning the whole acpi tree the system must find the rsdp (Root System Description Pointer), a pointer to the rsdt.
Once the rsdt has been found, the system reads all the other tables to gather informations about the hardware.
This module's main function is scan_acpi(), called by the boot manager.
*/

//search for the RSDP into the extended bios data area and into another memory area
//this data has been written into memory from the firmware, once the tables has been read the system can overwrite those memory areas

void *seek_rsdp() {
    const char str[9] = "RSD PTR ";
    char _str[9];
    void *rsdp = 0;
    uint32_t ebda_ptr = 0;

    //search in the memory area
    for (int i = 0x000E0000; i < 0x000FFFF8; i++) {
        memcpy((void *) _str, (void *) i, 8);
        _str[8] = 0x00; //add the string terminator to be used with strcmp routine

        if (strcmp(_str, str) == 0) {
            rsdp = (void *) i;
            break;
        }
    }

    //search in the EBDA
    memcpy(&ebda_ptr, (void *) 0x40E, 2); //ebda address is always stored at 0x40E
    ebda_ptr <<= 4; //the address is 4 bit right shifted, we have to shift it back

    for (int i = 0; i < 1024; i++) {
        memcpy((void *) _str, (void *)(ebda_ptr + i), 8);
        _str[8] = 0x00; //add the string terminator to be used with strcmp routine

        if (strcmp(_str, str) == 0) {
            rsdp = (void *)(ebda_ptr + i);
            break;
        }
    }

    return rsdp;
}

bool do_checksum(struct acpi_table_header *table_header) {
    uint8_t sum = 0;
 
    for (int i = 0; i < table_header -> length; i++) {
        sum += ((char *) table_header)[i];
    }
 
    return sum == 0;
}

bool scan_acpi() {
    void *rsdp = seek_rsdp();

    if (rsdp == null) {
        error("RSDP not found");
        return false;
    }
    
    //reads the rsdp
    struct rsdp_descriptor rsdp_descr; //new version is being used to cover both cases, 1.0 and newer versions
    memcpy(&rsdp_descr, rsdp, RSDP_NEW_SIZE);

    //WARNING: the system has still to check the checksum
    //do_checksum()

    //reads the rsdt
    struct rsdt rsdt;
    memcpy(&rsdt, (void *) rsdp_descr.rsdt_address, RSDT_SIZE);
    static const char critical_tables[5][5] = {"APIC", "DSDT", "FACP", "SRAT", "SSDT"};

    for (int i = 0; i < 5; i++) {
        void *ptr;

        if ((ptr = find_table(critical_tables[i], &rsdt)) != null) {
            print("found ", VGA_COLOR_WHITE);
            print(critical_tables[i], VGA_COLOR_WHITE);
            print(": ", VGA_COLOR_WHITE);
            println_int((uint32_t) ptr, VGA_COLOR_WHITE);
        } else {
            println("Table not found", VGA_COLOR_WHITE);
        }
    }

    return true;
}

static void *find_table(const char *name, struct rsdt *rsdt) {
    int entries = (rsdt -> header.length - sizeof(rsdt -> header)) / 4;
    struct acpi_table_header tmp;
    char table_name[5];

    for (int i = 0; i < entries; i++) {
        memcpy(&tmp, (void *)(rsdt -> pointers[i]), ACPI_TABLE_HEADER_SIZE);
        memcpy(table_name, tmp.signature, 4);
        table_name[4] = 0x00;

        if (strcmp(table_name, name) == 0) {
            return (void *) rsdt -> pointers[i];
        }
    }

    return null;
}