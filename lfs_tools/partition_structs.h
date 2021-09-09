enum partition_type {
    raw = 0,
    swap = 1,
    leofs = 2,
    ntfs = 3,
    fat16 = 4,
    fat32 = 5,
    zfs = 6,
    apfs = 7,
    nfs = 8
};

//tot. 16 bytes
struct pt_entry {
    unsigned int pt_type;           //4 byte
    unsigned int start_sector;      //4 byte
    unsigned int end_sector;        //4 byte
    unsigned char flags;            //1 byte
    unsigned char magic;            //1 byte
    char padding[2];                //2 byte
};