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
  uintptr_t tag;
  char used;
  unsigned int last_used;
} cache_entry_t;

typedef struct {
  long set_idx_bits;
  uintptr_t set_mask;
  long assoc;
  long block_bits;
  long long n_set;
  int miss_cnt;
  int hit_cnt;
  int eviction_cnt;

  int tick;

  cache_entry_t *arr;
} cache_t;

int try_cache(cache_t *cache, char op, uintptr_t addr, int verbose){
  uintptr_t tmp;
  uintptr_t tag;
  uintptr_t set_idx;
  int i;
  int hit = 0;

  cache_entry_t *entry;

  tmp = addr >> (cache->block_bits);
  set_idx = tmp & cache->set_mask;
  tag = tmp >> cache->set_idx_bits;
  //printf("\nset_idx %"PRIxPTR" tag %"PRIxPTR"\n",set_idx,tag);

  entry = cache->arr + (set_idx * cache->assoc);


  for(i=0; i < cache->assoc; i++){
    if(entry[i].tag == tag && entry[i].used){
      if(verbose){
        printf(" hit");
      }
      entry[i].last_used = cache->tick;
      cache->hit_cnt++;
      hit = 1;
      break;
    }
  }

  if(!hit){
    int need_eviction = 1;
    cache->miss_cnt++;
    if(verbose){
      printf(" miss");
    }
    for(i=0; i < cache->assoc; i++){
      if(!entry[i].used){
        entry[i].tag = tag;
        entry[i].used = 1;
        entry[i].last_used = cache->tick;
        need_eviction = 0;
        break;
      }
    }
    if(need_eviction){
      int last_used = entry[0].last_used;
      int target = 0;
      cache->eviction_cnt++;
      printf(" eviction");
      for(i=1; i < cache->assoc; i++){
        if(entry[i].last_used < last_used){
          target = i;
          last_used = entry[i].last_used;
        }
      }
      entry[target].tag = tag;
      entry[target].last_used = cache->tick;
    }
  }

  if(op == 'M'){
    if(verbose){
      printf(" hit");
    }
    cache->hit_cnt++;
  }
  return 0;

}

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
  cache.tick = 0;

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
        cache.set_mask = ~0;
        cache.set_mask >>= (sizeof(uintptr_t)*8 - cache.set_idx_bits);
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

  if(!fp){
    return -1;
  }

  cache.arr = calloc((1 << cache.set_idx_bits) * cache.assoc, sizeof(cache_entry_t));

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

    try_cache(&cache, type[0], addr, verbose);

    if(verbose){
      printf("\n");
    }
    cache.tick++;

  }
  printSummary(cache.hit_cnt, cache.miss_cnt, cache.eviction_cnt);
  if(cache.arr){
    free(cache.arr);
  }
  return 0;
}
