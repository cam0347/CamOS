#pragma once
#include "types.h"

#define RSDP_OLD_SIZE 20
#define RSDP_NEW_SIZE 34
#define ACPI_TABLE_HEADER_SIZE 36
#define RSDT_SIZE 128 + ACPI_TABLE_HEADER_SIZE

struct rsdp_descriptor {
  char signature[8];
  uint8_t checksum;
  char oem[6];
  uint8_t revision;
  uint32_t rsdt_address;
  uint32_t length;
  uint64_t xsdt_address;
  uint8_t ext_checksum;
  uint8_t reserved[3];
  //uint32_t pointers[length - sizeof(struct acpi_table_header)]
};

//36 bytes
struct acpi_table_header {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oem_id[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creatorID;
  uint32_t creator_revision;
};

struct rsdt {
  struct acpi_table_header header;
  uint32_t pointers[32];
};

struct GenericAddressStructure {
  uint8_t address_space;
  uint8_t bit_width;
  uint8_t bit_offset;
  uint8_t access_size;
  uint64_t address;
};

struct fadt {
  struct acpi_table_header header;
  uint32_t firmware_control; //physical address of FACS (if X_FIRMWARE_CONTROL != 0 use it and ignore this field)
  uint32_t dsdt; //physical address of DSDT (if X_DSDT != 0 use it and ignore this field)
  uint8_t reserved; //no longer used, compatibility reason
  uint8_t preferred_pm_profile; //preferred power management profile
  uint16_t sci_interrupt;
  uint32_t smi_command_port;
  uint8_t acpi_enable;
  uint8_t acpi_disable;
  uint8_t s4_bios_req;
  uint8_t pstate_control;
  uint32_t pm1a_event_block;
  uint32_t pm1b_event_block;
  uint32_t pm1a_control_block;
  uint32_t pm1b_control_block;
  uint32_t pm2control_block;
  uint32_t pm_timer_block;
  uint32_t gpe0_block;
  uint32_t gpe1_block;
  uint8_t pm1_event_length;
  uint8_t pm1_control_length;
  uint8_t pm2_control_length;
  uint8_t pm_timer_length;
  uint8_t gpe0_length;
  uint8_t gpe1_length;
  uint8_t gpe1_base;
  uint8_t cstate_control;
  uint16_t worst_c2_latency;
  uint16_t worst_c3_latency;
  uint16_t flush_size;
  uint16_t flush_stride;
  uint8_t duty_offset;
  uint8_t duty_width;
  uint8_t day_alarm;
  uint8_t month_alarm;
  uint8_t century;
  uint16_t boot_architecture_flags; //reserved in ACPI 1.0; used since ACPI 2.0+
  uint8_t reserved2;
  uint32_t flags;
  struct GenericAddressStructure reset_reg; //12 byte structure; see below for details
  uint8_t reset_value;
  uint8_t reserved3[3];
  uint64_t x_firmware_control; //64bit pointers - Available on ACPI 2.0+
  uint64_t x_dsdt;
  struct GenericAddressStructure x_pm1a_event_block;
  struct GenericAddressStructure x_pm1b_event_block;
  struct GenericAddressStructure x_pm1a_control_block;
  struct GenericAddressStructure x_pm1b_control_block;
  struct GenericAddressStructure x_pm2_control_block;
  struct GenericAddressStructure x_pm_timer_block;
  struct GenericAddressStructure x_gpe0_block;
  struct GenericAddressStructure x_gpe1_block;
};

struct madt_entry_header {
  uint8_t ent_type;
  uint8_t length;
};

//this table is variable length
struct madt {
  struct acpi_table_header header;
  uint32_t local_apic_address;
  uint32_t flags;
  //struct madt_entry_header[];
};

void *seek_rsdp();
bool do_checksum(struct acpi_table_header *table_header);
bool scan_acpi();
static void *find_table(const char *name, struct rsdt *rsdt);