#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"



int main(int argc, char *argv[]) {
  int size = memsize();
  printf("Memory size before allocation: %d bytes\n", size);
  char *e = (char*) malloc(20000 * sizeof(char));
  printf("Memory size after allocation: %d bytes\n", memsize());
  free(e);
  printf("Memory size after freeing: %d bytes\n", memsize());
  exit(0);
}