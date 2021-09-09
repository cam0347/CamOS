#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "partition_tools.h"

#define DEFAULT_DISK_PATH "disk.iso"
#define SECTOR_SIZE 512
#define SYSTEM_CLEAR_STATEMENT "clear"
#define PARTITION_TABLE_LBA 1
#define TABLE_OFFSET SECTOR_SIZE * PARTITION_TABLE_LBA

void print_menu();
int get_user_input();
void switch_choice(int choice);
int add_partition();
int remove_partition();
int edit_partition();
int check_table();
int get_choice();
void erase_table();
void initialize_table();
int read_table(struct pt_entry table[]);
void reverse_endianess(void *ptr, unsigned int length);
void print_table();

FILE *disk;
long disk_size;
char *path;
int entry_number; //compiled by check_table() function
struct pt_entry entries[32];
const int TABLE_MAGIC_NUMBER = 187;

int main(int argc, char *argv[]) {
    //system(SYSTEM_CLEAR_STATEMENT);
    path = malloc(1024);

    if (!path) {
        printf("Buffer error\n");
        return 0;
    }

    strcpy(path, DEFAULT_DISK_PATH);

    if ((disk = fopen(path, "r+")) == NULL) {
        printf("Disk path: ");
        scanf("%s", path);

        if ((disk = fopen(path, "r+")) == NULL) {
            printf("Disk file not found: %s\n", path);
            return 0;
        }
    }

    struct stat file_inode;
    if (stat(path, &file_inode) == -1) {
        printf("Error getting file inode\n");
        return 0;
    }

    disk_size = (long) file_inode.st_size;
    printf("Disk size: %ld (%ld sectors)\n", disk_size, disk_size / SECTOR_SIZE);

    if (!check_table()) {
        printf("Error verifying table's integrity\n");
        return 0;
    }

    if (argc > 1) {
        //the program has been launched by the master script
        if (strcmp(argv[1], "master") == 0) {
            struct pt_entry entry;
            entry.start_sector = 1000;
            entry.end_sector = 10000;
            entry.pt_type = leofs;
            entry.flags = 0x81; //bootable and present
            entry.magic = 0xB0;

            reverse_endianess(&entry.pt_type, 4);
            reverse_endianess(&entry.start_sector, 4);
            reverse_endianess(&entry.end_sector, 4);

            fseek(disk, SECTOR_SIZE * PARTITION_TABLE_LBA, SEEK_SET);
            fwrite(&entry, 16, 1, disk);
            fseek(disk, SECTOR_SIZE * PARTITION_TABLE_LBA + 16, SEEK_SET);
            fwrite(&entry, 16, 1, disk);
        }

        printf("done");
    } else {
        print_menu();
        switch_choice(get_user_input());
    }

    fclose(disk);
    free(path);

    return 0;
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

int get_choice() {
    char *ans = malloc(2);
    printf("Proceed? [y/n]: ");
    scanf("%s", ans);

    return *ans == 'y' || *ans == 'Y';
}

//count entries number
int check_table() {
    if ((entry_number = read_table(entries)) == -1) {
        return 0;
    } else {
        printf("I found %d partitions\n", entry_number);
    }

    return 1;
}

//reads the table and puts the entries into table[], returns the number of entries or -1 in case of error
int read_table(struct pt_entry table[]) {
    int i = 0, end = 0;
    struct pt_entry tmp;

    while(!end) {
        fseek(disk, TABLE_OFFSET + i * 16, SEEK_SET);

        if (fread(&tmp, 16, 1, disk) != 1) {
            return -1;
        }

        if (tmp.magic == 0x00 || i == 32) {
            end = 1;
        } else {
            //to little
            reverse_endianess(&tmp.pt_type, 4);
            reverse_endianess(&tmp.start_sector, 4);
            reverse_endianess(&tmp.end_sector, 4);
            memcpy(&table[i], &tmp, 16);
            i++;
        }
    }

    return i;
}

void print_table() {
    char *pt_type_text = malloc(10);

    if (!pt_type_text) {
        printf("Buffer error\n");
        return;
    }

    for (int i = 0; i < entry_number; i++) {
        switch(entries[i].pt_type) {
            case 0: strcpy(pt_type_text, "raw"); break;
            case 1: strcpy(pt_type_text, "swap"); break;
            case 2: strcpy(pt_type_text, "LeoFS"); break;
            case 3: strcpy(pt_type_text, "NTFS"); break;
            case 4: strcpy(pt_type_text, "fat16"); break;
            case 5: strcpy(pt_type_text, "fat32"); break;
            case 6: strcpy(pt_type_text, "zfs"); break;
            case 7: strcpy(pt_type_text, "apfs"); break;
            case 8: strcpy(pt_type_text, "nfs"); break;
        }
        
        printf("Partition number %3d: %7s %5d %5d %2d 0x%2x\n", i, pt_type_text, entries[i].start_sector, entries[i].end_sector, entries[i].flags, entries[i].magic);
    }
}

void print_menu() {
    printf("0: exit\n");
    printf("1: add new partition\n");
    printf("2: remove partition\n");
    printf("3: edit partition\n");
    printf("4: print partition table\n");
    printf("5: erase partition system (reset disk)\n");
}

int get_user_input() {
    int choice;

    do {
        printf("Type the choice: ");
        scanf("%d", &choice);
    } while (choice < 0 || choice > 5);

    return choice;
}

void switch_choice(int choice) {
    switch(choice) {
        case 0:
            break;

        case 1:
            if (add_partition()) {
                printf("Partition added\n");
            } else {
                printf("There was an error adding the partition entry into ptb\n");
            }
            break;

        case 2:
            if (remove_partition()) {
                printf("Partition removed\n");
            } else {
                printf("There was an error removing the partition entry\n");
            }
            break;

        case 3:
            if (edit_partition()) {
                printf("Partition edited succesfully\n");
            } else {
                printf("There was an error editing the partition entry\n");
            }
            break;

        case 4:
            print_table();
            break;

        case 5:
            erase_table();
            break;

        default:
            printf("Invalid choice number\n");
            break;
    }
}

int add_partition() {
    unsigned int pt_type_int;
    unsigned long start_sector, end_sector;
    unsigned int flags[8];
    enum partition_type pt_type;
    struct pt_entry entry;

    if (entry_number == 32) {
        printf("No more space into ptb\n");
        return 0;
    }

    entry.flags = 0x00;

    printf("Partition type [0 ~ 8]: ");
    scanf("%d", &pt_type_int);

    if (pt_type_int < 0 || pt_type_int > 8) {
        printf("Invalid partition type\n");
        return 0;
    }

    printf("Partition start sector: ");
    scanf("%ld", &start_sector);

    if (start_sector > 0x100000000) {
        printf("Partition number exceed bound (1099511627776)\n");
        return 0;
    }

    printf("Partition end sector: ");
    scanf("%ld", &end_sector);

    if (end_sector > 0x100000000) {
        printf("Partition number exceed bound (1099511627776)\n");
        return 0;
    }

    if (start_sector >= end_sector) {
        printf("The last sector must be greater than start sector\n");
        return 0;
    }

    for (int i = 0; i < entry_number; i++) {
        if ((start_sector >= entries[i].start_sector && start_sector <= entries[i].end_sector) || (end_sector >= entries[i].start_sector && end_sector <= entries[i].end_sector)) {
            printf("Warning: partition overlaps another one\n");
            return 0;
        }
    }

    printf("Partition is bootable [0 - 1]: ");
    scanf("%d", &flags[0]);

    printf("Partition is swap [0 - 1]: ");
    scanf("%d", &flags[1]);

    printf("Partition is raw [0 - 1]: ");
    scanf("%d", &flags[2]);

    printf("Partition is damaged [0 - 1]: ");
    scanf("%d", &flags[3]);

    printf("Partition is hidden [0 - 1]: ");
    scanf("%d", &flags[4]);

    printf("Partition is locked [0 - 1]: ");
    scanf("%d", &flags[5]);

    printf("Partition is raid [0 - 1]: ");
    scanf("%d", &flags[6]);

    printf("Partition is present [0 - 1]: ");
    scanf("%d", &flags[7]);

    entry.pt_type = pt_type_int;
    entry.start_sector = start_sector;
    entry.end_sector = end_sector;

    for (int i = 0; i < 8; i++) {
        entry.flags |= ((unsigned char) flags[i] << (7 - i));
    }

    entry.magic = (char)((pt_type_int * 4 + start_sector * 3 + end_sector * 2 + entry.flags + 1) % 255);
    printf("Magic: 0x%x\n", (int) entry.magic);

    //to big
    reverse_endianess(&entry.pt_type, 4);
    reverse_endianess(&entry.start_sector, 4);
    reverse_endianess(&entry.end_sector, 4);

    fseek(disk, TABLE_OFFSET + entry_number * 16, SEEK_SET);
    if (fwrite(&entry, 16, 1, disk) != 1) {
        return 0;
    }

    entry_number++; //update the number of entries
    memcpy(&entries[entry_number - 1], &entry, 16); //update the entries array

    return 1;
}

int remove_partition() {
    int n;

    if (entry_number <= 0) {
        printf("The table is empty\n");
        return 0;
    }

    print_table();
    printf("Which partition would you like to remove [0 ~ %d]? ", entry_number - 1);
    scanf("%d", &n);

    if (n < 0 || n > entry_number - 1) {
        printf("Invalid partition index\n");
        return 0;
    }

    void *buffer = malloc(16);
    void *zero = calloc(1, 16);

    if (!buffer || !zero) {
        printf("Buffer error\n");
        return 0;
    }

    for (int i = n; i < entry_number - 1; i++) {
        //copy the next entry
        fseek(disk, TABLE_OFFSET + 16 * (i + 1), SEEK_SET);
        if (fread(buffer, 16, 1, disk) != 1) {
            return 0;
        }

        //paste in the previous location
        fseek(disk, TABLE_OFFSET + 16 * i, SEEK_SET);
        if (fwrite(buffer, 16, 1, disk) != 1) {
            return 0;
        }

        memmove(&entries[i], &entries[i + 1], 16);
    }

    fseek(disk, TABLE_OFFSET + 16 * (entry_number - 1), SEEK_SET);
    fwrite(zero, 16, 1, disk);
    entry_number -= 1;

    free(buffer);
    free(zero);

    return 1;
}

int edit_partition() {
    int n, property;

    if (entry_number <= 0) {
        printf("The table is empty\n");
        return 0;
    }

    print_table();
    printf("Which partition would you like to edit [0 ~ %d]? ", entry_number - 1);
    scanf("%d", &n);

    if (n < 0 || n > entry_number - 1) {
        printf("Invalid partition index\n");
        return 0;
    }

    printf("0: partition type\n");
    printf("1: start block\n");
    printf("2: end block\n");
    printf("3: flags\n");

    printf("Which property would you like to edit [0 ~ 3]? ");
    scanf("%d", &property);

    if (property < 0 || property > 3) {
        printf("Invalid property number\n");
        return 0;
    }

    struct pt_entry updated_entry;
    fseek(disk, TABLE_OFFSET + 16 * n, SEEK_SET);
    fread(&updated_entry, 16, 1, disk);

    switch(property) {
        case 0: {
            int pt_type;
            printf("New partition type [0 ~ 8]: ");
            scanf("%d", &pt_type);

            if (pt_type < 0 || pt_type > 8) {
                printf("Invalid partition type\n");
                return 0;
            }

            reverse_endianess(&pt_type, 4);
            updated_entry.pt_type = (unsigned int) pt_type;
        } break;

        case 1: {
            int start_block;
            printf("New start block: ");
            scanf("%d", &start_block);

            if (start_block < 0 || start_block >= updated_entry.end_sector) {
                printf("Invalid start block\n");
                return 0;
            }

            reverse_endianess(&start_block, 4);
            updated_entry.start_sector = (unsigned int) start_block;
        } break;

        case 2: {
            int end_block;
            printf("New end block: ");
            scanf("%d", &end_block);

            if (end_block < 0 || end_block <= updated_entry.start_sector) {
                printf("Invalid end block\n");
                return 0;
            }

            reverse_endianess(&end_block, 4);
            updated_entry.end_sector = (unsigned int) end_block;
        } break;

        case 3: {
            int flags[8];
            unsigned char flags_total;

            printf("Partition is bootable [0 - 1]: ");
            scanf("%d", &flags[0]);

            printf("Partition is swap [0 - 1]: ");
            scanf("%d", &flags[1]);

            printf("Partition is raw [0 - 1]: ");
            scanf("%d", &flags[2]);

            printf("Partition is damaged [0 - 1]: ");
            scanf("%d", &flags[3]);

            printf("Partition is hidden [0 - 1]: ");
            scanf("%d", &flags[4]);

            printf("Partition is locked [0 - 1]: ");
            scanf("%d", &flags[5]);

            printf("Partition is raid [0 - 1]: ");
            scanf("%d", &flags[6]);

            printf("Partition is present [0 - 1]: ");
            scanf("%d", &flags[7]);

            for (int i = 0; i < 8; i++) {
                flags_total |= ((unsigned char) flags[i] << (7 - i));
            }

            updated_entry.flags = (unsigned char) flags_total;
        } break;
    }

    char new_magic = (char)((updated_entry.pt_type * 4 + updated_entry.start_sector * 3 + updated_entry.end_sector * 2 + updated_entry.flags + 1) % 255);
    updated_entry.magic = new_magic;
    fseek(disk, TABLE_OFFSET + 16 * n, SEEK_SET);
    fwrite(&updated_entry, 16, 1, disk);
    
    return 1;
}

void erase_table() {
    if (entry_number > 0) {
        char choice[2];
        printf("There are %d partitions on this disk\n", entry_number);

        if (!get_choice()) {
            printf("Operation suspended\n");
            return;
        }
    }

    void *zero = calloc(1, 512);

    if (!zero) {
        printf("Buffer error\n");
        return;
    }

    fseek(disk, TABLE_OFFSET, SEEK_SET);
    fwrite(zero, 512, 1, disk);
    printf("Table erased\n");
}
