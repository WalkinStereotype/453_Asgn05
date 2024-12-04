#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>

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

/*
uint16_t octToDec(uint16_t oct) {
    int num = 0;
    int base = 1;
    int currDig;

    while (oct > 0){
        currDig = oct % 10; 
        num += currDig * base; 
        base *= 8; 
        oct /= 10; 
    }

    return num;
}
*/


/* 
Takes in a mode and puts the permissions
(including whether it is a directory,
file or neither) into perms
*/
void modeToPerms(unsigned char *perms, uint16_t mode){
    int fileType = mode & FILE_TYPE_MASK;
    int iter;
    
    for(iter = 0; iter < LEN_PERMS_LIST - 1; iter++){
        perms[iter] = '-';
    }

    perms[10] = '\0';


    if(fileType == DIR_FILE){
        perms[0] = 'd';
    } else if (fileType != REG_FILE){
        /* ERROR */
    }

    
    if ((mode & U_READ) == U_READ) {
        perms[1] = 'r';
    }
    if ((mode & U_WRITE) == U_WRITE) {
        perms[2] = 'w';
    }
    if ((mode & U_EXEC) == U_EXEC) {
        perms[3] = 'x';
    }
    if ((mode & G_READ) == G_READ) {
        perms[4] = 'r';
    }
    if ((mode & G_WRITE) == G_WRITE) {
        perms[5] = 'w';
    }
    if ((mode & G_EXEC) == G_EXEC) {
        perms[6] = 'x';
    }
    if ((mode & O_READ) == O_READ) {
        perms[7] = 'r';
    }
    if ((mode & O_WRITE) == O_WRITE) {
        perms[8] = 'w';
    }
    if ((mode & O_EXEC) == O_EXEC) {
        perms[9] = 'x';
    }
}

/* 
Format file fields
*/
void printFileDetails(
            uint16_t mode, 
            uint32_t inode, 
            unsigned char name[]){
    
    unsigned char *perms = 
        (unsigned char *) malloc(
                        sizeof(unsigned char) * FILE_MAX_CHARS
                        );
    
    modeToPerms(perms, mode);

    printf("%s %*d %s\n", perms, INODE_MAX_DIGITS, mode, name);

}

int main(int argc, char *argv[]){
    uint16_t mode = 
        DIR_FILE | U_READ | U_WRITE 
        | U_EXEC | G_READ | G_WRITE 
        | G_EXEC | O_READ | O_WRITE | O_EXEC;
    
    unsigned char name[] = "Hello. world";

    printFileDetails(mode, 12341, name);
}



