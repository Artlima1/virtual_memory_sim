#ifndef _VIRTUAL_MEMORY_H_
#define _VIRTUAL_MEMORY_H_

typedef enum {
    false,
    true
} bool;

enum{
    SUBS_ALG_LRU,
    SUBS_ALG_2ND,
    SUBS_ALG_FIFO,
    SUBS_ALG_RAND
};

void vm_init(int subs_alg_type, int total_memory_size, int page_size, bool debug);
void vm_write(unsigned addr);
void vm_read(unsigned addr);

void vm_print_report();

#endif