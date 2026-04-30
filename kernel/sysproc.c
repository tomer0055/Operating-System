#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

extern struct proc proc[NPROC];


uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
uint64
sys_memsize(void)
{
  return myproc()->sz;
}








//** HM1 */
//**return with lock still held.*/

#define CO_CHAN_SCHED(p)  ((void *)(p))
#define CO_CHAN_DIRECT(p) ((void *)(((uint64)(p)) | 1ULL))

static int
is_co_waiting(struct proc *p)
{
  return p->state == SLEEPING &&
         (p->chan == CO_CHAN_SCHED(p) || p->chan == CO_CHAN_DIRECT(p));
}

static int
is_co_waiting_sched(struct proc *p)
{
  return p->state == SLEEPING && p->chan == CO_CHAN_SCHED(p);
}

static struct proc *
find_proc_by_pid(int pid)
{
  struct proc *p;
  for (p = proc; p < &proc[NPROC]; p++)
  {
    acquire(&p->lock);
    if (p->pid == pid)
    {
      if (p->state == UNUSED || p->state == ZOMBIE)
      {
        release(&p->lock);
        return 0;
      }
      return p;
    }
    release(&p->lock);
  }
  return 0;
}
static void
co_sleep(struct proc *p)
{
  acquire(&p->lock);
  p->chan = CO_CHAN_SCHED(p);
  p->state = SLEEPING;
  sched();
  p->chan = 0;
  if (holding(&p->lock))
    release(&p->lock);
}

uint64
sys_co_yield(void)
{

  int value, pid;
  struct proc *curr, *target;
  int target_from_sched;

  argint(0, &pid);
  argint(1, &value);

  curr = myproc();
  //Edge Case 1: pid <= 0 or pid == curr->pid
  if (pid <= 0 || pid == curr->pid)
    return -1;

  target = find_proc_by_pid(pid);   // returns with target->lock held
  //Edge Case 2: target == 0
  if (target == 0)
    return -1;
  //Edge Case 3: target is not waiting
  if (!is_co_waiting(target))
  {
  release(&target->lock);
  co_sleep(curr);
  //Edge Case 4: curr is killed
  if (killed(curr))
    return -1;
  return curr->trapframe->a0;
}

  target_from_sched = is_co_waiting_sched(target);

  acquire(&curr->lock);

  target->trapframe->a0 = value;

  curr->chan = CO_CHAN_DIRECT(curr);
  curr->state = SLEEPING;

  target->state = RUNNING;
  mycpu()->proc = target;

  release(&curr->lock);

  //Edge Case 5: target is resuming from co_sleep()->sched() keep the lock held. else release it
  if (!target_from_sched && holding(&target->lock))
  release(&target->lock);

// if went to sleep from sched keep the lock else release it
swtch(&curr->context, &target->context);

mycpu()->proc = curr;
curr->chan = 0;


if (holding(&curr->lock))
  release(&curr->lock);

//Edge Case 6: curr is killed after swtch
if (killed(curr))
  return -1;

return curr->trapframe->a0;
}
//*HM1 */
