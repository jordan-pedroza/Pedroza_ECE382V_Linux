#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(){

  int cpid;

  cpid = fork();
  if (cpid == 0){
    // Child runs here
    printf("I am the child of pid=%d\n", getppid());
    //exit(0);
  }
  printf("I am the parent of pid=%d\n", cpid);
  // code here
  // to do blah
}
