#include "types.h"
#include "spinlock.h"
#include "defs.h"

static uint rand_state = 0;
static struct spinlock rand_lock;

void randinit(void) {
  initlock(&rand_lock, "rand");
}

void lcg_srand(uint seed) {
  acquire(&rand_lock);
  rand_state = seed;
  release(&rand_lock);
}

uint lcg_rand(void) {
  acquire(&rand_lock);
  rand_state = 1664525 * rand_state + 1013904223;
  uint r = rand_state;
  release(&rand_lock);
  return r;
}