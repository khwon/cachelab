#include "cachelab.h"
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

void print_help(char *progname){
  printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", progname);
  printf("Options:\n");
  printf("  -h         Print this help message.\n");
  printf("  -v         Optional verbose flag.\n");
  printf("  -s <num>   Number of set index bits.\n");
  printf("  -E <num>   Number of lines per set.\n");
  printf("  -b <num>   Number of block offset bits.\n");
  printf("  -t <file>  Trace file.\n\n");

  printf("Examples:\n");
  printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", progname);
  printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", progname);
}

typedef struct {
  long set_idx_bits;
  long assoc;
  long block_bits;
  long long n_set;
  uintptr_t *arr;
  int miss_cnt;
  int hit_cnt;
  int eviction_cnt;
} cache_t;

int main(int argc, char **argv)
{
  int opt;
  int verbose = 0;
  char *endptr;
  FILE *fp = NULL;
  char buf[1024];

  cache_t cache;

  cache.arr = NULL;
  cache.miss_cnt = 0;
  cache.hit_cnt = 0;
  cache.eviction_cnt = 0;

  while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1){
    switch(opt) {
      case 'h':
        print_help(argv[0]);
        return 0;
      case 'v':
        verbose = 1;
        break;
      case 's':
        cache.set_idx_bits = strtol(optarg, &endptr, 10);
        break;
      case 'E':
        cache.assoc = strtol(optarg, &endptr, 10);
        break;
      case 'b':
        cache.block_bits = strtol(optarg, &endptr, 10);
        break;
      case 't':
        if(!fp){
          fp = fopen(optarg, "r");
        }
        break;
      default:
        print_help(argv[0]);
        return -1;
    }
  }

  cache.arr = calloc((1 << cache.set_idx_bits) * cache.assoc, sizeof(uintptr_t));

  if(!cache.arr){
    return -1;
  }

  while(fgets(buf,1024,fp)){
    char *ret_ptr;
    char type[3];
    uintptr_t addr;
    int size;

    ret_ptr = strtok(buf," ,");
    if(!ret_ptr){
      printf("unknown format\n");
      return -1;
    }
    strncpy(type,ret_ptr,2);
    type[2] = '\0';

    if(type[0] == 'I'){
      // ignore instruction load
      continue;
    }

    ret_ptr = strtok(NULL," ,");
    if(!ret_ptr){
      printf("unknown format\n");
      return -1;
    }
    addr = strtol(ret_ptr, &endptr, 16);

    ret_ptr = strtok(NULL," ,");
    if(!ret_ptr){
      printf("unknown format\n");
      return -1;
    }
    size = strtol(ret_ptr, &endptr, 16);

    if(verbose){
      printf("%c %" PRIxPTR ",%d", type[0], addr, size);
    }

    if(verbose){
      printf("\n");
    }

  }
  printSummary(0, 0, 0);
  if(cache.arr){
    free(cache.arr);
  }
  return 0;
}
