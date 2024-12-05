#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>

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

/*takes in partition and subpartion number and returns the offset to the start of the used partition*/
int get_start(FILE *image_file, int partition, int subpartition){
    int start;
    if(partition == -1){
        return 0;
    }
    //get partition table
    struct Par_Table par_table;
    fseek(image_file, TABLE_ADDR + (partition * TABLE_SIZE), SEEK_SET);
    fread(&par_table, sizeof(par_table), 1, image_file);
    if(par_table.type != MINIX_MAGIC_NUM || par_table.bootind != BOOTABLE_MAGIC_NUM){
        perror("Bad Magic Number. \n");
        return EXIT_FAILURE;
    }
    //check for valid signature
    //0x55 in byte 510
    //0xAA in byte 511
    printf("partable lfirst %d\n", par_table.lFirst);
    printf("parsize: %d\n", par_table.size);
    start = par_table.lFirst;
    //check subpartion
    if(subpartition == -1){
        return start;
    } 
    struct Par_Table sub_par_table;
    fseek(image_file, TABLE_ADDR + (partition * TABLE_SIZE) + start, SEEK_SET);
    fread(&sub_par_table, sizeof(sub_par_table), 1, image_file);
    if(par_table.type != MINIX_MAGIC_NUM || sub_par_table.bootind != BOOTABLE_MAGIC_NUM){
        perror("Bad Magic Number. \n");
        return EXIT_FAILURE;
    }
    return start;
}

void read_inode(FILE *image_file, struct Superblock superblock, uint32_t inode_offset, Dir_Entry dir_entry, char *path){
    struct Inode sub_inode;
    char *rest_of_path = malloc(64);
    char *next_dir = malloc(64);
    strcpy(next_dir, path);
    strcpy(rest_of_path, path);
    // printf("path: %s\n", path);
    
    if (next_dir[0] == '/'){
        next_dir += 1;
    }
    if (strlen(next_dir) + 1 == strlen(path)){
        rest_of_path[0] = '\0';
    }else {
        rest_of_path += strlen(next_dir) + 1;
    }
    // printf("nextdir: %s\n", next_dir);
    // printf("rest: %s\n", rest_of_path);
    
    fseek(image_file, inode_offset, SEEK_SET);
    fread(&sub_inode, sizeof(sub_inode), 1, image_file);
    if(path == ""){
        // printf("case1\n");
        printFileDetails(sub_inode.mode, sub_inode.size, dir_entry.name);
    }else if(strcmp(next_dir, dir_entry.name) && strlen(rest_of_path) > 0){
        //directory on the path
        // printf("case2\n");
        get_directory_files(image_file, sub_inode, superblock, rest_of_path);

    }else if(strcmp(next_dir, dir_entry.name) && strlen(rest_of_path) == 0){
        // printf("case3\n");
        get_directory_files(image_file, sub_inode, superblock, "");
    }else{
        // printf("case4");
        // printf("direntry name: %s\nnext_dir: %s\n", dir_entry.name, next_dir);
        // printf("len1 %d\nlen2 %d\n", strlen(next_dir), strlen(dir_entry.name));
    }

}

/* gets the files that a directory contains
   enable printing by setting printing to 1/ON */
void get_directory_files(FILE *image_file, struct Inode inode, struct Superblock superblock, char *path){
    int i, j;
    uint32_t zone_number;
    uint32_t zone_offset;
    uint32_t zone_size;
    uint32_t inode_start;
    uint32_t inode_offset;
    uint32_t entries;
    uint32_t current_loc;

    for(i = 0; i < DIRECT_ZONES; i ++){
        zone_number = inode.zone[i];
        if(zone_number == 0){
            // printf("\nzone: %d\n", zone_number);
            //unused block
            continue;
        }
        
        // printf("\nzone: %d\n", zone_number);
        //get zone offset
        zone_size = superblock.blocksize << superblock.log_zone_size;
        zone_offset = (zone_number *  zone_size);
        // printf("superblcok first data: %d\n\n", superblock.firstdata);
        fseek(image_file, zone_offset, SEEK_SET);

        struct Dir_Entry dir_entry;
        // printf("inode size: %d\n", inode.size);
        entries = inode.size / INODE_SIZE;
        
        current_loc = ftell(image_file);
        for(j = 0; j < entries; j++){
            fseek(image_file, current_loc, SEEK_SET);
            current_loc += sizeof(dir_entry);
            if(fread(&dir_entry, sizeof(dir_entry), 1, image_file) == 0){
                continue;
            }
            if(dir_entry.inode == 0){
                continue;
            }

            
            inode_start = INODE_START_BLOCK(superblock);
            inode_offset = ((dir_entry.inode - 1) * INODE_SIZE) + inode_start;
            //read inode
            read_inode(image_file, superblock, inode_offset, dir_entry, path);
        }
    }
}

/* returns the inode number*/
// int 

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

