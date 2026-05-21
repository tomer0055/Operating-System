#include "types.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"

static uint rand_state = 1;
static struct spinlock rand_lock;

void
randinit(void)
{
  initlock(&rand_lock, "rand");
}

void
lcg_srand(uint seed)
{
  acquire(&rand_lock);
  rand_state = seed;
  release(&rand_lock);
}

uint
lcg_rand(void)
{
  uint result;
  acquire(&rand_lock);
  rand_state = rand_state * 1664525 + 1013904223;
  result = rand_state;
  release(&rand_lock);
  return result;
}