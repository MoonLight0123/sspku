// init: The initial user-level program

#include "kernel/include/types.h"
#include "kernel/include/stat.h"
#include "kernel/include/file.h"
#include "kernel/include/fcntl.h"
#include "kernel/include/sbi.h"
#include "xv6-user/user.h"

char *argv[] = {0};
char *name[] = {"fork","wait","getpid","write","exit","brk","yield","execve",
                "waitpid","gettimeofday","times","sleep","getppid","uname",
                "getcwd","read","open","openat","close","mkdir_","pipe","dup",
                "dup2","getdents","chdir","fstat","unlink","clone","mmap","munmap",
                "mount","umount",};
// char *name[] = {"mount","umount"};
int
main(void)
{
  dev(O_RDWR, CONSOLE, 0);
  dup(0);  // stdout
  dup(0);  // stderr
  int pid, wpid,status;
  int num=sizeof(name)/sizeof(name[0]);
  for(int i=0;i<num;i++)
  {
    pid=fork();
    if(pid==0)
    {
      exec(name[i],argv);
    }
    wpid=wait(&status);
  }
  my_shutdown();
  return 0;
}
