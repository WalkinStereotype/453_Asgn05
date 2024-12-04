#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* CONSTANTS */
#define INODE_SIZE 64
#define INODE_START_BLOCK(sb) ((2 + sb.i_blocks + sb.z_blocks) * sb.blocksize)

#define DIRECT_ZONES 7


/* Struct for inode */
typedef struct Inode {
    uint16_t mode;
    uint16_t links;
    uint16_t uid;
    uint16_t gid;
    uint32_t size;
    int32_t atime;
    int32_t mtime;
    int32_t ctime;
    uint32_t zone[DIRECT_ZONES];
    uint32_t indirect;
    uint32_t two_indirect;
    uint32_t unused;
} Inode;