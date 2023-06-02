#include "virtual_memory.h"

#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <stdio.h>
#endif

void vm_init(int subs_alg_type, int total_memory_size, int page_size){

    #ifdef PRINT_DEBUG
    printf("Initial params: %d %d %d\n", subs_alg_type, total_memory_size, page_size);
    #endif

    return;
}

void vm_write(int addr){

    #ifdef PRINT_DEBUG
    printf("Writing at %d\n", addr);
    #endif

    return;
}

void vm_read(int addr){

    #ifdef PRINT_DEBUG
    printf("Reading from %d\n", addr);
    #endif

    return;
}

void vm_print_report(){

    #ifdef PRINT_DEBUG
    printf("Printing report...\n");
    #endif

    return;
}
