/* --------------------------------------- Includes --------------------------------------------------------- */
#include "virtual_memory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --------------------------------------- Macros ----------------------------------------------------------- */
#define PAGE_INDEX(addr) (addr >> mem_info.offset_bits)
#define GET_OFFSET(addr) (addr & (~(~1<<mem_info.offset_bits)))

/* --------------------------------------- Type Definitions ------------------------------------------------- */
typedef struct lru_node_t lru_node_t;
typedef struct {
    int subs_alg_type;

    int page_size;
    int offset_bits;
    int page_table_size;

    int phy_mem_size;
    int phy_mem_num_of_pages;
    int phy_mem_filled_pages;

    int dirty_pages;
    int page_faults;
    int tot_mem_reqs;

} memory_info_t;

typedef struct {
    int phy_page;
    unsigned int validity : 1;
    unsigned int dirty : 1;
    unsigned int ref_bit : 1;
} page_table_info_t;

struct lru_node_t{
    unsigned page_index;
    lru_node_t *next;
    lru_node_t *prev;
};

/* --------------------------------------- Local Variables -------------------------------------------------- */
static memory_info_t mem_info;
static page_table_info_t * page_table;
static int (*get_victim)();
static int fifo_first_in_index = 0;
static unsigned time = 1;
static bool list_is_full = false;
static bool debug_logs;
static lru_node_t *lru_pages;
static lru_node_t *lru_head = NULL;
static lru_node_t *lru_tail = NULL;

/* --------------------------------------- Local Function Declaration --------------------------------------------- */
static int calc_offset_bits(int page_size);
static void page_fault(int page_index);

static void print_lru_list();
static int lru_subs_alg();
static int second_chance_subs_alg();
static int fifo_subs_alg();
static int random_subs_alg();

static void swap_node_to_head(lru_node_t *node);
static void swap_in(int disk_page_index, int phy_page_index);
static void swap_out(int disk_page_index, int phy_page_index);

/* --------------------------------------- External Function Implementation ---------------------------------- */
void vm_init(int subs_alg_type, int total_memory_size, int page_size, bool debug){

    mem_info.subs_alg_type = subs_alg_type;
    switch (subs_alg_type)
    {
        case SUBS_ALG_LRU:
            get_victim = &lru_subs_alg;
            break;
        case SUBS_ALG_2ND:
            get_victim = &second_chance_subs_alg;
            break;
        case SUBS_ALG_FIFO:
            get_victim = &fifo_subs_alg;
            break;
        case SUBS_ALG_RAND:
            get_victim = &random_subs_alg;
            break;
    }

    mem_info.page_size = page_size*1024;
    mem_info.offset_bits = calc_offset_bits(page_size*1024);
    mem_info.page_table_size = (1 << (32 - mem_info.offset_bits+2))-1;

    mem_info.phy_mem_size = total_memory_size*1024;
    mem_info.phy_mem_num_of_pages = mem_info.phy_mem_size / mem_info.page_size;
    mem_info.phy_mem_filled_pages = 0;

    debug_logs = debug;

    if (debug_logs)
        printf("Mem info:\n\tAlg: %d\n\tPage_size: %d\n\tOffset Bits: %d\n\tPage table size: %d\n\tPhy_mem_size: %d\n\tPhy_mem_num_of_pages: %d\n", 
            mem_info.subs_alg_type,
            mem_info.page_size,
            mem_info.offset_bits,
            mem_info.page_table_size,
            mem_info.phy_mem_size,
            mem_info.phy_mem_num_of_pages
        );

    page_table = (page_table_info_t *) malloc(mem_info.page_table_size * sizeof(page_table_info_t));
    memset(page_table, 0, (mem_info.page_table_size * sizeof(page_table_info_t)));

    if(subs_alg_type == SUBS_ALG_LRU){
        lru_pages = (lru_node_t *) malloc(mem_info.phy_mem_num_of_pages * sizeof(lru_node_t));
        memset(lru_pages, 0, (mem_info.phy_mem_num_of_pages * sizeof(lru_node_t)));
    }

    return;
}

void vm_write(unsigned addr){
    unsigned page_index = PAGE_INDEX(addr);

    if (debug_logs)
        printf("[%u] Req to write to address %8x, on page %d\n", time, addr, page_index);

    if(page_table[page_index].validity==0){
        page_fault(page_index);
        vm_write(addr);
    }
    else {
        /* Write at addres page_table[page_index].phy_page + offset bits */
        // int phy_addr = (page_table[page_index].phy_page << mem_info.offset_bits) | GET_OFFSET(addr);
        
        if (debug_logs)
            printf("\tNo page fault, write to phy page %d\n\n", page_table[page_index].phy_page);

        page_table[page_index].ref_bit = 1;
        page_table[page_index].dirty = 1;
        mem_info.tot_mem_reqs++;

        if (mem_info.subs_alg_type == SUBS_ALG_LRU) {
            lru_node_t *node = &lru_pages[page_table[page_index].phy_page];
            node->page_index = page_index; 
            swap_node_to_head(node);
        }
    }
    return;
}

void vm_read(unsigned addr){
    unsigned page_index = PAGE_INDEX(addr);

    if (debug_logs)
        printf("[%u] Req to read from address %8x, on page %d\n", time, addr, page_index);

    if(page_table[page_index].validity==0){
        page_fault(page_index);
        vm_read(addr);
    }
    else {
        /* Read from addres page_table[page_index].phy_page + offset bits */
        // int phy_addr = (page_table[page_index].phy_page << mem_info.offset_bits) | GET_OFFSET(addr);
        
        if (debug_logs)
            printf("\tNo page fault, read from phy page %d\n\n", page_table[page_index].phy_page);

        page_table[page_index].ref_bit = 1;
        mem_info.tot_mem_reqs++;

        if (mem_info.subs_alg_type == SUBS_ALG_LRU) {
            lru_node_t *node = &lru_pages[page_table[page_index].phy_page];
            node->page_index = page_index; 
            swap_node_to_head(node);
        }
    }

    return;
}

void vm_print_report(){

    if (debug_logs)
        printf("Printing report...\n");

    int mem_size = mem_info.phy_mem_size/1024;
    int page_size = mem_info.page_size/1024;
    char victim_choosing_method[10];
    switch (mem_info.subs_alg_type)
    {
        case SUBS_ALG_LRU:
            strcpy(victim_choosing_method, "lru");
            break;
        case SUBS_ALG_2ND:
            strcpy(victim_choosing_method, "2a");
            break;
        case SUBS_ALG_FIFO:
            strcpy(victim_choosing_method, "fifo");
            break;
        case SUBS_ALG_RAND:
            strcpy(victim_choosing_method, "random");
            break;
    }

    printf("Tamanho da memoria: %d KB\nTamanho das paginas: %d KB\nTecnica de reposicao: %s\nPaginas lidas: %d\nPaginas escritas: %d\n", 
        mem_size, 
        page_size,
        victim_choosing_method,
        mem_info.page_faults,
        mem_info.dirty_pages
    );

    return;
}

/* --------------------------------------- Internal Function Implementation ---------------------------------- */
static int calc_offset_bits(int page_size){
    int tmp = page_size;
    int s = 0;
    while (tmp>1) {
        tmp = tmp>>1;
        s++;
    }
    return s;
}

static void page_fault(int page_index){
    
    mem_info.page_faults++;

    if(mem_info.phy_mem_filled_pages < mem_info.phy_mem_num_of_pages){
        if (debug_logs)
            printf("\tFree page available at %d. Assigning to %d\n", mem_info.phy_mem_filled_pages, page_index);

        swap_in(page_index, mem_info.phy_mem_filled_pages);
        page_table[page_index].phy_page = mem_info.phy_mem_filled_pages;
        page_table[page_index].validity = 1;
        mem_info.phy_mem_filled_pages++;
    }
    else{
        int victim_index = (*get_victim)();

        if (debug_logs)
            printf("\tVictim %d. Originally from %d, now %d\n", page_table[victim_index].phy_page, victim_index, page_index);
        
        swap_out(victim_index, page_table[victim_index].phy_page);
        swap_in(page_index, page_table[victim_index].phy_page);

        if(page_table[victim_index].dirty==1){
            mem_info.dirty_pages++;
        }

        page_table[page_index].phy_page = page_table[victim_index].phy_page;
        page_table[page_index].validity = 1;

        page_table[victim_index].phy_page = 0;
        page_table[victim_index].validity = 0;
        page_table[victim_index].ref_bit = 0;
        page_table[victim_index].dirty = 0;
    }
}

static int lru_subs_alg(){
    return lru_tail->page_index;
}

static int second_chance_subs_alg(){
    int phy_index = fifo_first_in_index;
    int table_index;
    int found = 0;
    while (found == 0) {
        // Finds the physical page index of the next page in the fifo order
        for(int i=0; i<mem_info.page_table_size; i++){
            if(page_table[i].validity==1 &&  page_table[i].phy_page == phy_index){
                table_index = i;
                break;
            }
        }
        // Page gets second chance and next page in fifo order is queued to be checked
        if(page_table[table_index].ref_bit == 1){
            page_table[table_index].ref_bit = 0;
            fifo_first_in_index = (fifo_first_in_index + 1) % mem_info.phy_mem_num_of_pages;
            phy_index = fifo_first_in_index;
        }
        // Found the page
        else{
            found = 1;
        }
    }

    return table_index;
}

static int fifo_subs_alg(){
    int phy_index = fifo_first_in_index;
    fifo_first_in_index = (fifo_first_in_index+1)%mem_info.phy_mem_num_of_pages;

    int table_index;
    for(int i=0; i<mem_info.page_table_size; i++){
        if(page_table[i].validity==1 &&  page_table[i].phy_page == phy_index){
            table_index = i;
            break;
        }
    }

    return table_index;
}

static int random_subs_alg(){
    int phy_index = rand()%mem_info.phy_mem_num_of_pages;

    int table_index;
    for(int i=0; i<mem_info.page_table_size; i++){
        if(page_table[i].validity==1 &&  page_table[i].phy_page == phy_index){
            table_index = i;
            break;
        }
    }
    return table_index;
}

static void print_lru_list() {
    lru_node_t *aux = lru_head;
    while(aux != NULL){
        printf("%d ", page_table[aux->page_index].phy_page);
        aux = aux->next;
    }
    printf("\n");
}

static void swap_node_to_head(lru_node_t *node) {
    if (mem_info.phy_mem_filled_pages == 1) {
        lru_head = node;
        lru_tail = node;
    }
    if (node == lru_head) {
        return;
    }
    if (node == lru_tail) {
        lru_tail = node->prev;
        lru_tail->next = NULL;
    } else {
        if (node->prev != NULL)
            node->prev->next = node->next;
        if (node->next != NULL)
            node->next->prev = node->prev;
    }
    node->next = lru_head;
    node->prev = NULL;
    lru_head->prev = node;
    lru_head = node;
}

static void swap_in(int disk_page_index, int phy_page_index){
    return;
}

static void swap_out(int disk_page_index, int phy_page_index){
    return;
}


