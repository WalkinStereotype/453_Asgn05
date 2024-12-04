#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>

#include "dirHelp.h"
#include "inode.h"
#include "parTable.h"
#include "superblock.h"



int isValidPartition(char *num){
    char *currChar;

    if (!num || *num == '\0'){
        return 0;
    }

    for(currChar = num; *currChar != '\0'; currChar++){
        if(!isdigit(*currChar)){
            return 0;
        }
    }

    return 1;
}

int main(int argc, char *argv[]){
    int opt;
    int verbose = 0;
    int partition = -1;  
    int subpartition = -1;
    char *image = NULL;
    char *dest = NULL;

    // Parse flags and options
    while ((opt = getopt(argc, argv, "vp:s:")) != -1){
        switch (opt) {
            case 'v':
                verbose = 1;
                break;
            case 'p':
                if (isValidPartition(optarg)){
                    partition = atoi(optarg);
                } else {
                    printf("Invalid partition arg\n");
                    return EXIT_FAILURE;
                }

                break;
            case 's':
                if (isValidPartition(optarg)){
                    subpartition = atoi(optarg);
                } else {
                    printf("Invalid subpartition arg\n");
                    return EXIT_FAILURE;
                }
                break;
            default:
                return EXIT_FAILURE;
        }
    }

    // Remaining non-flag arguments
    if (optind < argc) {
        image = argv[optind++];
        if (optind < argc) {
            dest = argv[optind++];
        }
    } else {
        printf("usage: minls [ -v ] [ -p num [ -s num ] ] imagefile [path ]\n"
                "Options:\n"
                "-p part --- select partition for filesystem (default: none)\n"
                "-s sub --- select subpartition for filesystem (default: none)\n"
                "-h help --- print usage information and exit\n"
                "-v verbose --- increase verbosity level\n");
        return EXIT_FAILURE;
    }

    // Check if mandatory arguments are provided
    if (!image) {
        printf("Error: no image given\n");
        return EXIT_FAILURE;
    }

    // Output parsed arguments for debugging
    printf("Verbose mode: %s\n", verbose ? "ON" : "OFF");
    printf("P argument: %d\n", partition);
    printf("S argument: %d\n", subpartition);
    printf("Image: %s\n", image);

    // processing logic here
    FILE *image_file = fopen(image, "rb");
    if (!image_file){
        perror("Invalid file\n");
        printf("Error code: %d\n", errno);
        return EXIT_FAILURE;
    }
    struct Superblock superblock;
    fseek(image_file, SUPERBLOCK_ADDR, SEEK_SET);
    fread(&superblock, sizeof(superblock), 1, image_file);
    // printf(superblock.magic);
    if (superblock.magic != MINIX_MAGIC){
        printf("Invalid superblock magic number");
        return EXIT_FAILURE;
    }
    
    printf("valid");
    

    return EXIT_SUCCESS;
}

