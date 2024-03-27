#include <fs/ramfs/include/ramfs.h>
#include <include/types.h>
#include <mm/include/memory_manager.h>

bool __ramfs_ready = false;
void *__ramfs_base = null;
uint32_t __ramfs_final_size = 0;

bool init_ramfs() {
    if ((__ramfs_base = kalloc_page(__RAMFS_PREFERRED_SIZE_IN_PAGES)) != null) {
        __ramfs_final_size = __RAMFS_PREFERRED_SIZE_IN_PAGES;
    } else if ((__ramfs_base = kalloc_page(__RAMFS_FALLBACK_SIZE_IN_PAGES)) != null) {
        __ramfs_final_size = __RAMFS_FALLBACK_SIZE_IN_PAGES;
    } else if ((__ramfs_base = kalloc_page(__RAMFS_MIN_SIZE_IN_PAGES)) != null) {
        __ramfs_final_size = __RAMFS_MIN_SIZE_IN_PAGES;
    } else {
        return false; //allocation failed, return false
    }



    __ramfs_ready = true;
    return true;
}