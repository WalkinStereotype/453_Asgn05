#include <stdint.h>
#include <stdio.h>

#define DIR_ENTRY_SIZE 64

typedef struct Dir_Entry {
    uint32_t inode;
    char name[60];
} Dir_Entry;


