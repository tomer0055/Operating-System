#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define NISRAELI_LOCKS 15
#define MAX_WAITERS 16

struct israeli_lock {
    struct spinlock lk;
    int locked; 
    int active;
    int favoritism;
    struct proc *owner;
    struct proc *queue[MAX_WAITERS];
    int queue_size;
};

static struct israeli_lock israeli_locks[NISRAELI_LOCKS];

void
israeli_lock_init(void) {
    for (int i = 0; i < NISRAELI_LOCKS; i++) {
        initlock(&israeli_locks[i].lk, "israeli_lock");
        israeli_locks[i].locked = 0;
        israeli_locks[i].active = 0;
        israeli_locks[i].favoritism = 0;
        israeli_locks[i].owner = 0;
        israeli_locks[i].queue_size = 0;

        for (int j = 0; j < MAX_WAITERS; j++) {
            israeli_locks[i].queue[j] = 0;
        }
    }
}

int
israeli_create(int favoritism) {
    if (favoritism < 0 || favoritism > 100) {
        return -1; // Invalid favoritism value
    }

    for (int i = 0; i < NISRAELI_LOCKS; i++) {
        acquire(&israeli_locks[i].lk);

        if (israeli_locks[i].active == 0) {
            israeli_locks[i].active = 1;
            israeli_locks[i].locked = 0;
            israeli_locks[i].favoritism = favoritism;
            israeli_locks[i].owner = 0;
            israeli_locks[i].queue_size = 0;

            for (int j = 0; j < MAX_WAITERS; j++) {
                israeli_locks[i].queue[j] = 0;
            }
            release(&israeli_locks[i].lk);
            return i; // Return the index of the created lock
        }
        release(&israeli_locks[i].lk);
    }
    return -1; // No available lock
}

int
israeli_destroy(int lock_id) {
    if (lock_id < 0 || lock_id >= NISRAELI_LOCKS) {
        return -1; // Invalid lock ID
    }

    struct israeli_lock *lock = &israeli_locks[lock_id];
    acquire(&lock->lk);

    if (lock->active == 0) {
        release(&lock->lk);
        return -1; // Lock is not active
    }

    if (lock->locked || lock->queue_size > 0) {
        release(&lock->lk);
        return -1; // Lock is currently in use
    }

    lock->active = 0;
    lock->favoritism = 0;
    lock->owner = 0;
    lock->queue_size = 0;

    release(&lock->lk);
    return 0; // Lock destroyed successfully
}

static int 
in_queue(struct israeli_lock *lock, struct proc *p) {
    for (int i = 0; i < lock->queue_size; i++) {
        if (lock->queue[i] == p) {
            return 1; // Process is already in the queue
        }
    }
    return 0; // Process is not in the queue
}

static int
enqueue(struct israeli_lock *lock, struct proc *p) {
    if (lock->queue_size >= MAX_WAITERS) {
        return -1; // Queue is full
    }
    
    if (in_queue(lock, p)) {
        return 0; // Process is already in the queue
    }
    
    lock->queue[lock->queue_size] = p;
    lock->queue_size++;
    return 0; // Process enqueued successfully
}

static struct proc*
remove_at(struct israeli_lock *lock, int index) {
    if (index < 0 || index >= lock->queue_size) {
        return 0; // Invalid index
    }
    
    struct proc *p = lock->queue[index];
    
    for (int i = index; i < lock->queue_size - 1; i++) {
        lock->queue[i] = lock->queue[i + 1];
    }

    lock->queue_size--;
    lock->queue[lock->queue_size] = 0; // Clear the last element
    return p; // Return the removed process
}


static int
choose_next_index(struct israeli_lock *lock, int releasing_gid) {
    int gid_index = -1;

    for (int i = 0; i < lock->queue_size; i++) {
        if (lock->queue[i] && lock->queue[i]->gid == releasing_gid) {
            gid_index = i;
            break;
        }
    }

    if (gid_index != -1) {
        uint random_val = lcg_rand() % 100;

        if (random_val < lock->favoritism) {
            return gid_index; // Favor the same group
        }
    }
    return 0; // Default to the first process in the queue
}
    

int
israeli_acquire(int lock_id) {
    if (lock_id < 0 || lock_id >= NISRAELI_LOCKS) {
        return -1; // Invalid lock ID
    }

    struct israeli_lock *lock = &israeli_locks[lock_id];
    struct proc *current_proc = myproc();

    acquire(&lock->lk);

    if (lock->active == 0) {
        release(&lock->lk);
        return -1; // Lock is not active
    }

    if (lock->owner == current_proc) {
        release(&lock->lk);
        return -1; // Process already owns the lock
    }

    if (lock->locked == 0 && lock->queue_size == 0) {
        lock->locked = 1;
        lock->owner = current_proc;
        release(&lock->lk);
        return 0; // Lock acquired successfully
    }

    if (enqueue(lock, current_proc) < 0) {
        release(&lock->lk);
        return -1; // Failed to enqueue process
    }

    while(lock->owner != current_proc) {
        sleep(current_proc, &lock->lk);

        if (lock->active == 0) {
            release(&lock->lk);
            return -1; // Lock was destroyed while waiting
        }
    }

    release(&lock->lk);
    return 0; // Lock acquired successfully
}


int
israeli_release(int lock_id) {
    if (lock_id < 0 || lock_id >= NISRAELI_LOCKS) {
        return -1; // Invalid lock ID
    }

    struct israeli_lock *lock = &israeli_locks[lock_id];
    struct proc *current_proc = myproc();

    acquire(&lock->lk);

    if (lock->active == 0 || lock->locked == 0 || lock->owner != current_proc) {
        release(&lock->lk);
        return -1; // Lock is not active, or not locked, or current process does not own the lock
    }

    int releasing_gid = current_proc->gid;

    if(lock->queue_size == 0) {
        lock->locked = 0;
        lock->owner = 0;
        release(&lock->lk);
        return 0; // Lock released successfully
    }

    int next_index = choose_next_index(lock, releasing_gid);
    struct proc *next_proc = remove_at(lock, next_index);

    lock->owner = next_proc;
    lock->locked = 1;

    wakeup(next_proc);

    release(&lock->lk);
    return 0; // Lock released successfully
}
        