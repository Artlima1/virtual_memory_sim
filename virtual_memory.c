#include "virtual_memory.h"

#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <stdio.h>
#endif

#define PAGE_ADDR(addr) addr>>mem_info.s

typedef struct {
    int page_size;
    int mem_size;

    int s;

    int dirty_pages;
    int page_faults;
    int tot_mem_reqs;

} memory_info_t;

static memory_info_t mem_info;

static int calc_s(int page_size){
    int tmp = page_size;
    int s = 0;
    while (tmp>1) {
        tmp = tmp>>1;
        s++;
    }
    return s;
}

void vm_init(int subs_alg_type, int total_memory_size, int page_size){

    #ifdef PRINT_DEBUG
    printf("Initial params: %d %d %d\n", subs_alg_type, total_memory_size, page_size);
    #endif

    mem_info.mem_size = total_memory_size;
    mem_info.page_size = page_size;
    mem_info.s = calc_s(page_size);

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
