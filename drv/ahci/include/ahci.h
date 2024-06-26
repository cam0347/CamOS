#pragma once
#include <include/types.h>
#include <drv/ahci/include/ahci_fis.h>

typedef enum {
	ahci_ata = 0x00000101,    //SATA drive
	ahci_atapi = 0xEB140101,  //SATAPI drive
	ahci_semb = 0xC33C0101,   //enclosure management bridge
	ahci_pm = 0x96690101      //port multiplier
} ahci_device_type_t;

/* represents a port (device) on a bus */
typedef struct {
	uint32_t clb_lo;		    // 0x00, command list base address, 1K-byte aligned
	uint32_t clb_hi;		    // 0x04, command list base address upper 32 bits
	uint32_t fis_base_lo;		// 0x08, FIS base address, 256-byte aligned
	uint32_t fis_base_hi;		// 0x0C, FIS base address upper 32 bits
	uint32_t int_status;		// 0x10, interrupt status
	uint32_t int_enable;		// 0x14, interrupt enable
	uint32_t command;		    // 0x18, command and status
	uint32_t rsv0;		        // 0x1C, Reserved
	uint32_t task_file_data;	// 0x20, task file data
	uint32_t signature;		    // 0x24, signature
	uint32_t sata_status;		// 0x28, SATA status (SCR0:SStatus)
	uint32_t sata_control;		// 0x2C, SATA control (SCR2:SControl)
	uint32_t sata_error;		// 0x30, SATA error (SCR1:SError)
	uint32_t sata_active;		// 0x34, SATA active (SCR3:SActive)
	uint32_t command_issue;		// 0x38, command issue
	uint32_t sata_notification;	// 0x3C, SATA notification (SCR4:SNotification)
	uint32_t fis_switch_ctrl;	// 0x40, FIS-based switch control
	uint32_t rsv1[11];	        // 0x44 ~ 0x6F, Reserved
	uint32_t vendor[4];	        // 0x70 ~ 0x7F, vendor specific
} ahci_port_t;

/* represents the ahci controller memory mapped registers */
typedef struct {
	uint32_t host_capability;		// 0x00, Host capability
	uint32_t global_host_control;	// 0x04, Global host control
	uint32_t int_status;		    // 0x08, Interrupt status
	uint32_t port_impl;		        // 0x0C, Port implemented
	uint32_t version;		        // 0x10, Version
	uint32_t ccc_ctrl;	            // 0x14, Command completion coalescing control
	uint32_t ccc_ports;          	// 0x18, Command completion coalescing ports
	uint32_t em_loc;		        // 0x1C, Enclosure management location
	uint32_t em_ctrl;		        // 0x20, Enclosure management control
	uint32_t host_capability_ext;	// 0x24, Host capabilities extended
	uint32_t bohc;		            // 0x28, BIOS/OS handoff control and status
	uint8_t  rsv[0xA0-0x2C];        //reserved
	uint8_t  vendor[0x100-0xA0];    //vendor specific
	ahci_port_t	ports[1];	    //port control registers
} ahci_ctrl_memory_t;

/* AHCI PRDT entry */
typedef struct {
	uint32_t dba;		// Data base address
	uint32_t dbau;		// Data base address upper 32 bits
	uint32_t rsv0;		// Reserved
	uint32_t dbc: 22;	// Byte count, 4M max
	uint32_t rsv1: 9;	// Reserved
	uint32_t i: 1;		// Interrupt on completion
} ahci_prdt_entry_t;

/* command header, each device has up to 32 of these */
typedef struct {
	uint8_t comm_fis_length: 5;   // Command FIS length in DWORDS, 2 ~ 16
	uint8_t atapi: 1;		      // ATAPI
	uint8_t write: 1;		      // Write, 1: H2D, 0: D2H
	uint8_t prefetchable: 1;	  // Prefetchable
	uint8_t reset: 1;		      // Reset
	uint8_t bist: 1;		      // BIST
	uint8_t clear: 1;		      // Clear busy upon R_OK
	uint8_t rsv0: 1;		      // Reserved
	uint8_t port_mult_port: 4;    // Port multiplier port
	uint16_t prdt_llength;		  // Physical region descriptor table length in entries
	volatile uint32_t prd_bytes;  // Physical region descriptor byte count transferred
	uint32_t comm_table_lo;       // Command table descriptor base address
	uint32_t comm_table_hi;       // Command table descriptor base address upper 32 bits
	uint32_t rsv1[4];             // Reserved
} ahci_command_header_t;

/* command table, pointed by a command header */
typedef struct {
	uint8_t cfis[64];                    // Command FIS
	uint8_t acmd[16];                    // ATAPI command, 12 or 16 bytes
	uint8_t rsv[48];	                 // Reserved
	ahci_prdt_entry_t prdt_entry[1]; // Physical region descriptor table entries, 0 ~ 65535
} ahci_command_table_t;

typedef struct {
	ahci_fis_dma_setup_t dsfis;  // DMA Setup FIS
	uint8_t pad0[4];
	ahci_fis_pio_setup_t psfis;  // PIO Setup FIS
	uint8_t pad1[12];
	ahci_fis_reg_d2h_t rfis;     // Register – Device to Host FIS
	uint8_t pad2[4];
	uint8_t sdbfis[1];           // Set Device Bit FIS placeholder
	uint8_t ufis[64];
	uint8_t rsv[0x100-0xA0];
} ahci_received_fis_t;

typedef struct {
	ahci_ctrl_memory_t *ctrl; //pointer to the device's controller structure
	ahci_port_t *port;        //pointer to the device's port structure
} ahci_device_t;

bool ahci_command(ahci_device_t *dev, ahci_command_header_t comm);
ahci_command_header_t *ahci_cl_get_slot(ahci_device_t *dev);