#pragma once
#include <include/types.h>
#define __RAMFS_PREFERRED_SIZE_IN_PAGES 2048 //1 MByte
#define __RAMFS_FALLBACK_SIZE_IN_PAGES 1024  //512 KByte
#define __RAMFS_MIN_SIZE_IN_PAGES 512  //256 KByte

bool init_ramfs();