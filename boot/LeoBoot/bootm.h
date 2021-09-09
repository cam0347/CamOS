#pragma once
#include "ptb.h"

void boot();
int read_acpi_tables();
struct pt_entry parse_pt();
void clean_stack_mem();
void setup_aux_stack();