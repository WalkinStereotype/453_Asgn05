#include <stdint.h>
#include <stdio.h>

#define DIR_ENTRY_SIZE 64
#define MAX_NAME_SIZE 60

typedef struct Dir_Entry {
    uint32_t inode;
    unsigned char name[MAX_NAME_SIZE];
} Dir_Entry;


