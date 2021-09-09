#include "../types.h"
#include "../string.h"
#include "../tty.h"
#include "../mem.h"
#include "../disk.h"
#include "lfs_drv.h"

static char error_chain[ERROR_CHAIN_BOUND];
static uint32_t error_chain_length = 0;
static struct LFSObjectDescriptor root_descriptor;
static uint32_t FS_FIRST_BLOCK, FS_END_BLOCK;
static bool _init = false;

//LBDI routines
void init(uint32_t start, uint32_t end) {
    FS_FIRST_BLOCK = start;
    FS_END_BLOCK = end;
    _init = true;
}

bool read_file(const char *path, void *loc) {
    if (!_init) {
        return false;
    }

    //loads the file
}

//module's output point, this function is the only one accessible by other modules, thus this is the output point
static int read_object(void *buf, struct LFSObjectDescriptor *start) {
    uint32_t count = start -> size; //object's blocks number
    struct LFSBlock block; //structure in which save the blocks one by one
    uint32_t bytes = 0;
    uint32_t block_n = start -> first_block;

    for (int i = 0; i < count; i++) {
        if (block_n == END_OF_CHAIN_PTR) {
            return 0;
        }

        if (!read_block(&block, block_n)) {
            append_error_trace("error reading block", "read_object");
            return -1;
        }

        memcpy(buf + BLOCK_DATA_LENGTH * i, &block.data, BLOCK_DATA_LENGTH);
        bytes += BLOCK_DATA_LENGTH;
        block_n = block.next;
    }

    return bytes;
}

static bool scan_directory(char *path, struct LFSObjectDescriptor *des) {
    int path_length = strlen(path);
    if (path_length <= 0) {
        append_error_trace("invalid path length", "scan_directory");
        return false;
    }

    if (strcmp(path, "/") == STR_EQUAL) {
        memcpy(des, &root_descriptor, LFS_OBJECT_DESCRIPTOR_LENGTH);
        return true;
    }

    //read root directory first
    int subdirs_n = root_descriptor.bsize / LFS_OBJECT_DESCRIPTOR_LENGTH; //number of subdirectories in the current directory (starting with root directory)
    struct LFSObjectDescriptor subdirs[100]; //array of descriptors (100 max subdirectories)(to improve)
    struct LFSObjectDescriptor current; //current directory's descriptor

    if (read_dir_from_descriptor(&root_descriptor, subdirs) == -1) { //disk -> mem (ref)
        append_error_trace("starting subdirectory (root) reading error", "scan_directory");
        return false;
    }
   
    char *token = strtok(path, "/");
    bool found = false;

    while (token != null) {
        //scan all the subdirectories name
        for (int i = 0; i < subdirs_n; i++) {
            //if found, read the new subdirectory and set subdirs_n to the number of descriptor in that directory
            if (strcmp(subdirs[i].name, token) == STR_EQUAL) {
                memcpy(&current, &subdirs[i], LFS_OBJECT_DESCRIPTOR_LENGTH); //copy the right descriptor into current
                subdirs_n = current.bsize / LFS_OBJECT_DESCRIPTOR_LENGTH; //update the number of descriptor

                //read the new directory
                if (current.obj_type == lfs_directory) {
                    if (read_dir_from_descriptor(&current, subdirs) == -1) {
                        append_error_trace("subdirectory reading error", "scan_directory");
                        return false;
                    }
                }

                found = true;
                break;
            }
        }

        if (!found) {
            append_error_trace("subdirectory not found", "scan_directory");
            return false;
        }

        found = 0;
        token = strtok(token, "/");
    }

    //the cycle ends only if the directory exists
    memcpy(des, &current, LFS_OBJECT_DESCRIPTOR_LENGTH);
    return true;
}

static int read_dir_from_descriptor(struct LFSObjectDescriptor *dir, struct LFSObjectDescriptor array[]) {
    int n_descriptors = dir -> bsize / LFS_OBJECT_DESCRIPTOR_LENGTH; //calculate the number of descriptor contained in this directory

    if (dir -> obj_type != lfs_directory) {
        append_error_trace("descriptor not a directory", "read_dir_from_descriptor");
        return -1;
    }

    char buffer[dir -> bsize]; //allocate a buffer that can hold the exact number of bytes

    if (read_object(buffer, dir) == -1) {
        append_error_trace("directory reading error", "read_dir_from_descriptor");
        return -1;
    }

    //transfer the descriptors into array, one by one
    for (int i = 0; i < n_descriptors; i++) {
        memcpy(&array[i], buffer + i * LFS_OBJECT_DESCRIPTOR_LENGTH, LFS_OBJECT_DESCRIPTOR_LENGTH);
        reverse_descriptor_endianess(&array[i]);
    }

    return n_descriptors;
}

static void reverse_descriptor_endianess(struct LFSObjectDescriptor *str) {
    reverse_endianess(&(str -> size), 4);
    reverse_endianess(&(str -> bsize), 4);
    reverse_endianess(&(str -> first_block), 4);
    reverse_endianess(&(str -> obj_type), 4);
    reverse_endianess(&(str -> location.block), 4);
    reverse_endianess(&(str -> location.offset), 2);
    reverse_endianess(&(str -> parent.block), 4);
    reverse_endianess(&(str -> parent.offset), 2);
}

static void lfs_drv_error(const char *str) {
    char _str[strlen(str) + 15];
    strcpy(_str, "LeoFS driver: ");
    strcat(_str, str);
    error(_str);
}

static void append_error_trace(char *error, char *trigger) {
    if (error_chain_length + strlen(error) > ERROR_CHAIN_BOUND) {
        lfs_drv_error("ERROR TRACE BUFFER FULL");
        print_error_trace();
        return;
    }

    strcat(error_chain, trigger);
    strcat(error_chain, ": ");
    strcat(error_chain, error);
    strcat(error_chain, "$");
    error_chain_length += strlen(error);
}

static void print_error_trace() {
    char *subs = strtok(error_chain, "$");
    char buffer[103];
    println("[ERROR TRACE]", VGA_COLOR_LIGHT_BLUE);

    while (subs != null) {
        strcat(buffer, "-> ");
        strcat(buffer, subs);
        println(buffer, VGA_COLOR_LIGHT_BLUE);
        subs = strtok(subs, "$");
    }

    strcpy(error_chain, "");
    error_chain_length = 0;
}

static bool read_block(struct LFSBlock *block, const uint32_t block_n) {
    if (ata_read_sector(block_n, 1, (void *) block)) {
        return false;
    }

    reverse_endianess(&(block -> prev), 4); //to little
    reverse_endianess(&(block -> next), 4); //to little

    return true;
}

static void load_root_descriptor() {
    struct LFSBlock root_block;
    read_block(&root_block, FS_FIRST_BLOCK);
    memcpy(&root_descriptor, &(root_block.data), LFS_OBJECT_DESCRIPTOR_LENGTH);
    reverse_descriptor_endianess(&root_descriptor);
}