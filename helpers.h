#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
/* Constants (masks switched to hex) */
#define FILE_TYPE_MASK 0xF000
#define REG_FILE 0x8000
#define DIR_FILE 0x4000
#define U_READ 0x0100 
#define U_WRITE 0x0080 
#define U_EXEC 0x0040 
#define G_READ 0x0020 
#define G_WRITE 0x0010 
#define G_EXEC 0x0008 
#define O_READ 0x0004 
#define O_WRITE 0x0002 
#define O_EXEC 0x0001 


#define LEN_PERMS_LIST 11 /* dir? + 9 perms + null */
#define INODE_MAX_DIGITS 9
#define FILE_MAX_CHARS 60

void modeToPerms(unsigned char *perms, uint16_t mode);
void printFileDetails(
            uint16_t mode, 
            uint32_t size, 
            unsigned char name[]);
