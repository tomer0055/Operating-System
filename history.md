# Operating Systems - Assignment 1
**Course:** Operating Systems 202.1.3031  
**Semester:** Spring 2026  
**Project:** xv6-riscv Assignment 1  

## Overview
This repository contains my solution for Assignment 1 in xv6-riscv.

The assignment includes:
- Task 1: Hello World userspace program
- Task 2: `memsize` system call and test program
- Task 3: `co_yield` coroutine-style system call and test program

## Repository Structure

### New files added
- `user/helloworld.c` — prints `Hello World xv6`
- `user/memsize_test.c` — tests the `memsize()` system call
- `user/co_test.c` — tests the `co_yield()` system call
- `README.md` — documentation of changes and progress

### Existing files modified
- `Makefile` — added userspace programs and any required configuration changes
- `kernel/syscall.h` — added new syscall numbers/declarations
- `kernel/syscall.c` — registered syscall handlers
- `kernel/sysproc.c` — implemented syscall kernel logic
- `user/usys.pl` — added syscall wrappers
- `user/user.h` — added userspace function declarations
- `kernel/proc.c` — scheduling / process logic changes for `co_yield`
- `kernel/defs.h` — added declarations if needed

## Tasks Completed

### Task 1 — Hello World
Implemented a userspace program:
- File: `user/helloworld.c`
- Added program to `Makefile`
- Running `helloworld` in xv6 prints:
  - `Hello World xv6`

### Task 2 — memsize system call
Implemented:
- `int memsize(void);`

Behavior:
- Returns the size of the current running process memory in bytes.

Files involved:
- `kernel/syscall.h`
- `kernel/syscall.c`
- `kernel/sysproc.c`
- `user/usys.pl`
- `user/user.h`
- `user/memsize_test.c`

Test flow:
1. Print current memory usage
2. Allocate 20 KB using `malloc`
3. Print memory usage again
4. Free allocated memory
5. Print memory usage after release

### Task 3 — co_yield system call
Implemented:
- `int co_yield(int pid, int value);`

Behavior:
- Allows one process to cooperatively yield execution to another process
- Passes an integer value between processes
- Returns `-1` on error

Files involved:
- `kernel/syscall.h`
- `kernel/syscall.c`
- `kernel/sysproc.c`
- `kernel/proc.c`
- `user/usys.pl`
- `user/user.h`
- `user/co_test.c`
- `Makefile`

Error cases tested:
- yielding to invalid PID
- yielding to killed process
- self-yield

## Commit Log / Change History

### Commit 1
**Message:** `hw1: basic syscall, function`

Changes:
- added `user/helloworld.c`
- added `user/memsize_test.c`
- started Task 2 syscall integration
- updated relevant build/system call files

### Commit 2
**Message:** `...`
Changes:
- ...

### Commit 3
**Message:** `...`
Changes:
- ...

## Build and Run
Inside the xv6 project directory:

```bash
make qemu
