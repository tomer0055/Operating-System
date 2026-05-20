#include "kernel/types.h"
#include "user/user.h"

int main() {
  lcg_srand(42);
  for(int i = 0; i < 5; i++)
    printf("%u\n", lcg_rand());
  exit(0);
}