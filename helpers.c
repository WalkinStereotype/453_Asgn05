#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>

#include "dirEntry.h"
#include "inode.h"
#include "parTable.h"
#include "superblock.h"


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
#define SIZE_MAX_DIGITS 9
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
            uint32_t size, 
            unsigned char name[]){
    
    unsigned char *perms = 
        (unsigned char *) malloc(
                        sizeof(unsigned char) * LEN_PERMS_LIST
                        );
    
    modeToPerms(perms, mode);
  
    printf("%s %*d %s\n", perms, SIZE_MAX_DIGITS, size, name);

    free(perms);

}

/*
Verbose details
*/
void printVerboseDetails(Superblock sb, Inode inode){
    unsigned char *perms = 
        (unsigned char *) malloc(sizeof(unsigned char) * LEN_PERMS_LIST);
    struct tm *time= (struct tm *) malloc(sizeof(struct tm));



    modeToPerms(perms, inode.mode);

    printf("\nSuperblock Contents\n");
    printf("Stored Fields:\n");

    printf("\tninodes %*d\n", 12, sb.ninodes);
    printf("\ti_blocks %*d\n", 11, sb.i_blocks);
    printf("\tz_blocks %*d\n", 11, sb.z_blocks);
    printf("\tfirstdata %*d\n", 10, sb.firstdata);

    printf("\tlog_zone_size %*d", 6, sb.log_zone_size);
    printf(" (zone size: %d)\n", (sb.blocksize << sb.log_zone_size));

    printf("\tmax_file %*d\n", 11, sb.max_file);
    printf("\tmagic         0x%04x\n", sb.magic);
    printf("\tzones %*d\n", 19, sb.zones);
    printf("\tblocksize %*d\n", 10, sb.blocksize);
    printf("\tsubversion %*d\n", 9, sb.subversion);
    printf("\n");

    printf("File inode:\n");
    printf("\tuint16_t mode             0x%04x ", inode.mode);
    printf("(%s)\n", perms);
    printf("uint16_t links %*d\n", 16, inode.links);
    printf("uint16_t uid   %*d\n", 16, inode.uid);
    printf("uint16_t size  %*d\n", 16, inode.size);
    printf("uint16_t atime %*d --- ", 16, inode.uid);
    printf("\n"); /* TODO */
    printf("uint16_t mtime %*d --- ", 16, inode.uid);
    printf("\n");
    printf("uint16_t ctime %*d --- ", 16, inode.uid);
    printf("\n");

    printf("\n");

    printf("             zone[0]   = %*d\n", 8, inode.zone[0]);
    printf("             zone[1]   = %*d\n", 8, inode.zone[1]);
    printf("             zone[2]   = %*d\n", 8, inode.zone[2]);
    printf("             zone[3]   = %*d\n", 8, inode.zone[3]);
    printf("             zone[4]   = %*d\n", 8, inode.zone[4]);
    printf("             zone[5]   = %*d\n", 8, inode.zone[5]);
    printf("             zone[6]   = %*d\n", 8, inode.zone[6]);
    printf("uint32_t     indirect    %*d\n", 8, inode.indirect);
    printf("uint32_t     double      %*d\n", 8, inode.two_indirect);


    free(perms);
    free(time);
}


void readFile(
        FILE *fs, 
        Inode inode, 
        uint32_t zonesize,
        uint32_t blocksize, 
        uint32_t firstZoneAddr
    ){
    
    /* buffer for reading by zone */
    unsigned char *buffer = (unsigned char *) malloc(zonesize);

    /* amount of size left to read */
    uint32_t sizeLeftToRead = inode.size;

    /* amount just read */
    size_t sizeRead = -1;

    /* number of zone numbers per indirect block */
    uint32_t zonesPerBlock = zonesize / sizeof(uint32_t);

    /* iterator */
    int i;

    /* Read each direct zone */
    for(i = 0; i < DIRECT_ZONES; i++){

        /* Break if done */
        if(sizeLeftToRead <= 0){
            break;
        }
        /* Skip zone if hole */
        if(inode.zone[i] == 0){
            continue;
        }

        /* Seek to position and read into buffer */
        fseek(
                fs, 
                firstZoneAddr + (inode.zone[i] * zonesize), 
                SEEK_SET
            );
        fread(&buffer, zonesize, 1, fs);

        /* set sizeRead depending on what is less */
        if (sizeLeftToRead < zonesize){
            sizeRead = sizeLeftToRead;
        } else {
            sizeRead = zonesize;
        }

        /* Write from buffer */
        fwrite(buffer, 1, sizeRead, stdout);

        /* Update size left */
        sizeLeftToRead -= sizeRead;
    }

    /* For indirect block */
    if (sizeLeftToRead && inode.indirect){
        /* Malloc indirect block */
        uint32_t *indirectBlock = (uint32_t *) malloc(blocksize);

        /* Obtain and read zone numbers from indirect block */
        fseek(
                fs, 
                firstZoneAddr + (inode.indirect * zonesize), 
                SEEK_SET
            );
        
        fread(indirectBlock, zonesize, 1, fs);

        /* iterate through every zone in indirect */
        for(i = 0; i < zonesPerBlock; i++){
            uint32_t currZone = indirectBlock[i];

            /* Break if done */
            if(sizeLeftToRead <= 0){
                break;
            }
            /* Skip zone if hole */
            if(inode.zone[i] == 0){
                continue;
            }

            /* Seek to position and read into buffer */
            fseek(
                    fs, 
                    firstZoneAddr + (currZone * zonesize), 
                    SEEK_SET
                );
            fread(&buffer, zonesize, 1, fs);

            /* set sizeRead depending on what is less */
            if (sizeLeftToRead < zonesize){
                sizeRead = sizeLeftToRead;
            } else {
                sizeRead = zonesize;
            }

            /* Write from buffer */
            fwrite(buffer, 1, sizeRead, stdout);

            /* Update size left */
            sizeLeftToRead -= sizeRead;
        }

        free(indirectBlock);

    }

    // /* For two_indirect block */
    // if (sizeLeftToRead && inode.two_indirect){
    //     /* Malloc for twoIndirect and Direct */
    //     uint32_t *twoIndirectBlock = (uint32_t *) malloc(blocksize);
    //     uint32_t *indirectBlock = (uint32_t *) malloc(blocksize);

    //     /* Obtain and read zone numbers from indirect block */
    //     fseek(
    //             fs, 
    //             firstZoneAddr + (inode.indirect * zonesize), 
    //             SEEK_SET
    //         );
        
    //     fread(indirectBlock, zonesize, 1, fs);
    // }

    free(buffer);
}

// int main(int argc, char *argv[]){
//     uint16_t mode = 
//         DIR_FILE | U_READ | U_WRITE 
//         | U_EXEC | G_READ | G_WRITE 
//         | G_EXEC | O_READ | O_WRITE | O_EXEC;
    
//     unsigned char name[] = "Hello. world";

//     printFileDetails(mode, 12341, name);
// }
