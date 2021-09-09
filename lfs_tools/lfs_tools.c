#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include "lfs_tools.h"
#include "partition_structs.h"

#define END_OF_CHAIN_PTR 0xFFFFFFFF
#define BLOCK_DATA_LENGTH 504
#define LFS_OBJECT_DESCRIPTOR_LENGTH 96
#define STANDARD_PERMISSIONS 00000110
#define MIN_BLOCK_NUM 10
#define FLAG_UP 1
#define FLAG_DOWN 0
#define ERROR_CHAIN_BOUND 2048
#define DEFAULT_DISK_PATH "../disk.iso"
#define PARTITION_TABLE_OFFSET 512 * 1

FILE *file;
char *file_name;
struct stat file_info;
int FS_FIRST_BLOCK, FS_END_BLOCK;
struct LFSObjectDescriptor root_descriptor;
int fs_provided;

char *error_chain;
long error_chain_length = 0;

int main() {
    system("clear");

    file_name = malloc(50);
    strcpy(file_name, DEFAULT_DISK_PATH);
    file = fopen(file_name, "r+");

    while(file == NULL) {
        printf("Disk name: ");
        scanf("%s", file_name);
        file = fopen(file_name, "r+");
    }

    if (stat(file_name, &file_info) == -1) {
        printf("Error: gathering file inode\n");
        return 1;
    }

    printf("Disk selected: %s\n", file_name);

    error_chain = (char *) malloc(ERROR_CHAIN_BOUND);
    if (!error_chain) {
        printf("Error chain buffer failed\n");
        return 0;
    }

    //search LeoFS partitions on partition table
    struct pt_entry entries[32], entry;
    int leofs_partitions[32];
    int leofs_partitions_number = 0;

    for (int i = 0; i < 32; i++) {
        fseek(file, PARTITION_TABLE_OFFSET + 16 * i, SEEK_SET);
        fread(&entry, 16, 1, file);
        reverse_endianess(&entry.pt_type, 4);
        reverse_endianess(&entry.start_sector, 4);
        reverse_endianess(&entry.end_sector, 4);
        memcpy(&entries[i], &entry, 16);

        if ((enum partition_type) entry.pt_type == leofs) {
            leofs_partitions[leofs_partitions_number] = i;
            leofs_partitions_number++;
        } else if (entry.magic == 0x00) {
            break;
        }
    }

    if (leofs_partitions_number == 0) {
        printf("No LeoFS partitions found on this disk, use a partitioning tool to create one\n");
        return 0;
    } else if (leofs_partitions_number == 1) {
        FS_FIRST_BLOCK = entries[leofs_partitions[0]].start_sector;
        FS_END_BLOCK = entries[leofs_partitions[0]].end_sector;
    } else {
        for (int i = 0; i < leofs_partitions_number; i++) {
            printf("%d: %d\n", i, leofs_partitions[i]);
        }

        int part_number;
        printf("I found multiple partitions formatted with LeoFS on this disk, which one would you like to use [0 ~ %d]?: ", leofs_partitions_number - 1);
        scanf("%d", &part_number);

        if (part_number < 0 || part_number >= leofs_partitions_number) {
            printf("Invalid partition number\n");
            return 0;
        }

        FS_FIRST_BLOCK = entries[leofs_partitions[part_number]].start_sector;
        FS_END_BLOCK = entries[leofs_partitions[part_number]].end_sector;
    }

    printf("First sector: %d\n", FS_FIRST_BLOCK);
    printf("Last sector: %d\n", FS_END_BLOCK);

    get_fs_info(); //get file system info

    while(1) {
        print_menu();
        switch_choice(get_choice());
        printf("[operation complete, press enter to continue...]");
        load_root_descriptor();
        getchar();
        getchar();
        system("clear");
    }

    return 0;
}

void append_error_trace(char *error, char *trigger) {
    if (error_chain_length + strlen(error) > ERROR_CHAIN_BOUND) {
        printf("ERROR TRACE BUFFER FULL\n");
        print_error_trace();
        return;
    }

    strcat(error_chain, trigger);
    strcat(error_chain, ": ");
    strcat(error_chain, error);
    strcat(error_chain, "$");
    error_chain_length += strlen(error);
}

void print_error_trace() {
    char *token = strtok(error_chain, "$");
    printf("[ERROR TRACE]\n");

    while (token != NULL) {
        printf("-> %s\n", token);
        token = strtok(NULL, "$");
    }

    strcpy(error_chain, "");
    error_chain_length = 0;
}

void reverse_endianess(void *ptr, unsigned int length) {
    if (length <= 1) {
        return;
    }

    char tmp;

    for (int i = 0; i < length / 2; i++) {
        tmp = *(char *)(ptr + i);
        *(char *)(ptr + i) = *(char *)(ptr + (length - 1) - i);
        *(char *)(ptr + (length - 1) - i) = tmp;
    }
}

void reverse_descriptor_endianess(struct LFSObjectDescriptor *str) {
    reverse_endianess(&(str -> size), 4);
    reverse_endianess(&(str -> bsize), 4);
    reverse_endianess(&(str -> first_block), 4);
    reverse_endianess(&(str -> obj_type), 4);
    reverse_endianess(&(str -> location.block), 4);
    reverse_endianess(&(str -> location.offset), 2);
    reverse_endianess(&(str -> parent.block), 4);
    reverse_endianess(&(str -> parent.offset), 2);
}

void get_fs_info() {
    load_root_descriptor();

    if (strcmp(root_descriptor.name, "root") == 0) {
        fs_provided = 1;
    } else {
        fs_provided = 0;
        printf("[this disk is not formatted with LeoFS, use option n. 1 to format]\n");
        init_descriptor(&root_descriptor, "root", FS_FIRST_BLOCK, STANDARD_PERMISSIONS, lfs_directory); //automatically set size to 1
        root_descriptor.bsize = LFS_OBJECT_DESCRIPTOR_LENGTH;
        root_descriptor.location.block = FS_FIRST_BLOCK;
        root_descriptor.location.offset = 0;
        root_descriptor.flags.fl_root = FLAG_UP;
        reverse_descriptor_endianess(&root_descriptor);
    }

    if (fs_provided) {
        printf("LeoFS provided\nRoot directory info:\n");
        printf("Name: %s\n", root_descriptor.name);
        printf("Blocks: %d\n", root_descriptor.size);
        printf("Bytes: %d (%d objects)\n", root_descriptor.bsize, root_descriptor.bsize / LFS_OBJECT_DESCRIPTOR_LENGTH);
        printf("First block: %d\n", root_descriptor.first_block);
        printf("Location: %d %d\n", root_descriptor.location.block, root_descriptor.location.offset);
    }
}

void print_menu() {
    printf("\nSTREAM CONTROL\n");
    printf("0: exit\n");                    //yes

    printf("\nFS OPERATIONS\n");
    printf("1: init LeoFS\n");              //yes
    printf("2: move file to LeoFS\n");      //yes
    printf("3: move file to host fs\n");    //yes
    printf("4: gather fs infos\n");         //yes
    printf("5: checks fs integrity\n");     //no

    printf("\nFILES & DIRECTORIES\n");
    printf("6: create directory\n");        //yes
    printf("7: read object\n");             //yes
    printf("8: delete object\n");           //yes
    printf("9: migrate object\n");          //yes
    printf("10: get object info\n");        //yes
    printf("11: edit object info\n");       //yes

    printf("\nRAW DATA OPERATIONS\n");
    printf("12: read raw block\n");         //yes
    printf("13: get block info\n");         //yes
}

int get_choice() {
    int n;

    do {
        printf("Operation: ");
        scanf("%d", &n);
    } while (n < 0 || n > 13);

    return n;
}

void switch_choice(int n) {
    switch(n) {
        case 0:
            fclose(file);
            free(file_name);
            free(error_chain);
            exit(0);
            break;

        case 1:
            init_fs();
            break;

        case 2:
            f_move_to_leofs();
            break;

        case 3:
            f_move_to_host();
            break;

        case 4:
            f_get_fs_info();
            break;

        case 5:
            check_fs_integrity();
            break;

        case 6:
            f_create_directory();
            break;

        case 7:
            f_read_object();
            break;

        case 8:
            f_delete_object();
            break;

        case 9:
            f_migrate_object();
            break;

        case 10:
            f_get_object_info();
            break;

        case 11:
            f_edit_object_info();
            break;

        case 12:
            f_read_block();
            break;

        case 13:
            f_get_block_info();
            break;

        default:
            printf("Unknown choice number");
            break;
    }
}

void print_date(struct full_date_t date) {
    int y, m, d, h, mm, s;
    y = date.date.y;
    m = date.date.m;
    d = date.date.d;
    h = date.time.h;
    mm = date.time.m;
    s = date.time.s;

    switch(m) {
        case 1:
            printf("gen");
            break;

        case 2:
            printf("feb");
            break;

        case 3:
            printf("mar");
            break;

        case 4:
            printf("apr");
            break;

        case 5:
            printf("may");
            break;

        case 6:
            printf("jun");
            break;

        case 7:
            printf("jul");
            break;

        case 8:
            printf("aug");
            break;

        case 9:
            printf("sep");
            break;

        case 10:
            printf("oct");
            break;

        case 11:
            printf("nov");
            break;

        case 12:
            printf("dec");
            break;
    }

    printf(" %d", d);
    printf(" %d", y);
    printf(" %d:%d:%d ", h, mm, s);
}

void print_object_data(struct LFSObjectDescriptor obj) {
    //print type
    if (obj.obj_type == lfs_directory) {
        printf("D ");
    } else if (obj.obj_type == lfs_file) {
        printf("F ");
    } else if (obj.obj_type == lfs_oth) {
        printf("O ");
    } else {
        printf("? ");
    }

    if (is_hidden(obj)) {
        printf("(H) ");
    }

    if (is_locked(obj)) {
        printf("(L) ");
    }

    if (is_corrupted(obj)) {
        printf("(C) ");
    }

    printf("%10s ", obj.name);
    printf("%3dB %3db ", obj.size, obj.bsize);
    printf("%3d ", obj.first_block);
    printf("%d:%d ", obj.location.block, obj.location.offset);
    printf("%d:%d ", obj.parent.block, obj.parent.offset);
    print_date(obj.creation);
    printf(" ");
    print_date(obj.last_edit);
    printf(" ");
    print_date(obj.last_opened);
    printf("\n");
}

int u_get_object(struct LFSObjectDescriptor *obj) {
    char *path = malloc(100);
    if (!path) {
        printf("Buffer error\n");
        return 0;
    }

    printf("Path: ");
    scanf("%s", path);

    if (strcmp(path, "") == 0) {
        return 0;
    }

    if (!scan_directory(path, obj)) {
        return 0;
    }

    return 1;
}

int u_get_choice() {
    char *ans = malloc(2);
    printf("Proceed? [y/n]: ");
    scanf("%s", ans);

    return *ans == 'y' || *ans == 'Y';
}

void f_delete_object() {
    struct LFSObjectDescriptor obj;
    if (!u_get_object(&obj)) {
        printf("Object not found\n");
        return;
    }

    if (obj.bsize > 0) {
        printf("Warning: \"%s\" is not empty\n", obj.name);
    }

    if (u_get_choice()) {
        if (delete_object(&obj)) {
            printf("\"%s\" deleted successfully\n", obj.name);
        } else {
            print_error_trace();
        }
    } else {
        printf("Operation suspended\n");
    }
}

void f_create_directory() {
    struct LFSObjectDescriptor dir;

    if (!u_get_object(&dir)) {
        printf("Parent directory not found\n");
        return;
    }

    if (dir.obj_type != lfs_directory) {
        printf("Not a directory\n");
        return;
    }

    int subdirs_n = dir.bsize / LFS_OBJECT_DESCRIPTOR_LENGTH;
    struct LFSObjectDescriptor subdirs[subdirs_n];
    if (read_dir_from_descriptor(&dir, subdirs) == -1) {
        print_error_trace();
        return;
    }

    printf("Files & directories in %s:\n", dir.name);

    for (int i = 0; i < subdirs_n; i++) {
        print_object_data(subdirs[i]);
    }

    char *name = malloc(32);
    if (!name) {
        printf("Buffer error\n");
        return;
    }

    printf("Directory name (max 32 characters): ");
    scanf("%s", name);

    for (int i = 0; i < subdirs_n; i++) {
        if (strcmp(name, subdirs[i].name) == 0) {
            printf("Directory already exists\n");
            return;
        }
    }

    validate_object_name(name);

    struct LFSObjectDescriptor subdir;
    if (mkdir_from_descriptor(&dir, &subdir, name)) {
        printf("\"%s\" created successfully\n", name);
    } else {
        print_error_trace();
    }

    free(name);
}

void f_read_block() {
    int block_n;
    void *data = malloc(BLOCK_DATA_LENGTH);

    if (!data) {
        printf("Buffer error\n");
        return;
    }

    printf("Block number: ");
    scanf("%d", &block_n);

    if (block_n < FS_FIRST_BLOCK || block_n > FS_END_BLOCK) {
        printf("Invalid block number, choose a block between %d and %d\n", FS_FIRST_BLOCK, FS_END_BLOCK);
        free(data);
        return;
    }

    if (!read_raw_block(block_n, data)) {
        print_error_trace();
        free(data);
        return;
    }

    system("clear");
    printf("Block n. %d - %d bytes\n", block_n, BLOCK_DATA_LENGTH);

    printf("(Character representation)\n");
    for (int i = 0; i < BLOCK_DATA_LENGTH; i++) {
        printf("%c ", *(char *)(data + i));
    }

    printf("\n\n(Numeric representation)\n");
    for (int i = 0; i < BLOCK_DATA_LENGTH; i++) {
        printf("%d ", *(unsigned char *)(data + i));
    }

    printf("\n");
    free(data);
}

void f_get_block_info() {
    int block_n;
    struct LFSBlock block;

    printf("Block number: ");
    scanf("%d", &block_n);

    if (block_n < FS_FIRST_BLOCK || block_n > FS_END_BLOCK) {
        printf("Invalid block number, choose a block between %d and %d\n", FS_FIRST_BLOCK, FS_END_BLOCK);
        return;
    }

    if (!read_block(&block, block_n)) {
        print_error_trace();
        return;
    }

    int root_block = block_n, sequence_n = 0;
    struct LFSBlock tmp;
    tmp.prev = block.prev;

    while (tmp.prev != root_block) {
        root_block = tmp.prev;
        sequence_n++;

        if (!read_block(&tmp, tmp.prev)) {
            break;
        }
    }

    printf("Block selected: %d\n", block_n);
    printf("Absolute offset: %d\n", 512 * block_n);

    if (block.next == 0 && block.prev == 0) {
        printf("Void block (uninitialized)\n");
        return;
    }

    printf("Next block: %d ", block.next);

    if (block.next == END_OF_CHAIN_PTR) {
        printf("(last of the chain)");
    }

    printf("\n");


    printf("Previous block: %d ", block.prev);

    if (block.prev == 0) {
        printf("(first of the chain)");
    }

    printf("\n");

    printf("Chain's first block: %d ", root_block);

    if (root_block == block_n) {
        printf("(first of the chain)");
    }

    printf("\n");
    
    printf("Block's sequence number: %d\n", sequence_n);
}

void f_get_object_info() {
    struct LFSObjectDescriptor obj;
    if (!u_get_object(&obj)) {
        printf("Object not found\n");
        return;
    }

    print_object_data(obj);
}

void f_get_fs_info() {
    printf("Partition first block: %d\n", FS_FIRST_BLOCK);
    printf("Partition last block: %d\n", FS_END_BLOCK);
}

void f_migrate_object() {
    struct LFSObjectDescriptor target, dest;

    printf("Target object\n");
    if (!u_get_object(&target)) {
        printf("Object not found\n");
        return;
    }

    if (is_root(target)) {
        printf("Sorry, is not possible to migrate the root directory\n");
        return;
    }

    printf("Destination directory\n");
    if (!u_get_object(&dest)) {
        printf("Object not found\n");
        return;
    }

    if (dest.obj_type != lfs_directory) {
        printf("Destination must be a directory\n");
        return;
    }

    if (are_equal(target, dest)) {
        printf("Sorry, is not possible to migrate an object into itself\n");
    }

    if (target.bsize > 0) {
        printf("Warning: \"%s\" is not empty\n", target.name);
    }

    if (u_get_choice()) {
        if (migrate_from_descriptor(&target, &dest)) {
            printf("%s moved to %s\n", target.name, dest.name);
        } else {
            print_error_trace();
        }
    } else {
        printf("Operation suspended\n");
    }
}

void f_read_object() {
    struct LFSObjectDescriptor obj;
    if (!u_get_object(&obj)) {
        printf("Object not found\n");
        return;
    }

    printf("Object type: ");

    if (obj.obj_type == lfs_directory) {
        printf("directory\n");
        print_directory(obj);
    } else if (obj.obj_type == lfs_file) {
        printf("file\n");
        //print_file(obj);
    } else {
        printf("other or unknown\n");
        printf("Cannot do anything...\n");
    }
}

void f_edit_object_info() {
    struct LFSObjectDescriptor obj;

    if (!u_get_object(&obj)) {
        printf("Object not found\n");
        return;
    }

    int choice;
    printf("0: name\n");
    printf("1: permissions\n");
    printf("2: toggle hidden flag\n");
    printf("3: toggle locked flag\n");
    printf("4: toggle corrupted flag\n");

    printf("What would you like to edit? ");
    scanf("%d", &choice);

    switch (choice) {
        case 0: {
            char *name = malloc(32);

            if (!name) {
                printf("Buffer error\n");
                return;
            }

            printf("Old name: %s\n", obj.name);
            printf("New name: ");
            scanf("%s", name);
            validate_object_name(name);
            strcpy(obj.name, name);

            free(name);
        } break;

        case 1: {
            int r, w, x;
            unsigned char permissions;

            printf("This object will be readable? ");
            scanf("%d", &r);

            printf("This object will be writable? ");
            scanf("%d", &w);

            printf("This object will be executable? ");
            scanf("%d", &x);

            permissions += (x & 00000001) << 2;
            permissions += (w & 00000001) << 1;
            permissions += (r & 00000001) << 0;

            obj.permissions = permissions;
        } break;

        case 2: {
            if (is_hidden(obj)) {
                obj.flags.fl_hidden = FLAG_DOWN;
                printf("%s marked as visible\n", obj.name);
            } else {
                obj.flags.fl_hidden = FLAG_UP;
                printf("%s marked as hidden\n", obj.name);
            }
        } break;

        case 3: {
            if (is_locked(obj)) {
                obj.flags.fl_locked = FLAG_DOWN;
                printf("%s marked as unlocked\n", obj.name);
            } else {
                obj.flags.fl_locked = FLAG_UP;
                printf("%s marked as locked\n", obj.name);
            }
        } break;

        case 4: {
            if (is_corrupted(obj)) {
                obj.flags.fl_corrupted = FLAG_DOWN;
                printf("%s marked as intact\n", obj.name);
            } else {
                obj.flags.fl_corrupted = FLAG_UP;
                printf("%s marked as corrupted\n", obj.name);
            }
        } break;

        default:
            printf("Invalid option\n");
            return;
    }

    if (!update_descriptor(&obj)) {
        print_error_trace();
    }
}

void f_move_to_leofs() {
    FILE *file;
    char *host_file_name = malloc(100);
    char *leofs_file_name = malloc(32);
    struct stat inode;
    struct LFSObjectDescriptor parent, tmp;

    if (!host_file_name || !leofs_file_name) {
        printf("Buffer error\n");
        return;
    }

    if (!u_get_object(&parent)) {
        printf("Parent directory not found\n");
        return;
    }

    if (parent.obj_type != lfs_directory) {
        printf("Not a directory\n");
        free(host_file_name);
        free(leofs_file_name);
        return;
    }

    printf("File name: ");
    scanf("%s", leofs_file_name);
    validate_object_name(leofs_file_name);

    printf("File path (on host file system): ");
    scanf("%s", file_name);

    file = fopen(file_name, "r");

    if (file == NULL) {
        printf("File not found: %s\n", file_name);
        free(file_name);
        return;
    }

    if (stat(file_name, &inode) == -1) {
        printf("Error getting file inode\n");
        free(file_name);
        return;
    }

    if (inode.st_size > (FS_END_BLOCK - FS_FIRST_BLOCK) * 512) {
        printf("Warning: file too big\n");
        return;
    }

    void *data = malloc(inode.st_size);

    if (!data) {
        printf("Buffer error\n");
        free(host_file_name);
        free(leofs_file_name);
        return;
    }

    fread(data, inode.st_size, 1, file);

    printf("%s is %lld byte length\n", file_name, inode.st_size);
    if (!u_get_choice()) {
        free(file_name);
        printf("Operation suspended\n");
        return;
    }

    printf("Transferring %lld bytes to %s...\n", inode.st_size, parent.name);

    if (mkfile(&parent, &tmp, leofs_file_name, data, inode.st_size)) {
        printf("Transfer complete\n");
    } else {
        printf("Transfer error\n");
    }

    free(host_file_name);
    free(leofs_file_name);
    free(data);
}

void f_move_to_host() {
    struct LFSObjectDescriptor file;
    char *host_path = malloc(1024);
    FILE *host_file;

    if (!host_path) {
        printf("Buffer error\n");
        return;
    }

    if (!u_get_object(&file)) {
        printf("Object not found\n");
        return;
    }

    printf("Host path: ");
    scanf("%s", host_path);

    host_file = fopen(host_path, "wb");
    free(host_path);

    printf("%s is %u byte length\n", file.name, file.bsize);
    if (!u_get_choice()) {
        fclose(host_file);
        printf("Operation suspended\n");
        return;
    }

    printf("Transferring %u bytes to %s...\n", file.bsize, host_path);
    void *data = malloc(file.bsize);

    if (!data) {
        printf("Buffer error\n");
        fclose(host_file);
        return;
    }

    if (read_object(data, &file) != -1) {
        printf("File ready to be transferred\n");
    } else {
        printf("File reading error\n");
        free(data);
        fclose(host_file);
        return;
    }

    if (fwrite(data, file.bsize, 1, host_file) == 1) {
        printf("Transfer complete\n");
    } else {
        printf("Transfer error\n");
    }

    printf("\nShould i remove %s from LeoFS? ", file.name);
    if (u_get_choice()) {
        if (delete_object(&file)) {
            printf("File deleted\n");
        } else {
            print_error_trace();
        }
    } else {
        printf("Ok, i'll keep the file\n");
    }

    free(data);
    fclose(host_file);
}

void check_fs_integrity() {
    system("clear");
    printf("LeoFS integrity test started\n");

    printf("File system name and version... ");

}

void load_root_descriptor() {
    struct LFSBlock root_block;
    read_block(&root_block, FS_FIRST_BLOCK);
    memcpy(&root_descriptor, &(root_block.data), LFS_OBJECT_DESCRIPTOR_LENGTH);
    reverse_descriptor_endianess(&root_descriptor);
}

//write the root directory and subdirectories
void init_fs() {
    if (fs_provided) {
        printf("LeoFS already written\n");
        return;
    }
    
    printf("Writing root directory from block #%d\n", FS_FIRST_BLOCK);

    //write the root directory descriptor
    struct LFSBlock root_block;
    init_block(&root_block, 0, END_OF_CHAIN_PTR); //mem -> mem (ref)
    write_block(&root_block, FS_FIRST_BLOCK); //mem -> disk (ref)
    write_block_data(FS_FIRST_BLOCK, &root_descriptor, LFS_OBJECT_DESCRIPTOR_LENGTH); //mem -> disk (ref)

    //at this point root descriptor's endianess is still big endian (must be changed to create subdirs)
    //turn back to little endian
    reverse_descriptor_endianess(&root_descriptor);

    //add the subdirectories
    struct LFSObjectDescriptor subdirs[7];
    mkdir_from_descriptor(&root_descriptor, &subdirs[0], "bin");  //mem -> disk (ref)
    mkdir_from_descriptor(&root_descriptor, &subdirs[1], "sbin"); //mem -> disk (ref)
    mkdir_from_descriptor(&root_descriptor, &subdirs[2], "boot"); //mem -> disk (ref)
    mkdir_from_descriptor(&root_descriptor, &subdirs[3], "sys");  //mem -> disk (ref)
    mkdir_from_descriptor(&root_descriptor, &subdirs[4], "usr");  //mem -> disk (ref)
    mkdir_from_descriptor(&root_descriptor, &subdirs[5], "conf"); //mem -> disk (ref)
    mkdir_from_descriptor(&root_descriptor, &subdirs[6], "dev");  //mem -> disk (ref)
}

//warning: init as directory (1 block size and obj_type = lfs_directory)
void init_descriptor(struct LFSObjectDescriptor *str, char *name, uint32_t first_block, unsigned char permissions, enum LFSObjectType type) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char null = 0x00;
    
    for (int i = 0; i < 32; i++) {
        memcpy(str -> name + i, &null, 1);
    }
    
    strcpy(str -> name, name);

    if (type == lfs_directory) {
        str -> size = 1;
    } else {
        str -> size = 0;
    }

    str -> bsize = 0; //4 byte
    str -> first_block = first_block; //4 byte

    //8 byte creation full date
    str -> creation.date.d = tm.tm_mday; //1 byte
    str -> creation.date.m = tm.tm_mon + 1; //1 byte
    str -> creation.date.y = tm.tm_year + 1900; //2 byte
    str -> creation.time.h = tm.tm_hour; //1 byte
    str -> creation.time.m = tm.tm_min; //1 byte
    str -> creation.time.s = tm.tm_sec; //1 byte
    str -> creation.time.h24 = 1; //1 byte

    //8 byte last edit full date
    str -> last_edit.date.d = tm.tm_mday;
    str -> last_edit.date.m = tm.tm_mon + 1;
    str -> last_edit.date.y = tm.tm_year + 1900;
    str -> last_edit.time.h = tm.tm_hour;
    str -> last_edit.time.m = tm.tm_min;
    str -> last_edit.time.s = tm.tm_sec;
    str -> last_edit.time.h24 = 1;

    //8 byte last opened full date
    str -> last_opened.date.d = tm.tm_mday;
    str -> last_opened.date.m = tm.tm_mon + 1;
    str -> last_opened.date.y = tm.tm_year + 1900;
    str -> last_opened.time.h = tm.tm_hour;
    str -> last_opened.time.m = tm.tm_min;
    str -> last_opened.time.s = tm.tm_sec;
    str -> last_opened.time.h24 = 1;

    str -> permissions = permissions; //1 byte
    str -> obj_type = type; //4 byte

    str -> location.block = (unsigned int) 0;
    str -> location.offset = (unsigned short) 0;
    str -> parent.block = (unsigned int) 0;
    str -> parent.offset = (unsigned int) 0;

    str -> flags.fl_valid = FLAG_UP;
    str -> flags.fl_edited = FLAG_DOWN;
    str -> flags.fl_locked = FLAG_DOWN;
    str -> flags.fl_hidden = FLAG_DOWN;
    str -> flags.fl_root = FLAG_DOWN;
    str -> flags.fl_corrupted = FLAG_DOWN;
    str -> flags.fl_orphan = FLAG_DOWN;

    for (int i = 0; i < 2; i++) {
        str -> padding[i] = 0x00;
    }
}

int write_block(struct LFSBlock *block, const unsigned long block_n) {
    fseek(file, 512 * block_n, SEEK_SET); //move to the right offset

    if (fwrite(&(block -> data), BLOCK_DATA_LENGTH, 1, file) != 1) {
        append_error_trace("fwrite failure", "write_block");
        return 0;
    }

    fseek(file, 512 * block_n + BLOCK_DATA_LENGTH, SEEK_SET);

    reverse_endianess(&(block -> prev), 4); //to big
    if (fwrite(&(block -> prev), 4, 1, file) != 1) {
        append_error_trace("fwrite failure", "write_block");
        return 0;
    }

    fseek(file, 512 * block_n + 508, SEEK_SET);

    reverse_endianess(&(block -> next), 4); //to big
    if (fwrite(&(block -> next), 4, 1, file) != 1) {
        append_error_trace("fwrite failure", "write_block");
        return 0;
    }

    reverse_endianess(&(block -> prev), 4); //back to little
    reverse_endianess(&(block -> next), 4); //back to little

    return 1;
}

//returns 1 or 0
int read_block(struct LFSBlock *block, const unsigned long block_n) {
    if (block_n < 0) {
        append_error_trace("offset exceed bound", "read_block");
        return 0;
    }

    fseek(file, 512 * block_n, SEEK_SET);
    if (fread(block, 512, 1, file) != 1) {
        append_error_trace("fread failure", "read_block");
        return 0;
    }

    reverse_endianess(&(block -> prev), 4); //to little
    reverse_endianess(&(block -> next), 4); //to little

    return 1;
}

//check whether the block is already used or not
int is_block_free(const unsigned long n) {
    struct LFSBlock block;

    if (!read_block(&block, n)) {
        return 0;
    }

    if (block.prev == 0x0 && block.next == 0x0) {
        return 1;
    } else {
        return 0;
    }
}

long find_free_block() {
    struct LFSBlock tmp;

    //search from fs first block to the last
    for (long i = FS_FIRST_BLOCK; i < FS_END_BLOCK; i++) {
        if (is_block_free(i)) {
            return i;
        }
    }

    printf("WARNING: no more blocks remaining\n");
    return -1;
}

int reserve_block(const unsigned int block_n) {
    int end = END_OF_CHAIN_PTR;
    unsigned char zero = 0x00;

    if (is_block_free(block_n)) {
        //reset the block (for safety)
        fseek(file, 512 * block_n, SEEK_SET);
        for (int i = 0; i < BLOCK_DATA_LENGTH; i++) {
            fwrite(&zero, 1, 1, file);
            fseek(file, 1, SEEK_CUR);
        }

        fseek(file, 512 * block_n + BLOCK_DATA_LENGTH + 4, SEEK_SET); //moves to the right position
        return fwrite(&end, 4, 1, file) == 1;
    } else {
        return 0;
    }
}

void free_block(const unsigned int block_n) {
    int void_ptr = 0;
    fseek(file, 512 * block_n + BLOCK_DATA_LENGTH, SEEK_SET); //moves to the right position
    fwrite(&void_ptr, 4, 1, file);
    fseek(file, 4, SEEK_CUR);
    fwrite(&void_ptr, 4, 1, file);
}

int get_last_block(struct LFSObjectDescriptor *descr) {
    unsigned int last;
    struct LFSBlock tmp;
    tmp.next = descr -> first_block;

    for (int i = 0; i < descr -> size - 1; i++) {
        if (!read_block(&tmp, tmp.next)) {
            append_error_trace("error reading block", "get_last_block");
            return -1;
        }
    }

    if (tmp.next < 0) {
        append_error_trace("invalid last block number", "get_last_block");
    }

    return tmp.next; //ALREADY IN LITTLE ENDIAN
}

//returns the free space at the end of an object
int get_free_space(struct LFSObjectDescriptor *descr) {
    unsigned int blocks_number = descr -> size; //get the number of blocks
    unsigned int size_bytes = descr -> bsize; //get the size of the object in bytes
    int result = BLOCK_DATA_LENGTH * blocks_number - size_bytes;

    if (result >= 0) {
        return result;
    } else {
        append_error_trace("invalid free space", "get_free_space");
        return -1;
    }
}

void init_block(struct LFSBlock *block, unsigned int prev, unsigned int next) {
    long old_ptr = ftell(file); //save the current file position

    for (int i = 0; i < BLOCK_DATA_LENGTH; i++) {
        block -> data[i] = 0x00;
    }

    block -> prev = prev;
    block -> next = next;

    fseek(file, old_ptr, SEEK_SET);
}

void write_block_data(unsigned int block, void *data, unsigned int size) {
    fseek(file, 512 * block, SEEK_SET); //move to the beginning of the block
    fwrite(data, size, 1, file); //write size bytes of data
}

//update the descriptor on the disk specified by the descr parameter (this function takes the location from the parameter)
int update_descriptor(struct LFSObjectDescriptor *descr) {
    int block = descr -> location.block;
    int offset = descr -> location.offset;

    reverse_descriptor_endianess(descr);

    if (offset <= BLOCK_DATA_LENGTH - LFS_OBJECT_DESCRIPTOR_LENGTH) {
        fseek(file, 512 * block + offset, SEEK_SET);

        if (fwrite(descr, LFS_OBJECT_DESCRIPTOR_LENGTH, 1, file) != 1) {
            append_error_trace("fwrite failure", "update_descriptor");
            return 0;
        }
    } else {
        int block1 = BLOCK_DATA_LENGTH - offset;
        int block2 = LFS_OBJECT_DESCRIPTOR_LENGTH - block1;
        int next_block;

        fseek(file, 512 * block + BLOCK_DATA_LENGTH + 4, SEEK_SET);
        fread(&next_block, 4, 1, file);
        reverse_endianess(&next_block, 4); //to little

        if (next_block <= 0 || next_block >= END_OF_CHAIN_PTR) {
            append_error_trace("invalid next block number", "update_descriptor");
            return 0;
        }

        fseek(file, 512 * block + offset, SEEK_SET);
        if (fwrite(descr, block1, 1, file) != 1) {
            append_error_trace("fwrite failure, block 1", "update_descriptor");
            return 0;
        }

        fseek(file, 512 * next_block, SEEK_SET);
        if (fwrite(descr + block1, block2, 1, file) != 1) {
            append_error_trace("fwrite failure, block 2", "update_descriptor");
            return 0;
        }
    }

    return 1;
}

int add_block(struct LFSObjectDescriptor *str) {
    int last_block = get_last_block(str);
    struct LFSBlock block;
    
    if (last_block == -1) {
        append_error_trace("invalid last block number", "add_block");
        return -1;
    }

    int new_block = find_free_block();
    if (new_block == -1) {
        append_error_trace("invalid new block number", "add_block");
        return -1;
    }

    for (int i = 0; i < BLOCK_DATA_LENGTH; i++) {
        block.data[i] = 0x00;
    }

    //set the old block's next pointer to the new block
    fseek(file, 512 * last_block + BLOCK_DATA_LENGTH + 4, SEEK_SET);
    reverse_endianess(&new_block, 4); //to big
    fwrite(&new_block, 4, 1, file);
    reverse_endianess(&new_block, 4); //back to little

    init_block(&block, last_block, END_OF_CHAIN_PTR);

    if (!write_block(&block, new_block)) {
        append_error_trace("error writing new block", "add_block");
        return -1;
    }

    (str -> size)++; //update the number of blocks of the object descriptor
    return new_block;
}

//return -1 if error, else the next block's number
int get_next_block(int block) {
    struct LFSBlock tmp;

    if (!read_block(&tmp, block)) {
        append_error_trace("error reading block", "get_next_block");
        return -1;
    }

    if (tmp.next < 0) {
        append_error_trace("invalid next block number", "get_next_block");
    }

    return tmp.next;
}

//return -1 if error, else the previous block's number
int get_previous_block(int block) {
    struct LFSBlock tmp;

    if (!read_block(&tmp, block)) {
    append_error_trace("error reading block", "get_previous_block");
        return -1;
    }

    if (tmp.prev < 0) {
        append_error_trace("invalid previous block number", "get_previous_block");
    }

    return tmp.prev;
}

//read the whole object described by the LFSObjectDescriptor scanning all the blocks chain
int read_object(void *buf, struct LFSObjectDescriptor *start) {
    unsigned int count = start -> size; //object's blocks number
    struct LFSBlock block; //structure in which save the blocks one by one
    unsigned long bytes = 0;
    int block_n = start -> first_block;

    for (int i = 0; i < count; i++) {
        if (block_n == END_OF_CHAIN_PTR) {
            printf("Warning: premature end of chain for object %s (%d %d)\n", start -> name, start -> location.block, start -> location.offset);
            break;
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

//write the data into a set of linked blocks, the size (in bytes) of the data to be written is located in size parameter
int write_object(void *data, const unsigned long size, const unsigned long start_block, struct LFSObjectDescriptor *descr) {
    const unsigned int blocks_number = (unsigned long) ceil((double) size / (double) BLOCK_DATA_LENGTH);
    unsigned int blocks[blocks_number];
    struct LFSBlock tmp;

    //if the desired first block is already filled, quit
    if (is_block_free(start_block)) {
        blocks[0] = start_block;
        reserve_block(start_block);
        descr -> first_block = start_block;
    } else {
        append_error_trace("first block not free", "write_object");
        return 0;
    }

    //else reserve the blocks to write in
    for (int i = 1; i < blocks_number; i++) {
        blocks[i] = find_free_block();

        if (!reserve_block(blocks[i])) {
            for (int j = 0; j < i; j++) {
                free_block(j);
            }

            append_error_trace("error reserving blocks", "write_object");
            return 0;
        }

        //if there's no more free blocks quit
        if (blocks[i] == -1) {
            for (int j = 0; j < i; j++) {
                free_block(j);
            }

            append_error_trace("no more free blocks", "write_object");
            return 0;
        }
    }
    
    //write the blocks
    unsigned long tmp_size = size;
    for (int i = 0; i < blocks_number; i++) {
        init_block(&tmp, 0, 0);

        if (tmp_size >= BLOCK_DATA_LENGTH) {
            memcpy(&(tmp.data), data + BLOCK_DATA_LENGTH * i, BLOCK_DATA_LENGTH);
            tmp_size -= BLOCK_DATA_LENGTH;
        } else {
            //should be executed one time only
            memcpy(&(tmp.data), data + BLOCK_DATA_LENGTH * i, tmp_size);
            tmp_size = 0;
        }

        //if this is the first block put the previous block number to the current block itself
        if (i == 0) {
            tmp.prev = blocks[i];
        } else {
            tmp.prev = blocks[i - 1];
        }

        //if this is the last block put the next block number to END_OF_CHAIN_PTR
        if (i == blocks_number - 1) {
            tmp.next = END_OF_CHAIN_PTR;
        } else {
            tmp.next = blocks[i + 1];
        }

        write_block(&tmp, blocks[i]);
    }

    descr -> size = blocks_number;
    descr -> bsize = size;

    return 1;
}

//needed to add a subdirectory
int add_data_to_object(struct LFSObjectDescriptor *str, void *data, unsigned long size) {
    unsigned long final_size = size; //save the number of bytes
    int free_space = get_free_space(str); //calculate how many bytes can be written in the last block of the structure

    fseek(file, 512 * get_last_block(str) + BLOCK_DATA_LENGTH - free_space, SEEK_SET); //move to the right position of the object - disk -> mem (ref)

    if (size > free_space) {
        if (free_space != 0) {
            //write the data that fits in the free space of the object
            if (fwrite(data, free_space, 1, file) != 1) {
                append_error_trace("fwrite failure", "add_data_to_object");
                return 0;
            }

            size -= free_space; //subtract to the total number of blocks to be written the numbers of byte just written
            data += free_space; //move the data pointer free_space bytes after
        }

        int needed_blocks = ceil((float) size / (float) BLOCK_DATA_LENGTH); //calculate the number of blocks needed to contain the remaining data
        int blocks[needed_blocks];

        //adds new blocks to the chain (the block's pointers will be automatically set) and save the blocks number into an array
        for (int i = 0; i < needed_blocks; i++) {
            blocks[i] = add_block(str);

            //if there's an error while adding new blocks, free all the already reserved blocks
            if (blocks[i] == -1) {
                for (int j = 0; j < i; j++) {
                    free_block(blocks[j]);
                    (str -> size)--;
                }

                append_error_trace("error adding new block", "add_data_to_object");
                return 0;
            }
        }

        //write the data into the new blocks
        for (int i = 0; i < needed_blocks; i++) {
            if (size > BLOCK_DATA_LENGTH) {
                write_block_data(blocks[i], data, BLOCK_DATA_LENGTH);
                size -= BLOCK_DATA_LENGTH;
                data += BLOCK_DATA_LENGTH;
            } else {
                write_block_data(blocks[i], data, size);
                data += size;
                size -= size;
            }
        }
    } else {
        fwrite(data, size, 1, file);
    }

    str -> bsize += final_size;
    update_size(str);
    update_bsize(str);

    return 1;
}

//delete all the blocks chain of the object (saves the descriptor)
int delete_object_data(struct LFSObjectDescriptor *object) {
    int current_block = object -> first_block;
    int next_block;

    if (current_block == 0 || current_block == END_OF_CHAIN_PTR) {
        append_error_trace("invalid current block number", "delete_object_data");
        return 0;
    }

    void *zero = malloc(512);
    if (!zero) {
        append_error_trace("buffer error", "delete_object_data");
        return 0;
    }

    do {
        fseek(file, 512 * current_block + BLOCK_DATA_LENGTH + 4, SEEK_SET);
        fread(&next_block, 4, 1, file);
        reverse_endianess(&next_block, 4); //to little

        fseek(file, 512 * current_block, SEEK_SET);
        fwrite(zero, 512, 1, file);

        current_block = next_block;
    } while (next_block != END_OF_CHAIN_PTR);

    return 1;
}

//pass the descriptor with the updated size value
void update_size(struct LFSObjectDescriptor *str) {
    fseek(file, 512 * str -> location.block, SEEK_SET);
    fseek(file, str -> location.offset, SEEK_CUR);
    fseek(file, 32, SEEK_CUR);
    reverse_endianess(&(str -> size), 4); //to big
    fwrite(&(str -> size), 4, 1, file);
    reverse_endianess(&(str -> size), 4); //back to little
}

//pass the descriptor with the updated bsize value
void update_bsize(struct LFSObjectDescriptor *str) {
    fseek(file, 512 * str -> location.block, SEEK_SET);
    fseek(file, str -> location.offset, SEEK_CUR);
    fseek(file, 36, SEEK_CUR);
    reverse_endianess(&(str -> bsize), 4); //to big
    fwrite(&(str -> bsize), 4, 1, file);
    reverse_endianess(&(str -> bsize), 4); //back to little
}

int read_raw_block(unsigned long block_n, void *buf) {
    if (block_n * 512 > file_info.st_size || block_n < 0) {
        append_error_trace("offset exceed file size or less than zero", "read_raw_block");
        return 0;
    }

    fseek(file, 512 * block_n, SEEK_SET);
    if (fread(buf, 512, 1, file) != 1) {
        append_error_trace("fread failure", "read_raw_block");
        return 0;
    }

    return 1;
}

int descriptor_from_location(unsigned int block, unsigned short offset, struct LFSObjectDescriptor *descr) {
    fseek(file, 512 * block + offset, SEEK_SET);

    if (offset <= BLOCK_DATA_LENGTH - LFS_OBJECT_DESCRIPTOR_LENGTH) {
        if (fread(descr, LFS_OBJECT_DESCRIPTOR_LENGTH, 1, file) != 1) {
            append_error_trace("fread failure", "descriptor_from_location");
            return 0;
        }
    } else {
        if (fread(descr, BLOCK_DATA_LENGTH - offset, 1, file) != 1) {
            append_error_trace("fread failure", "descriptor_from_location");
            return 0;
        }

        descr += BLOCK_DATA_LENGTH - offset;

        int next_block;
        fseek(file, 512 * block + BLOCK_DATA_LENGTH + 4, SEEK_SET);
        fread(&next_block, 4, 1, file);
        reverse_endianess(&next_block, 4); //to little

        if (next_block == END_OF_CHAIN_PTR) {
            append_error_trace("invalid next block number", "descriptor_from_location");
            return 0;
        }

        fseek(file, 512 * next_block, SEEK_SET);
        if (fread(descr, LFS_OBJECT_DESCRIPTOR_LENGTH - (BLOCK_DATA_LENGTH - offset), 1, file) != 1) {
            append_error_trace("fread failure", "descriptor_from_location");
            return 0;
        }
    }

    reverse_descriptor_endianess(descr);
    if (strcmp(descr -> name, "") == 0) {
        return 0;
    }

    return 1;
}

int create_object(struct LFSObjectDescriptor *parent, struct LFSObjectDescriptor *obj, char *name) {
    int first_block = find_free_block();

    if (first_block == -1) {
        append_error_trace("no more free blocks", "create_object");
        return 0;
    }

    reserve_block(first_block);
    init_descriptor(obj, name, first_block, STANDARD_PERMISSIONS, lfs_directory);

    //calculate the position at which the descriptor will be write
    int location_block = get_last_block(parent);
    short location_offset = BLOCK_DATA_LENGTH - get_free_space(parent);
    obj -> location.block = location_block;
    obj -> location.offset = location_offset;
    obj -> parent.block = parent -> location.block;
    obj -> parent.offset = parent -> location.offset;

    if (location_block < 0 || location_offset < 0) {
        append_error_trace("invalid location", "create_object");
        return 0;
    }

    if (location_offset == BLOCK_DATA_LENGTH) {
        obj -> location.block = find_free_block();
        obj -> location.offset = 0;
    }

    return 1;
}

//create a subdirectory in the directory specified by the descriptor, automatically update the descriptor on the disk and memory
int mkdir_from_descriptor(struct LFSObjectDescriptor *parent, struct LFSObjectDescriptor *subdir, char *name) {
    if (!create_object(parent, subdir, name)) {
        append_error_trace("error creating basic object", "mkdir_from_descriptor");
        return 0;
    }

    subdir -> obj_type = lfs_directory;
    reverse_descriptor_endianess(subdir);

    if (!add_data_to_object(parent, subdir, LFS_OBJECT_DESCRIPTOR_LENGTH)) {
        free_block(subdir -> first_block); //if failed, free the previously reserved block
        append_error_trace("error adding new data to parent", "mkdir_from_descriptor");
        return 0;
    }

    reverse_descriptor_endianess(subdir);

    return 1;
}

//read all the directory
int read_dir_from_descriptor(struct LFSObjectDescriptor *dir, struct LFSObjectDescriptor array[]) {
    int n_descriptors = dir -> bsize / LFS_OBJECT_DESCRIPTOR_LENGTH; //calculate the number of descriptor contained in this directory

    if (dir -> obj_type != lfs_directory) {
        append_error_trace("descriptor not a directory", "read_dir_from_descriptor");
        return -1;
    }

    void *buffer = malloc(dir -> bsize); //allocate a buffer that can hold the exact number of bytes
    if (!buffer) {
        append_error_trace("buffer error", "read_dir_from_descriptor");
        return -1;
    }

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

int remove_descriptor(struct LFSObjectDescriptor *target) {
    struct LFSObjectDescriptor parent;
    int *zero = malloc(LFS_OBJECT_DESCRIPTOR_LENGTH);
    int target_index = -1;

    if (!zero) {
        append_error_trace("buffer error", "remove_descriptor");
        return 0;
    }

    //get the parent
    if (!descriptor_from_location(target -> parent.block, target -> parent.offset, &parent)) {
        append_error_trace("error getting target's parent", "remove_descriptor");
        return 0;
    }

    if (strcmp(parent.name, "") == 0) {
        parent.flags.fl_corrupted = FLAG_UP;
        append_error_trace("parent null", "remove_descriptor");
        return 0;
    }

    int subdirs_n = parent.bsize / LFS_OBJECT_DESCRIPTOR_LENGTH; //get the number of subdirectories

    //read the subdirectories (reads the parent)
    struct LFSObjectDescriptor subdirs[subdirs_n]; //warning: false struct size (96)(8 bytes in excess for every struct)
    void *new_data = malloc(parent.bsize); //buffer to be written on the file

    if (!new_data) {
        append_error_trace("buffer error", "remove_descriptor");
        return 0;
    }

    if (read_dir_from_descriptor(&parent, subdirs) == -1) {
        append_error_trace("error reading parent's subdirs", "remove_descriptor");
        return 0;
    }

    //search the right one and delete it
    for (int i = 0; i < subdirs_n; i++) {
        if (strcmp(subdirs[i].name, target -> name) == 0) {
            memcpy(&subdirs[i], zero, LFS_OBJECT_DESCRIPTOR_LENGTH);
            target_index = i;
            break;
        }
    }

    if (target_index == -1) {
        //the searched subdirectory doesn't exist
        append_error_trace("target name not found (probably a corrupted or not updated target descriptor)", "remove_descriptor");
        return 0;
    }

    int offset = LFS_OBJECT_DESCRIPTOR_LENGTH * target_index; //starting offset of the zeroed area
    int leftover = parent.bsize - offset - LFS_OBJECT_DESCRIPTOR_LENGTH; //calculate the number of bytes left

    //paste the subdirectories into the buffer
    for (int i = 0; i < subdirs_n; i++) {
        reverse_descriptor_endianess(&subdirs[i]); //to big (i think)
        memcpy(new_data + LFS_OBJECT_DESCRIPTOR_LENGTH * i, &subdirs[i], LFS_OBJECT_DESCRIPTOR_LENGTH);
    }

    memmove(new_data + offset, new_data + offset + LFS_OBJECT_DESCRIPTOR_LENGTH, leftover);

    //write the object (delete parent's data and rewrite it) and reload the descriptor pointers
    if (!delete_object_data(&parent)) {
        append_error_trace("error deleting parent's data", "remove_descriptor");
        return 0;
    }

    //with this function, new blocks will be allocated for this object
    if (write_object(new_data, parent.bsize - LFS_OBJECT_DESCRIPTOR_LENGTH, parent.first_block, &parent) == 0) {
        append_error_trace("error writing new parent's data", "remove_descriptor");
        return 0;
    }

    //reload the pointers
    int last_block = parent.first_block; //at this point is little endian
    short last_offset = 0;
    struct LFSPointer tmp;

    for (int i = 0; i < subdirs_n - 1; i++) {
        //move to the right field of the right descriptor
        fseek(file, 512 * last_block + last_offset + 76, SEEK_SET);

        tmp.block = last_block;
        tmp.offset = last_offset;
        reverse_endianess(&tmp.block, 4);
        reverse_endianess(&tmp.offset, 2);

        //write the new location
        if (fwrite(&tmp, 8, 1, file) != 1) {
            append_error_trace("fwrite failure", "remove_descriptor");
            return 0;
        }

        if (last_offset >= BLOCK_DATA_LENGTH - LFS_OBJECT_DESCRIPTOR_LENGTH) { //the next descriptor will be splitted into two blocks
            last_block = get_next_block(last_block);
            last_offset = LFS_OBJECT_DESCRIPTOR_LENGTH - (BLOCK_DATA_LENGTH - last_offset);
        } else {
            last_offset += LFS_OBJECT_DESCRIPTOR_LENGTH;
        }
    }

    //update the parent descriptor (the one in central memory is already up to date)(warning: it may be in two blocks, check before writing)
    if (!update_descriptor(&parent)) {
        append_error_trace("error updating parent's descriptor", "remove_descriptor");
        return 0;
    }

    return 1;
}

int delete_object(struct LFSObjectDescriptor *target) {
    if (target -> obj_type == lfs_directory && target -> bsize > 0) {
        //must delete all the target subdirectories (if there's any)
        int subdirs_n = target -> bsize / LFS_OBJECT_DESCRIPTOR_LENGTH;
        struct LFSObjectDescriptor subdirs[subdirs_n];

        if (!read_dir_from_descriptor(target, subdirs)) {
            append_error_trace("Recursive directory deleting error: subdirectory reading error", "delete_object");
            return 0;
        }

        for (int i = 0; i < subdirs_n; i++) {
            delete_object(&subdirs[i]);
        }
    } else {
        if (!delete_object_data(target)) {
            append_error_trace("error deleting target's data", "delete_object");
            return 0;
        }
    }

    if (!remove_descriptor(target)) {
        append_error_trace("error removing target descriptor", "delete_object");
        return 0;
    }

    return 1;
}

int migrate_from_descriptor(struct LFSObjectDescriptor *src, struct LFSObjectDescriptor *dest) {
    if (dest -> obj_type != lfs_directory) {
        append_error_trace("destination not a directory", "migrate_from_descriptor");
        return 0;
    }

    //remove the target descriptor from the old location
    if (!remove_descriptor(src)) {
        append_error_trace("error removing target descriptor", "migrate_from_descriptor");
        return 0;
    }

    //update target's pointers
    int block = get_last_block(dest);
    int offset = BLOCK_DATA_LENGTH - get_free_space(dest);

    if (block == -1 || offset < 0) {
        append_error_trace("error calculating the new location pointer of the target object", "migrate_from_descriptor");
        return 0;
    }

    src -> parent.block = dest -> location.block;
    src -> parent.offset = dest -> location.offset;
    src -> location.block = block;
    src -> location.offset = offset;

    reverse_descriptor_endianess(src);

    //add the target descriptor to the new location
    if (!add_data_to_object(dest, src, LFS_OBJECT_DESCRIPTOR_LENGTH)) {
        append_error_trace("error moving target descriptor to the new parent", "migrate_from_descriptor");
        return 0;
    }

    return 1;
}

int mkfile(struct LFSObjectDescriptor *parent, struct LFSObjectDescriptor *file, char *name, void *data, unsigned long size) {
    if (!create_object(parent, file, name)) {
        append_error_trace("error creating basic object", "mkfile");
        return 0;
    }

    file -> obj_type = lfs_file;

    if (!add_data_to_object(file, data, size)) {
        append_error_trace("error writing file's data", "mkfile");
        return 0;
    }

    reverse_descriptor_endianess(file);

    if (!add_data_to_object(parent, file, LFS_OBJECT_DESCRIPTOR_LENGTH)) {
        free_block(file -> first_block); //if failed, free the previously reserved block
        append_error_trace("error adding new data to parent", "mkfile");
        return 0;
    }

    reverse_descriptor_endianess(file);
    return 1;
}

int mkdir_from_path(char *path, char *name, struct LFSObjectDescriptor *child) {
    struct LFSObjectDescriptor descr;
    char *total = malloc(strlen(path) + strlen(name) + 1);
    memcpy(total, path, strlen(path));
    strcat(total, name);

    if (scan_directory(total, NULL)) {
        append_error_trace("directory already existing", "mkdir_from_path");
        return 0;
    }

    if (!scan_directory(path, &descr)) {
        append_error_trace("error scanning through directories", "mkdir_from_path");
        return 0;
    }

    if (!mkdir_from_descriptor(&descr, child, name)) {
        append_error_trace("error making new directory, mkdir_from_descriptor failed", "mkdir_from_path");
        return 0;
    }

    return 1;
}

int rmdir_from_path(char *path) {
    struct LFSObjectDescriptor target;

    if (!scan_directory(path, &target)) {
        append_error_trace("error scanning through directories", "rmdir_from_path");
        return 0;
    }

    if (!delete_object(&target)) {
        append_error_trace("error deleting target", "rmdir_from_path");
        return 0;
    }

    return 1;
}

//scan the directory tree and return the descriptor of the wanted one
int scan_directory(char *path, struct LFSObjectDescriptor *des) {
    int path_length = strlen(path);
    if (path_length <= 0) {
        append_error_trace("invalid path length", "scan_directory");
        return 0;
    }

    if (strcmp(path, "/") == 0) {
        memcpy(des, &root_descriptor, LFS_OBJECT_DESCRIPTOR_LENGTH);
        return 1;
    }

    const char sep[2] = "/";
    char str[path_length];
    char *token;

    strcpy(str, path); //copy the path into str (the string must be editable)

    //read root directory first
    int subdirs_n = root_descriptor.bsize / LFS_OBJECT_DESCRIPTOR_LENGTH; //number of subdirectories in the current directory (starting with root directory)
    struct LFSObjectDescriptor subdirs[100]; //array of descriptors (100 max subdirectories)(to improve)
    struct LFSObjectDescriptor current; //current directory's descriptor

    if (read_dir_from_descriptor(&root_descriptor, subdirs) == -1) { //disk -> mem (ref)
        append_error_trace("starting subdirectory (root) reading error", "scan_directory");
        return 0;
    }
   
    token = strtok(str, sep);
    int found = 0;

    while (token != NULL) {
        //scan all the subdirectories name
        for (int i = 0; i < subdirs_n; i++) {
            //if found, read the new subdirectory and set subdirs_n to the number of descriptor in that directory
            if (strcmp(subdirs[i].name, token) == 0) {
                memcpy(&current, &subdirs[i], LFS_OBJECT_DESCRIPTOR_LENGTH); //copy the right descriptor into current
                subdirs_n = current.bsize / LFS_OBJECT_DESCRIPTOR_LENGTH; //update the number of descriptor

                //read the new directory
                if (current.obj_type == lfs_directory) {
                    if (read_dir_from_descriptor(&current, subdirs) == -1) { //disk -> mem (ref)
                        append_error_trace("subdirectory reading error", "scan_directory");
                        return 0;
                    }
                }

                found = 1;
                break;
            }
        }

        if (!found) {
            append_error_trace("subdirectory not found", "scan_directory");
            return 0;
        }

        found = 0;
        token = strtok(NULL, sep);
    }

    //the cycle ends only if the directory exists
    memcpy(des, &current, LFS_OBJECT_DESCRIPTOR_LENGTH);
    return 1;
}

int directory_exist_descr(struct LFSObjectDescriptor dir) {
    if (dir.obj_type != lfs_directory) {
        append_error_trace("not a directory", "directory_exist_descr");
        return 0;
    }

    if (is_root(dir)) {
        return 1;
    }

    struct LFSObjectDescriptor parent;
    if (!descriptor_from_location(dir.parent.block, dir.parent.offset, &parent)) {
        append_error_trace("parent retrieval error", "directory_exist_descr");
        return 0;
    }

    if (is_root(parent)) {
        return 1;
    } else {
        return directory_exist_descr(parent);
    }
}

int are_equal(struct LFSObjectDescriptor obj1, struct LFSObjectDescriptor obj2) {
    if ((obj1.location.block == obj2.location.block) && (obj1.location.offset == obj2.location.offset)) {
        if (strcmp(obj1.name, obj2.name) == 0) {
            return 1; //objects are equal
        } else {
            return -1; //objects are in the same location but with different names
        }
    } else {
        return 0; //objects are different
    }
}

void print_directory(struct LFSObjectDescriptor dir) {
    int subdirs_n = dir.bsize / LFS_OBJECT_DESCRIPTOR_LENGTH;
    struct LFSObjectDescriptor subdirs[subdirs_n];

    if (subdirs_n == 0) {
        printf("%s is empty\n", dir.name);
        return;
    }

    if (read_dir_from_descriptor(&dir, subdirs) == -1) {
        print_error_trace();
        return;
    }

    printf("Files & directories in %s:\n", dir.name);

    for (int i = 0; i < subdirs_n; i++) {
        print_object_data(subdirs[i]);
    }
}

void validate_object_name(char *str) {
    int corrections = 0;

    //apply corrections
    for (int i = 0; i < strlen(str); i++) {
        switch(*(str + i)) {
            case ' ':
            case '\"':
            case '\'':
            case '/':
                *(str + i) = '_';
                corrections = 1;
            break;
        }
    }

    if (corrections) {
        printf("Some corrections has been made, the new name is %s\n", str);
    }
}

char is_valid(struct LFSObjectDescriptor obj) {
    return obj.flags.fl_valid & 0x1;
}

char is_edited(struct LFSObjectDescriptor obj) {
    return obj.flags.fl_valid & 0x1;
}

char is_hidden(struct LFSObjectDescriptor obj) {
    return obj.flags.fl_hidden & 0x1;
}

char is_locked(struct LFSObjectDescriptor obj) {
    return obj.flags.fl_locked & 0x1;
}

char is_root(struct LFSObjectDescriptor obj) {
    return obj.flags.fl_root & 0x1;
}

char is_corrupted(struct LFSObjectDescriptor obj) {
    return obj.flags.fl_corrupted & 0x1;
}

char is_orphan(struct LFSObjectDescriptor obj) {
    return obj.flags.fl_orphan & 0x1;
}