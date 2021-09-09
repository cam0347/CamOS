#pragma once
#include "LeoFS.h"
#include "../types.h"

#define END_OF_CHAIN_PTR 0xFFFFFFFF
#define BLOCK_DATA_LENGTH 504
#define LFS_OBJECT_DESCRIPTOR_LENGTH 96
#define STANDARD_PERMISSIONS 00000110
#define FLAG_UP 1
#define FLAG_DOWN 0
#define ERROR_CHAIN_BOUND 2048

void init(uint32_t start, uint32_t end);
bool read_file(const char *path, void *loc);
static int read_object(void *buf, struct LFSObjectDescriptor *start);
static bool scan_directory(char *path, struct LFSObjectDescriptor *des);
static int read_dir_from_descriptor(struct LFSObjectDescriptor *dir, struct LFSObjectDescriptor array[]);
static void reverse_descriptor_endianess(struct LFSObjectDescriptor *str);
static void lfs_drv_error(const char *str);
static void append_error_trace(char *error, char *trigger);
static void print_error_trace();
static bool read_block(struct LFSBlock *block, const uint32_t block_n);
static void load_root_descriptor();