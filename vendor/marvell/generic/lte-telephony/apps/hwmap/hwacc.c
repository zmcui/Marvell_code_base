/*
 * (c) Copyright 2008 Marvell International Ltd.
 * All rights reserved
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

#define HWMAP_DEVICE "/dev/hwmap"

int main(int argc, char* argv[])
{
  int fid;
  unsigned int pagesize, len, len_aligned;
  char option;
  unsigned int addr, addr_aligned, data;
  volatile unsigned int *pa;
  void *vpa;
  FILE *fout = NULL;
  int ret;
  
  len = pagesize = sysconf(_SC_PAGESIZE);
  if (argc == 3 && tolower(*argv[1]) == 'l') {
    cp_save_diaglog(argv[2]);
    return 0;
  }

  ret = parseArgs(argc, argv, &option, &addr, &data);
  if(ret < 0)
  {
    printf("USAGE: %s [r|w] addr-hex <data-hex>\n", argv[0]);
    printf("USAGE: %s [d] addr-hex <size-hex> out file name\n", argv[0]);
    printf("USAGE: %s [l] out-file-name\n", argv[0]);
    exit(-1);
  }
  printf("Option = %c Addr = %.8x", option, addr);
  if(option == "w")
    printf("Data=%.8x", data);
  if(option == "d")
  {
    printf(" Dump=%.8x bytes int file %s", data, argv[4]);
    len = (int)data;
  }
  
  printf("\n");
  
  if((fid = open(HWMAP_DEVICE, O_RDWR)) < 0)
  {
    printf("Failed to open %s\n", HWMAP_DEVICE);
    exit(-1);
  }
  
  

}
