/*
Zavattaro Camillo
20/05/2021

LeoFS structures and routines definition

The disk is divided into 512 bytes blocks, each of them contains 508 bytes of data and 4 bytes for
the next block's index. A directory is a block or a sequence of them containing LFSObjectDescriptor structures.
*/

#define LEOFSH 1

#ifndef TYPESH
    #include "../../include/types.h"
#endif

#ifndef TIMEH
    #include "../../time.h"
#endif

//4 byte (int)
typedef enum LFSObjectType {
    lfs_directory = 0,
    lfs_file = 1,
    lfs_oth = 2,
    lfs_unknown = 3
} LFSObjectType;

//FS's basic block, containing data and next block's number, 512 bytes
struct LFSBlock {
    uint8_t data[504];    //504 bytes
    uint32_t prev;  //4 bytes
    uint32_t next;  //4 bytes
};

//6 byte
struct LFSPointer {
    uint32_t block; //4 byte
    uint16_t offset; //2 byte
};

struct LFSFlags {
    uint8_t fl7: 1;
    uint8_t fl_orphan: 1;
    uint8_t fl_corrupted: 1;
    uint8_t fl_root: 1;
    uint8_t fl_locked: 1;
    uint8_t fl_hidden: 1;
    uint8_t fl_edited: 1;
    uint8_t fl_valid: 1;
};

//88 byte
struct LFSObjectDescriptor {
    char name[32]; //32 byte
    uint32_t size; //4 byte (object's size in blocks (504 byte), directory: number of childs)(max file size: 2016 GByte)
    uint32_t bsize; //4 byte
    uint32_t first_block; //4 byte (object's first block number)
    struct full_date_t creation; //8 byte
    struct full_date_t last_edit; //8 byte
    struct full_date_t last_opened; //8 byte
    uint8_t permissions; //1 byte
    enum LFSObjectType obj_type; //4 byte
    struct LFSPointer location; //8 byte
    struct LFSPointer parent; //8 byte
    struct LFSFlags flags;
    char padding[2]; //2 bytes
};