#include <include/leokernel.h>
#include <include/bootp.h>
#include <include/low_level.h>
#include <mm/include/segmentation.h>
#include <mm/include/memory_manager.h>
#include <int/include/int.h>
#include <tty/include/tty.h>
#include <tty/include/def_colors.h>
#include <mm/include/paging.h>
#include <acpi/include/acpi_parser.h>
#include <io/include/pci.h>
#include <io/include/pci_tree.h>
#include <include/panic.h>
#include <int/include/apic.h>
#include <include/math.h>
#include <io/include/keyboard.h>
#include <io/include/files.h>
#include <io/include/pit.h>
#include <mm/include/obj_alloc.h>
#include <include/sleep.h>
#include <tty/include/term.h>
#include <mm/include/kmalloc.h>
#include <fs/ramfs/include/ramfs.h>
#include <user/include/user_mode.h>

void kmain(struct leokernel_boot_params bootp) {
    if (!check_boot_param(&bootp)) {
        sys_hlt();
    }

    if (!init_tty(&bootp)) {
        sys_hlt();
    }

    leokernel_init_subsystem(setup_gdt(), "setting up GDT");
    leokernel_init_subsystem(setup_interrupts(), "setting up interrupts");
    gdt_load_segments();
    leokernel_init_subsystem(init_mm(&bootp), "setting up memory manager");
    leokernel_init_subsystem(init_acpi(&bootp), "initializing ACPI");
    leokernel_init_subsystem(init_apic(), "setting up APIC");
    init_pit();
    apic_timer_init();
    leokernel_init_subsystem(init_pci(), "configuring PCI");
    leokernel_init_subsystem(init_ramfs(), "initializing ramfs");
    leokernel_init_subsystem(init_files(), "initializing file subsystem");
    leokernel_init_subsystem(init_um(), "initializing user mode");
    init_keyboard();
    enter_um();
    //init_terminal();
    while(true);
	sys_hlt();
}

void leokernel_init_subsystem(bool success, char *name) {
    tty_color_t prev = get_tty_char_fg();
    printf("[ ");

    if (success) {
        set_tty_char_fg(GREEN_COLOR);
        printf(" OK ");
    } else {
        set_tty_char_fg(RED_COLOR);
        printf("FAIL");
    }

    set_tty_char_fg(prev);
    printf(" ]  %s", name);
    printf("...\n");

    if (!success) {
        fail(name);
    }
}

void print_title() {
    tty_color_t prev = get_tty_char_fg();
    set_tty_char_fg(GREEN_COLOR);
    printf("*** CamOS v0.1 ***\n");
    set_tty_char_fg(prev);
}

//checks validity of boot parameters
bool check_boot_param(struct leokernel_boot_params *bootp) {
    if (bootp->frame_buffer == null) {return false;}
    if (bootp->frame_buffer_size == 0) {return false;}
    if (bootp->video_height == 0) {return false;}
    if (bootp->video_width == 0) {return false;}
    if (bootp->video_pitch == 0) {return false;}
    if (bootp->font == null) {return false;}
    if (bootp->map == null) {return false;}
    if (bootp->map_size == 0) {return false;}
    if (bootp->xsdt == null) {return false;}
    return true;
}
