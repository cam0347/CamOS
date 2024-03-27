#pragma once
#include <include/bootp.h>

void kmain(struct leokernel_boot_params);
void leokernel_init_subsystem(bool success, char *name);
bool check_boot_param(struct leokernel_boot_params *bootp);
void print_title();