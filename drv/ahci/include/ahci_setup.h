#pragma once
#include <include/types.h>
#include <io/include/pci.h>
#include <drv/ahci/include/ahci.h>

#define AHCI_COMMAND_LIST_SIZE 

bool ahci_init(pci_general_dev_t *dev);
bool ahci_check_flags(ahci_ctrl_memory_t *hba_mem);
void ahci_set_ahci_mode(ahci_ctrl_memory_t *hba_mem, bool enabled);
bool ahci_search_and_add_devices(ahci_ctrl_memory_t *hba_mem);
bool ahci_init_device(volatile ahci_port_t *port);