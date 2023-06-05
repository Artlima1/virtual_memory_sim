#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "virtual_memory.h"

#define SUBSET 200

int get_subs_alg_type(char * arg){

    if(strcmp(arg, "lru")==0){
        return SUBS_ALG_LRU;
    }

    if(strcmp(arg, "2a")==0){
        return SUBS_ALG_2ND;
    }

    if(strcmp(arg, "fifo")==0){
        return SUBS_ALG_FIFO;
    }

    if(strcmp(arg, "random")==0){
        return SUBS_ALG_RAND;
    }
}

int main(int argc, char **argv){

    printf("Executando o simulador...\n");
    printf("Arquivo de entrada: %s\n", argv[2]);

    vm_init(get_subs_alg_type(argv[1]), atoi(argv[4]), atoi(argv[3]));

    FILE * sim_file;
    char file_name[50] = "./simulations/";
    strcat(file_name, argv[2]);
    sim_file = fopen(file_name, "r");

    #ifdef SUBSET
    int subset = 1;
    #else
    int subset = 0;
    #endif
    int count = 0;

    unsigned addr;
    char rw;
    while (fscanf(sim_file,"%x %c",&addr,&rw) == 2 && (!subset || (subset && (count<SUBSET)))) {
        if(rw == 'W'){
            vm_write(addr);
        }
        if(rw == 'R'){
            vm_read(addr);
        }
        count++;
    }

    fclose(sim_file);

    vm_print_report();

    return 0;
}