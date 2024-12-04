#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* CONSTANTS */
#define TABLE_ADDR 0x1BE
#define TABLE_SIZE 16
#define BOOTABLE_MAGIC_NUM 0x80
#define MINIX_MAGIC_NUM 0x81


/* Struct for partition table entry */
typedef struct Par_Table {
    uint8_t bootind;     
    uint8_t start_head;  
    uint8_t start_sec;
    uint8_t start_cyl;
    uint8_t type; /* IMPORTANT */
    uint8_t end_head;
    uint8_t end_sec;
    uint8_t end_cyl;
    uint32_t lFirst; /* IMPORTANT */
    uint32_t size; /* IMPORTANT */
} Par_Table;