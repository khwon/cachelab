#include "cachelab.h"
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

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

int main(int argc, char **argv)
{
  int opt;
  int verbose = 0;
  long n_set_idx_bits;
  long assoc;
  long block_bits;
  char *endptr;
  FILE *fp = NULL;

  while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1){
    switch(opt) {
      case 'h':
        print_help(argv[0]);
        return 0;
      case 'v':
        verbose = 1;
        break;
      case 's':
        n_set_idx_bits = strtol(optarg, &endptr, 10);
        break;
      case 'E':
        assoc = strtol(optarg, &endptr, 10);
        break;
      case 'b':
        block_bits = strtol(optarg, &endptr, 10);
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
  printSummary(0, 0, 0);
  return 0;
}
