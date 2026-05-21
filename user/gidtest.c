#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  printf("initial gid = %d\n", getgid());
  setgid(7);
  printf("after setgid = %d\n", getgid());
  int pid = fork();
  if(pid == 0){
    printf("child gid = %d\n", getgid());
    exit(0);
  }
  wait(0);
  exit(0);
}