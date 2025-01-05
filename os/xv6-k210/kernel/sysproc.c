
#include "include/types.h"
#include "include/riscv.h"
#include "include/param.h"
#include "include/memlayout.h"
#include "include/spinlock.h"
#include "include/proc.h"
#include "include/syscall.h"
#include "include/timer.h"
#include "include/kalloc.h"
#include "include/string.h"
#include "include/printf.h"

extern int exec(char *path, char **argv);

uint64
sys_exec(void)
{
  char path[FAT32_MAX_PATH], *argv[MAXARG];
  int i;
  uint64 uargv, uarg;

  if(argstr(0, path, FAT32_MAX_PATH) < 0 || argaddr(1, &uargv) < 0){
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv)){
      goto bad;
    }
    if(fetchaddr(uargv+sizeof(uint64)*i, (uint64*)&uarg) < 0){
      goto bad;
    }
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    argv[i] = kalloc();
    if(argv[i] == 0)
      goto bad;
    if(fetchstr(uarg, argv[i], PGSIZE) < 0)
      goto bad;
  }

  int ret = exec(path, argv);

  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);

  return ret;

 bad:
  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);
  return -1;
}

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  
  // printf("exitid:%d",myproc()->pid);
  exit(n);
  return 0;  // not reached
}

uint64
sys_my_execve(void)
{
  char path[FAT32_MAX_PATH], *argv[MAXARG];
  int i;
  uint64 uargv, uarg;

  if(argstr(0, path, FAT32_MAX_PATH) < 0 || argaddr(1, &uargv) < 0){
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv)){
      goto bad;
    }
  //    // 3.copyin2(char *dst, uint64 srcva, uint64 len)  src->dst
// 1.fetchaddr(uint64 addr, uint64 *ip)  uargv+sizeof(uint64)*i->(uint64*)&uarg
  // 2.if(copyin2((char *)ip, addr, sizeof(*ip)) != 0)   addr->ip
    if(fetchaddr(uargv+sizeof(uint64)*i, (uint64*)&uarg) < 0){
      goto bad;
    }
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    argv[i] = kalloc();
    if(argv[i] == 0)
      goto bad;
      //uarg->argv[i]
    if(fetchstr(uarg, argv[i], PGSIZE) < 0)
      goto bad;
  }

  int ret = exec(path, argv);

  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);

  return ret;

 bad:
  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);
  return -1;
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_my_getppid(void)
{
  return myproc()->parent->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

#define SIGCHLD   17
uint64
sys_my_clone(void)
{
  int flag;
  uint64 fnAddr,spAddr;
  if(argint(0,&flag)<0 || argaddr(1,&spAddr)<0)
    return -1;
  if(spAddr==0)
  {
    return fork();
  }
  fnAddr=*((uint64*)spAddr);
  // printf("fnAddr:%d spAddr:%d\n",fnAddr,spAddr);
  return forkAndExec(fnAddr,spAddr);

}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_my_wait4(void)
{
  int pid;
  uint64 addr;
  int option;
  
  
  if(argint(0, &pid) < 0)
    return -1;
  if(argaddr(1, &addr) < 0)
    return -1;
  if(argint(2, &option) < 0)
    return -1;

  // int retVal=wait4(pid,addr,option);
  return wait4(pid,addr,option);
}

uint64
sys_my_gettimeofday(void)
{

  uint64 taddr;//timeval结构体的地址
  int nouse;

  if(argaddr(0, &taddr) < 0)
    return -1;
  if(argint(1, &nouse) < 0)
    return -1;

  // int retVal=wait4(pid,addr,option);
  int retVal=gettimeofday((TimeVal*)taddr,nouse);
  // printf("time:%d usec:%d \n",((TimeVal*)taddr)->sec,((TimeVal*)taddr)->usec);
  return retVal;
}

uint64
sys_my_times(void)
{
  uint64 taddr;//tms结构体的地址
  if(argaddr(0, &taddr) < 0)
    return -1;

  uint xticks;
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  gettms(taddr,xticks);
  return xticks;
}


uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_my_brk(void)
{
  int end_addr,cur_addr;
  // int n;

  if(argint(0, &end_addr) < 0)
    return -1;
  cur_addr = myproc()->sz;
  if(end_addr==0)
    return cur_addr;
  
  if(growproc(end_addr-cur_addr)<0)
    return -1;
  return 0;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_my_sleep(void)
{
  uint64 taddr;
  if(argaddr(0, &taddr) < 0)
    return -1;
  TimeVal *t=(TimeVal*)taddr;
  int sleepCycles=t->sec;

  // printf("sleepTime:%d\n",sleepCycles);
  uint64 ticks0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < sleepCycles){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    // printf("ticks:%d,ticks0:%d,sleepTime:%d\n",ticks,ticks0,sleepCycles);
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);

  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
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
sys_trace(void)
{
  int mask;
  if(argint(0, &mask) < 0) {
    return -1;
  }
  myproc()->tmask = mask;
  return 0;
}

uint64
sys_my_yield(void)
{
  yield();
  return 0;
}

