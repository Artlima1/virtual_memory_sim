#!/bin/bash

FILENAME_INDEX=6
ALGORITHM_INDEX=20
PAGES_READ_INDEX=23
DIRTY_PAGES_INDEX=26

LOG_FILE="log_comp.txt"

make

printf "Iniciando suíte de testes\n\n" > $LOG_FILE

printf -- "---------------------------------------------------------------------------------------\n" >> $LOG_FILE
printf "PARTE 1 - PÁGINAS DE 4KB, TAMANHO DE MEMÓRIA VARIÁVEL\n" >> $LOG_FILE
printf -- "---------------------------------------------------------------------------------------\n\n" >> $LOG_FILE

for memsize in 128 256 512 1024 2048 4096 8192 16384; do
  printf "Memória = $memsize KB\n" >> $LOG_FILE
  printf "%15s | %15s | %15s | %15s | %15s\n" "Arquivo" "Algoritmo" "Page faults" "Dirty pages" "Tempo" >> $LOG_FILE
  printf -- "---------------------------------------------------------------------------------------\n" >> $LOG_FILE
  for file in "compilador.log" "compressor.log" "matriz.log" "simulador.log"; do
    for alg in "random" "fifo" "2a" "lru"; do
      start=$(date +%s.%N)
      output=$(./tp2virtual $alg $file 4 $memsize)
      arrOutput=(${output//\\n/ })
      filename=${arrOutput[FILENAME_INDEX]}
      algorithm=${arrOutput[ALGORITHM_INDEX]}
      pages_read=${arrOutput[PAGES_READ_INDEX]}
      dirty_pages=${arrOutput[DIRTY_PAGES_INDEX]}
      end=$(date +%s.%N)
      duration=$(echo "$(date +%s.%N) - $start" | bc)
      execution_time=`printf "%.3fs" $duration`
      printf "%15s | %15s | %15d | %15d | %15s\n" $filename $algorithm $pages_read $dirty_pages $execution_time >> $LOG_FILE
    done
    printf -- "---------------------------------------------------------------------------------------\n" >> $LOG_FILE
  done
  printf "\n" >> $LOG_FILE
done

printf -- "---------------------------------------------------------------------------------------\n" >> $LOG_FILE
printf "PARTE 2 - PÁGINAS DE TAMANHO VARIÁVEL, TAMANHO DE MEMÓRIA FIXO EM 256KB\n" >> $LOG_FILE
printf -- "---------------------------------------------------------------------------------------\n\n" >> $LOG_FILE

for pagesize in 2 4 8 16 32 64; do
  printf "Tamanho de página = $pagesize KB\n" >> $LOG_FILE
  printf "%15s | %15s | %15s | %15s | %15s\n" "Arquivo" "Algoritmo" "Page faults" "Dirty pages" "Tempo" >> $LOG_FILE
  printf -- "---------------------------------------------------------------------------------------\n" >> $LOG_FILE
  for file in "compilador.log" "compressor.log" "matriz.log" "simulador.log"; do
    for alg in "random" "fifo" "2a" "lru"; do
      start=$(date +%s.%N)
      output=$(./tp2virtual $alg $file $pagesize 256)
      arrOutput=(${output//\\n/ })
      filename=${arrOutput[FILENAME_INDEX]}
      algorithm=${arrOutput[ALGORITHM_INDEX]}
      pages_read=${arrOutput[PAGES_READ_INDEX]}
      dirty_pages=${arrOutput[DIRTY_PAGES_INDEX]}
      end=$(date +%s.%N)
      duration=$(echo "$(date +%s.%N) - $start" | bc)
      execution_time=`printf "%.3fs" $duration`
      printf "%15s | %15s | %15d | %15d | %15s\n" $filename $algorithm $pages_read $dirty_pages $execution_time >> $LOG_FILE
    done
    printf -- "---------------------------------------------------------------------------------------\n" >> $LOG_FILE
  done
  printf "\n" >> $LOG_FILE
done
