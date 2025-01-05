
#include "include/types.h"
#include "include/param.h"
#include "include/memlayout.h"
#include "include/riscv.h"
#include "include/spinlock.h"
#include "include/proc.h"
#include "include/syscall.h"
#include "include/sysinfo.h"
#include "include/kalloc.h"
#include "include/vm.h"
#include "include/string.h"
#include "include/printf.h"
#include "include/sbi.h"
#include "include/timer.h"
// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz)
    return -1;
  // if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
  // copyin2(char *dst, uint64 srcva, uint64 len)
  if(copyin2((char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  // struct proc *p = myproc();
  // int err = copyinstr(p->pagetable, buf, addr, max);
  int err = copyinstr2(buf, addr, max);
  if(err < 0)
    return err;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  *ip = argraw(n);
  return 0;
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
int
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
  return 0;
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  if(argaddr(n, &addr) < 0)
    return -1;
  return fetchstr(addr, buf, max);
}

extern uint64 sys_chdir(void);
extern uint64 sys_close(void);
extern uint64 sys_dup(void);
extern uint64 sys_exec(void);
extern uint64 sys_exit(void);
extern uint64 sys_fork(void);
extern uint64 sys_fstat(void);
extern uint64 sys_getpid(void);
extern uint64 sys_kill(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_open(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_wait(void);
extern uint64 sys_write(void);
extern uint64 sys_uptime(void);
extern uint64 sys_test_proc(void);
extern uint64 sys_dev(void);
extern uint64 sys_readdir(void);
extern uint64 sys_getcwd(void);
extern uint64 sys_remove(void);
extern uint64 sys_trace(void);
extern uint64 sys_sysinfo(void);
extern uint64 sys_rename(void);

extern uint64 sys_my_shutdown(void);
extern uint64 sys_my_write(void);
extern uint64 sys_my_clone(void);
extern uint64 sys_my_wait4(void);
extern uint64 sys_my_brk(void);
extern uint64 sys_my_execve(void);
extern uint64 sys_my_yield(void);
extern uint64 sys_my_gettimeofday(void);
extern uint64 sys_my_times(void);
extern uint64 sys_my_sleep(void);
extern uint64 sys_my_getppid(void);
extern uint64 sys_my_getcwd(void);
extern uint64 sys_my_openat(void);
extern uint64 sys_my_mkdirat(void);
extern uint64 sys_my_pipe2(void);
extern uint64 sys_my_dup(void);
extern uint64 sys_my_dup2(void);
extern uint64 sys_my_getdents64(void);
extern uint64 sys_my_chdir(void);
extern uint64 sys_my_fstat(void);
extern uint64 sys_my_unlinkat(void);
extern uint64 sys_my_mmap(void);
extern uint64 sys_my_munmap(void);
extern uint64 sys_my_mount(void);
extern uint64 sys_my_umount(void);
static uint64 (*syscalls[])(void) = {
  [SYS_fork]        sys_fork,
  [SYS_exit]        sys_exit,
  [SYS_wait]        sys_wait,
  [SYS_pipe]        sys_pipe,
  [SYS_read]        sys_read,
  [SYS_kill]        sys_kill,
  [SYS_exec]        sys_exec,
  [SYS_fstat]       sys_fstat,
  [SYS_chdir]       sys_chdir,
  [SYS_dup]         sys_dup,
  [SYS_getpid]      sys_getpid,
  [SYS_sbrk]        sys_sbrk,
  [SYS_sleep]       sys_sleep,
  [SYS_uptime]      sys_uptime,
  [SYS_open]        sys_open,
  [SYS_write]       sys_write,
  [SYS_mkdir]       sys_mkdir,
  [SYS_close]       sys_close,
  [SYS_test_proc]   sys_test_proc,
  [SYS_dev]         sys_dev,
  [SYS_readdir]     sys_readdir,
  [SYS_getcwd]      sys_getcwd,
  [SYS_remove]      sys_remove,
  [SYS_trace]       sys_trace,
  [SYS_sysinfo]     sys_sysinfo,
  [SYS_rename]      sys_rename,

  [SYS_my_shutdown]      sys_my_shutdown,
  [SYS_my_write]      sys_my_write,
  [SYS_my_clone]      sys_my_clone,
  [SYS_my_wait4]      sys_my_wait4,
  [SYS_my_brk]      sys_my_brk ,
  [SYS_my_execve]      sys_my_execve ,
  [SYS_my_yield]      sys_my_yield ,
  [SYS_my_gettimeofday]      sys_my_gettimeofday ,
  [SYS_my_times]      sys_my_times ,
  [SYS_my_sleep]      sys_my_sleep ,
  [SYS_my_getppid]      sys_my_getppid ,
  [SYS_my_uname]      sys_my_uname ,
  [SYS_my_getcwd]      sys_my_getcwd ,
  [SYS_my_openat]      sys_my_openat ,
  [SYS_my_mkdirat]      sys_my_mkdirat ,
  [SYS_my_pipe2]      sys_my_pipe2 ,
  [SYS_my_dup]      sys_my_dup ,
  [SYS_my_dup2]      sys_my_dup2 ,
  [SYS_my_getdents64]      sys_my_getdents64 ,
  [SYS_my_chdir]      sys_my_chdir ,
  [SYS_my_fstat]      sys_my_fstat ,
  [SYS_my_unlinkat]      sys_my_unlinkat ,
  [SYS_my_mmap]      sys_my_mmap ,
  [SYS_my_munmap]      sys_my_munmap ,
  [SYS_my_mount]      sys_my_mount ,
  [SYS_my_umount]      sys_my_umount ,
};
// SYS_my_unlinkat
static char *sysnames[] = {
  [SYS_fork]        "fork",
  [SYS_exit]        "exit",
  [SYS_wait]        "wait",
  [SYS_pipe]        "pipe",
  [SYS_read]        "read",
  [SYS_kill]        "kill",
  [SYS_exec]        "exec",
  [SYS_fstat]       "fstat",
  [SYS_chdir]       "chdir",
  [SYS_dup]         "dup",
  [SYS_getpid]      "getpid",
  [SYS_sbrk]        "sbrk",
  [SYS_sleep]       "sleep",
  [SYS_uptime]      "uptime",
  [SYS_open]        "open",
  [SYS_write]       "write",
  [SYS_mkdir]       "mkdir",
  [SYS_close]       "close",
  [SYS_test_proc]   "test_proc",
  [SYS_dev]         "dev",
  [SYS_readdir]     "readdir",
  [SYS_getcwd]      "getcwd",
  [SYS_remove]      "remove",
  [SYS_trace]       "trace",
  [SYS_sysinfo]     "sysinfo",
  [SYS_rename]      "rename",
  [SYS_my_shutdown]      "my_shutdown",
  [SYS_my_write]      "my_write",
  [SYS_my_clone]      "my_clone",
  [SYS_my_wait4]      "my_wait4",
  [SYS_my_execve]      "my_execve" ,
  [SYS_my_yield]      "my_yield" ,
  [SYS_my_gettimeofday]      "my_gettimeofday" ,
  [SYS_my_times]      "my_times" ,
  [SYS_my_sleep]      "my_sleep ",
  [SYS_my_getppid]      "my_getppid",
  [SYS_my_uname]      "my_uname" ,
  [SYS_my_getcwd]      "my_getcwd",
  [SYS_my_openat]      "my_openat" ,
  [SYS_my_mkdirat]      "my_mkdirat" ,
  [SYS_my_pipe2]      "my_pipe2" ,
  [SYS_my_dup]      "my_dup" ,
  [SYS_my_dup2]      "my_dup2" ,
  [SYS_my_getdents64]      "my_getdents64" ,
  [SYS_my_chdir]      "my_chdir" ,
  [SYS_my_fstat]      "my_fstat" ,
  [SYS_my_unlinkat]      "my_unlinkat" ,
  [SYS_my_mmap]      "my_mmap" ,
  [SYS_my_mmap]      "my_munmap" ,
  [SYS_my_mount]      "my_mount" ,
  [SYS_my_umount]      "my_umount" ,
};

void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    p->trapframe->a0 = syscalls[num]();
        // trace
    if ((p->tmask & (1 << num)) != 0) {
      printf("pid %d: %s -> %d\n", p->pid, sysnames[num], p->trapframe->a0);
    }
  } else {
    printf("pid %d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}

uint64 
sys_test_proc(void) {
    int n;
    argint(0, &n);
    printf("hello world from proc %d, hart %d, arg %d\n", myproc()->pid, r_tp(), n);
    return 0;
}

uint64
sys_sysinfo(void)
{
  uint64 addr;
  // struct proc *p = myproc();

  if (argaddr(0, &addr) < 0) {
    return -1;
  }

  struct sysinfo info;
  info.freemem = freemem_amount();
  info.nproc = procnum();

  // if (copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0) {
  if (copyout2(addr, (char *)&info, sizeof(info)) < 0) {
    return -1;
  }

  return 0;
}

uint64
sys_my_shutdown(void)
{
  // printf("shutdown!");
  sbi_shutdown();
  // printf("shutdown2");
  return 0;
}

uint64
sys_my_uname(void)
{
  uint64 addr;
    if (argaddr(0, &addr) < 0) {
    return -1;
  }
  utsname name;
  name.domainname[0]=0;
  name.machine[0]=0;
  name.nodename[0]=0;
  name.release[0]=0;
  name.sysname[0]=0;
  name.version[0]=0;
  if (copyout2(addr, (char *)&name, sizeof(name)) < 0) {
    return -1;
  }

  return 0;//success
}