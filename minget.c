#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>

#include "dirEntry.h"
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
    char *src = NULL;
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
        if (optind < argc){
            src = argv[optind++];
            if (optind < argc){
                dest = argv[optind++];
            }
        } else {
            printf("Error: no src path given \n");
            return EXIT_FAILURE;
        }
    } else {
        printf("Error: no image given\n");
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
    printf("Source: %s\n", src);
    if (dest) {
        printf("Destination: %s\n", dest);
    } else {
        printf("Destination: (none)\n");
    }

    // Your processing logic here...

    return EXIT_SUCCESS;
}


