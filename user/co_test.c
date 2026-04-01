#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  // (a) yield to non-existent PID
  int ret = co_yield(9999, 1);
  if (ret == -1)
    printf("OK: non-existent PID returned -1\n");
  else
    printf("FAIL: non-existent PID returned %d\n", ret);

  // (b) yield to self
  ret = co_yield(getpid(), 1);
  if (ret == -1)
    printf("OK: self-yield returned -1\n");
  else
    printf("FAIL: self-yield returned %d\n", ret);

  // (c) yield to invalid PID
  ret = co_yield(-1, 1);
  if (ret == -1)
    printf("OK: negative PID returned -1\n");
  else
    printf("FAIL: negative PID returned %d\n", ret);

  
  int pid1 = getpid();   
  int pid2 = fork();     

  if (pid2 == 0) {
    // Child
    for (;;) {
      int value = co_yield(pid1, 1);
      printf("Child received: %d\n", value);  
    }
  } else {
    // Parent
    for (int i = 0; i < 5; i++) {  
      int value = co_yield(pid2, 2);
      printf("Parent received: %d\n", value);  
    }
    kill(pid2);
    wait(0);

    
    ret = co_yield(pid2, 1);
    if (ret == -1)
      printf("OK: killed process returned -1\n");
    else
      printf("FAIL: killed process returned %d\n", ret);
  }

  exit(0);
}