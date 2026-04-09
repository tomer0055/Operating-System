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
static int
is_co_waiting(struct proc *p)
{
  return p->state == SLEEPING && p->chan == (void *)p;
}
static void 
co_sleep(struct proc *p)
{
  acquire(&p->lock);
  p->chan = (void *)p;
  p->state = SLEEPING;
  sched();
  p->chan = 0;
  release(&p->lock);
}


uint64
sys_co_yield(void)
{
  int value, pid;
  argint(0, &pid);
  argint(1, &value);
  struct proc *curr = myproc();
  if (pid <= 0 || pid == curr->pid)
  {
    return -1;
  }
  struct proc *target = find_proc_by_pid(pid);
  if (target == 0)
  {
    return -1;
  }
  if (is_co_waiting(target))
  {
    target->trapframe->a0 = value;
    target->state = RUNNABLE;
    release(&target->lock);
  }
  else
  {
    release(&target->lock);
  }

  co_sleep(curr);
  return curr->trapframe->a0;
}
//*HM1 */
