#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include"malloc.hpp"

int div_roundup(int a, int b) {
  return (a+b-1)/b;
}

void * new_malloc(size_t size) {
  // int pagesize = getpagesize();
  // size_t required = size + sizeof(size_t);
  // int num_pages = div_roundup(required, pagesize);
  void * ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (ptr == MAP_FAILED) return NULL;
  // *(size_t*)new_region = required; // We use this to free() the right number of bytes
  // return (void*)((unsigned long)new_region+sizeof(size_t));
  return ptr;
}

void new_free(void * ptr) {
  // void* region = (void*)((unsigned long)ptr-sizeof(size_t));
  if (munmap(ptr, *(size_t*)ptr) != 0) {
    perror("Could not munmap");
  }
}