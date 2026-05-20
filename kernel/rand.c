#include "types.h"
#include "defs.h"
#include "spinlock.h"
static uint ran_state;
static struct spinlock ran_lock;
