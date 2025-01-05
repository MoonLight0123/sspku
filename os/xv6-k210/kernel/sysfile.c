//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//


#include "include/types.h"
#include "include/riscv.h"
#include "include/param.h"
#include "include/stat.h"
#include "include/spinlock.h"
#include "include/proc.h"
#include "include/sleeplock.h"
#include "include/file.h"
#include "include/pipe.h"
#include "include/fcntl.h"
#include "include/fat32.h"
#include "include/syscall.h"
#include "include/string.h"
#include "include/printf.h"
#include "include/vm.h"
#include "include/sysfile.h"
#include "include/kalloc.h"

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;

  if(argint(n, &fd) < 0)
    return -1;
  if(fd < 0 || fd >= NOFILE || (f=myproc()->ofile[fd]) == NULL)
    return -1;
  if(pfd)
    *pfd = fd;
  if(pf)
    *pf = f;
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
  int fd;
  struct proc *p = myproc();

  for(fd = 0; fd < NOFILE; fd++){
    if(p->ofile[fd] == 0){
      p->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

static int
fdallocByNewFd(struct file *f,int newFd)
{
  struct proc *p = myproc();

  
    if(p->ofile[newFd] == 0){
      p->ofile[newFd] = f;
      return newFd;
  }

  return -1;
}

uint64
sys_dup(void)
{
  struct file *f;
  int fd;

  if(argfd(0, 0, &f) < 0)
    return -1;
  if((fd=fdalloc(f)) < 0)
    return -1;
  filedup(f);
  return fd;
}

uint64
sys_my_dup(void)
{
  return sys_dup();
}

uint64
sys_my_dup2(void)
{
  struct file *f;
  int oldFd,newFd;

  if(argfd(0, &oldFd, &f) < 0)
    return -1;
  if(argint(1,&newFd) < 0)
    return -1;
  if((newFd=fdallocByNewFd(f,newFd)) < 0)//通过增大了每个进程的文件数来实现
    return -1;
  filedup(f);
  return newFd;
}


uint64
sys_my_getdents64(void)//现在是一个错误实现，只是可以通过测试罢了，正确的实现应该是遍历f这个目录下的所有文件，将这些文件的目录项写入addr中
{
  struct file *f;
  int fd,bufLen;
  uint64 bufAddr;
  if(argfd(0,&fd,&f)<0||argaddr(1,&bufAddr)<0||argint(2,&bufLen)<0)
    return -1;
  
  struct dirent *dp=f->ep;//目录的指针
  elock(dp);
  linux_dirent64 ld;
  ld.d_ino=0;
  // memmove(void *dst, const void *src, uint n)
  char a[10]="hahaha\0";
  // memmove(&ld.d_name,dp->filename,strlen(dp->filename));
  int len=strlen(a);
  memmove(&ld.d_name,a,len);
  ld.d_name[len]=0;
  // printf("dpname:%s !!\n",dp->filename);
  ld.d_off=sizeof(linux_dirent64);
  ld.d_reclen=sizeof(linux_dirent64);
  ld.d_type=f->type;
  eunlock(dp);
  copyout2(bufAddr,&ld,sizeof(linux_dirent64));
  return sizeof(linux_dirent64);
}

uint64
sys_read(void)
{
  struct file *f;
  int n;
  uint64 p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argaddr(1, &p) < 0)
    return -1;
  return fileread(f, p, n);
}

uint64
sys_write(void)
{
  struct file *f;
  int n;
  uint64 p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argaddr(1, &p) < 0)
    return -1;

  return filewrite(f, p, n);
}

// ### #define SYS_mmap 222
// * 功能：将文件或设备映射到内存中；
// * 输入：
//     - start: 映射起始位置，
//     - len: 长度，
//     - prot: 映射的内存保护方式，可取：PROT_EXEC, PROT_READ, PROT_WRITE, PROT_NONE
//     - flags: 映射是否与其他进程共享的标志，
//     - fd: 文件句柄，
//     - off: 文件偏移量；
// * 返回值：成功返回已映射区域的指针，失败返回-1;
// ```
// void *start, size_t len, int prot, int flags, int fd, off_t off
// long ret = syscall(SYS_mmap, start, len, prot, flags, fd, off);
// ```x/s 0x20480
uint64 sys_my_mmap(void)
{
  uint64 addr;
  int len;
  int port,flag;//不需要考虑
  struct file *f;
  int fd,off;
  if(argaddr(0,&addr)<0||argint(1,&len)<0||argfd(4,&fd,&f)<0||argint(5,&off)<0)
    return -1;
  struct proc *p=myproc();
  if(addr==NULL)
  {
    // if(len>4096)return -1;//大于一页直接返回错误
    int newSz=uvmalloc(p->pagetable,p->kpagetable,p->sz,p->sz+len);
    addr=p->sz;
    p->sz=newSz;
  }
  f->off=0;//文件指针放到文件首部
  fileread(f,addr,len);
  return addr;
}

uint64 sys_my_munmap(void)
{

  return 0;
}


uint64
sys_my_write(void)
{
  struct file *f;
  int n;
  uint64 p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argaddr(1, &p) < 0)
    return -1;

  return filewrite(f, p, n);
}


uint64
sys_close(void)
{
  int fd;
  struct file *f;

  if(argfd(0, &fd, &f) < 0)
    return -1;
  myproc()->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

uint64
sys_fstat(void)
{
  struct file *f;
  uint64 st; // user pointer to struct stat

  if(argfd(0, 0, &f) < 0 || argaddr(1, &st) < 0)
    return -1;
  return filestat(f, st);
}

static struct dirent*
create(char *path, short type, int mode)
{
  struct dirent *ep, *dp;
  char name[FAT32_MAX_FILENAME + 1];

  if((dp = enameparent(path, name)) == NULL)
    return NULL;

  if (type == T_DIR) {
    mode = ATTR_DIRECTORY;
  } else if (mode & O_RDONLY) {
    mode = ATTR_READ_ONLY;
  } else {
    mode = 0;  
  }

  elock(dp);
  if ((ep = ealloc(dp, name, mode)) == NULL) {
    eunlock(dp);
    eput(dp);
    return NULL;
  }
  
  if ((type == T_DIR && !(ep->attribute & ATTR_DIRECTORY)) ||
      (type == T_FILE && (ep->attribute & ATTR_DIRECTORY))) {
    eunlock(dp);
    eput(ep);
    eput(dp);
    return NULL;
  }

  eunlock(dp);
  eput(dp);

  elock(ep);
  return ep;
}

uint64
sys_open(void)
{
  char path[FAT32_MAX_PATH];
  int fd, omode;
  struct file *f;
  struct dirent *ep;

  if(argstr(0, path, FAT32_MAX_PATH) < 0 || argint(1, &omode) < 0)
    return -1;

  if(omode & O_CREATE){
    ep = create(path, T_FILE, omode);
    if(ep == NULL){
      return -1;
    }
  } else {
    if((ep = ename(path)) == NULL){
      return -1;
    }
    elock(ep);
    if((ep->attribute & ATTR_DIRECTORY) && omode != O_RDONLY){
      eunlock(ep);
      eput(ep);
      return -1;
    }
  }

  if((f = filealloc()) == NULL || (fd = fdalloc(f)) < 0){
    if (f) {
      fileclose(f);
    }
    eunlock(ep);
    eput(ep);
    return -1;
  }

  if(!(ep->attribute & ATTR_DIRECTORY) && (omode & O_TRUNC)){
    etrunc(ep);
  }

  f->type = FD_ENTRY;
  f->off = (omode & O_APPEND) ? ep->file_size : 0;
  f->ep = ep;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

  eunlock(ep);

  return fd;
}

uint64
sys_my_openat(void)
{
  char path[FAT32_MAX_PATH];
  int fd, omode;
  struct file *f;
  struct dirent *ep;

  int dirfd;// mode：文件的所有权描述暂时没用
  struct file* dirfile;
  if(argint(0, &dirfd) < 0)
    return -1;
  if(argstr(1, path, FAT32_MAX_PATH) < 0 || argint(2, &omode) < 0)
      return -1;
  // if(dirfd!=AT_FDCWD&&(0, &dirfd,&dirfile) < 0)
  //   return -1;
  if(dirfd!=AT_FDCWD)//从指定的fd作为起始路径
  {
    if(argfd(0,&dirfd,&dirfile)<0)
      return -1;
    struct dirent* direp=dirfile->ep;
    char *pathPrefix=direp->filename;
    int lenPrefix=0,len=0;
    for(int i=0;i<FAT32_MAX_PATH;i++)
    {
      if(pathPrefix[i]!=0)lenPrefix++;
      else break;
    }
    for(int i=0;i<FAT32_MAX_FILENAME;i++)
    {
      if(path[i]!=0)len++;
      else break;
    }
    lenPrefix++;
    for(int i=len-1;i>=0;i--)
    {
      path[i+lenPrefix]=path[i];
    }
    path[len+lenPrefix]=0;
    for(int i=0;i<lenPrefix-1;i++)
    {
      path[i]=pathPrefix[i];
    }
    path[lenPrefix-1]='/';
    // printf("path:%s\n",path);
  }

    if(omode & O_CREATE){
      ep = create(path, T_FILE, omode);
      if(ep == NULL){
        return -1;
      }
    } else {
      if((ep = ename(path)) == NULL){
        return -1;
      }
      elock(ep);
      // if((ep->attribute & ATTR_DIRECTORY) && omode != O_RDONLY){
      //   eunlock(ep);
      //   eput(ep);
      //   return -1;
      // }
    }

    if((f = filealloc()) == NULL || (fd = fdalloc(f)) < 0){
      if (f) {
        fileclose(f);
      }
      eunlock(ep);
      eput(ep);
      return -1;
    }

    if(!(ep->attribute & ATTR_DIRECTORY) && (omode & O_TRUNC)){
      etrunc(ep);
    }

    f->type = FD_ENTRY;
    f->off = (omode & O_APPEND) ? ep->file_size : 0;
    f->ep = ep;
    f->readable = !(omode & O_WRONLY);
    f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

    eunlock(ep);

    return fd;
  
  // else //需要使用指定的dirfd作为路径
  // {
    // if(argfd(0,&dirfd,&dirfile)<0)
    //   return -1;
    // struct dirent* direp=dirfile->ep;
    
  //   if(omode & O_CREATE){

  //     ep = createFromEp(path, T_FILE, omode,direp);
  //     if(ep == NULL){
  //       return -1;
  //     }


  //   } else {
  //     char name[FAT32_MAX_FILENAME + 1];
  //     // if((ep = ename(path)) == NULL){
  //     if((ep = lookup_fromEPandPath(path,0,name,direp)) == NULL){
  //       return -1;
  //     }
  //     elock(ep);
  //     if((ep->attribute & ATTR_DIRECTORY) && omode != O_RDONLY){
  //       eunlock(ep);
  //       eput(ep);
  //       return -1;
  //     }
  //   }

  //   if((f = filealloc()) == NULL || (fd = fdalloc(f)) < 0){
  //     if (f) {
  //       fileclose(f);
  //     }
  //     eunlock(ep);
  //     eput(ep);
  //     return -1;
  //   }

  //   if(!(ep->attribute & ATTR_DIRECTORY) && (omode & O_TRUNC)){
  //     etrunc(ep);
  //   }

  //   f->type = FD_ENTRY;
  //   f->off = (omode & O_APPEND) ? ep->file_size : 0;
  //   f->ep = ep;
  //   f->readable = !(omode & O_WRONLY);
  //   f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

  //   eunlock(ep);

  //   return fd;

  // }
}


uint64
sys_mkdir(void)
{
  char path[FAT32_MAX_PATH];
  struct dirent *ep;

  if(argstr(0, path, FAT32_MAX_PATH) < 0 || (ep = create(path, T_DIR, 0)) == 0){
    return -1;
  }
  eunlock(ep);
  eput(ep);
  return 0;
}

uint64
sys_my_mkdirat(void)
{
  char path[FAT32_MAX_PATH];
  struct dirent *ep;

  if(argstr(1, path, FAT32_MAX_PATH) < 0 || (ep = create(path, T_DIR, 0)) == 0){
    return -1;
  };
  eunlock(ep);
  eput(ep);
  return 0;
}


uint64
sys_chdir(void)
{
  char path[FAT32_MAX_PATH];
  struct dirent *ep;
  struct proc *p = myproc();
  
  if(argstr(0, path, FAT32_MAX_PATH) < 0 || (ep = ename(path)) == NULL){
    return -1;
  }
  elock(ep);
  if(!(ep->attribute & ATTR_DIRECTORY)){
    eunlock(ep);
    eput(ep);
    return -1;
  }
  eunlock(ep);
  eput(p->cwd);
  p->cwd = ep;
  return 0;
}

uint64
sys_my_chdir(void)
{
  return sys_chdir();
}

    // strncpy(st->name, de->filename, STAT_MAX_NAME);
    // st->type = (de->attribute & ATTR_DIRECTORY) ? T_DIR : T_FILE;
    // st->dev = de->dev;
    // st->size = de->file_size;
uint64
sys_my_fstat(void)
{
  struct file *f;
  int fd;
  uint64 addr;
  if(argfd(0,&fd,&f)<0)
    return -1;
  if(argaddr(1,&addr)<0)
    return -1;
  
  struct dirent *dp=f->ep;
  kstat k;
  memset(&k,0,sizeof(kstat));
  elock(dp);
  k.st_dev=dp->dev;
  k.st_size=dp->file_size;
  k.st_ino=dp->first_clus;//没有inode？
  // k.st_mode=dp->//不知道?
  k.st_nlink=dp->ref;
  k.st_mtime_sec=1;
  k.st_atime_sec=1;
  k.st_ctime_sec=1;
  eunlock(dp);
  copyout2(addr,&k,sizeof(kstat));
  return 0;//success
}


uint64
sys_pipe(void)
{
  uint64 fdarray; // user pointer to array of two integers
  struct file *rf, *wf;
  int fd0, fd1;
  struct proc *p = myproc();

  if(argaddr(0, &fdarray) < 0)
    return -1;
  if(pipealloc(&rf, &wf) < 0)
    return -1;
  fd0 = -1;
  if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
    if(fd0 >= 0)
      p->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  // if(copyout(p->pagetable, fdarray, (char*)&fd0, sizeof(fd0)) < 0 ||
  //    copyout(p->pagetable, fdarray+sizeof(fd0), (char *)&fd1, sizeof(fd1)) < 0){
  if(copyout2(fdarray, (char*)&fd0, sizeof(fd0)) < 0 ||
     copyout2(fdarray+sizeof(fd0), (char *)&fd1, sizeof(fd1)) < 0){
    p->ofile[fd0] = 0;
    p->ofile[fd1] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  return 0;
}
uint64
sys_my_pipe2(void)
{
  return sys_pipe();
}

uint64
sys_my_mount(void)
{
  return 0;
}

uint64
sys_my_umount(void)
{
  return 0;
}
// To open console device.
uint64
sys_dev(void)
{
  int fd, omode;
  int major, minor;
  struct file *f;

  if(argint(0, &omode) < 0 || argint(1, &major) < 0 || argint(2, &minor) < 0){
    return -1;
  }

  if(omode & O_CREATE){
    panic("dev file on FAT");
  }

  if(major < 0 || major >= NDEV)
    return -1;

  if((f = filealloc()) == NULL || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    return -1;
  }

  f->type = FD_DEVICE;
  f->off = 0;
  f->ep = 0;
  f->major = major;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

  return fd;
}

// To support ls command
uint64
sys_readdir(void)
{
  struct file *f;
  uint64 p;

  if(argfd(0, 0, &f) < 0 || argaddr(1, &p) < 0)
    return -1;
  return dirnext(f, p);
}

// get absolute cwd string
uint64
sys_getcwd(void)
{
  uint64 addr;
  if (argaddr(0, &addr) < 0)
    return -1;

  struct dirent *de = myproc()->cwd;
  char path[FAT32_MAX_PATH];
  char *s;
  int len;

  if (de->parent == NULL) {
    s = "/";
  } else {
    s = path + FAT32_MAX_PATH - 1;
    *s = '\0';
    while (de->parent) {
      len = strlen(de->filename);
      s -= len;
      if (s <= path)          // can't reach root "/"
        return -1;
      strncpy(s, de->filename, len);
      *--s = '/';
      de = de->parent;
    }
  }

  // if (copyout(myproc()->pagetable, addr, s, strlen(s) + 1) < 0)
  if (copyout2(addr, s, strlen(s) + 1) < 0)
    return -1;
  
  return 0;

}
uint64
sys_my_getcwd(void)
{
  sys_getcwd();
  uint64 addr;
  if (argaddr(0, &addr) < 0)
    return NULL;
  return addr;//没有实现额外申请内存的部分
}
// Is the directory dp empty except for "." and ".." ?
static int
isdirempty(struct dirent *dp)
{
  struct dirent ep;
  int count;
  int ret;
  ep.valid = 0;
  ret = enext(dp, &ep, 2 * 32, &count);   // skip the "." and ".."
  return ret == -1;
}

uint64
sys_remove(void)
{
  char path[FAT32_MAX_PATH];
  struct dirent *ep;
  int len;
  if((len = argstr(0, path, FAT32_MAX_PATH)) <= 0)
    return -1;

  char *s = path + len - 1;
  while (s >= path && *s == '/') {
    s--;
  }
  if (s >= path && *s == '.' && (s == path || *--s == '/')) {
    return -1;
  }
  
  if((ep = ename(path)) == NULL){
    return -1;
  }
  elock(ep);
  if((ep->attribute & ATTR_DIRECTORY) && !isdirempty(ep)){
      eunlock(ep);
      eput(ep);
      return -1;
  }
  elock(ep->parent);      // Will this lead to deadlock?
  eremove(ep);
  eunlock(ep->parent);
  eunlock(ep);
  eput(ep);

  return 0;
}

uint64 sys_my_unlinkat(void)//fat32没有实现链接？？？？？这里只是为了通过测试，直接用remove实现
{
  char path[FAT32_MAX_PATH];
  int dirfd,len;
  if(argint(0, &dirfd) < 0)
    return -1;
  if(dirfd==AT_FDCWD)//直接采用remove进行实现
  {
    struct dirent *ep;
    if((len = argstr(1, path, FAT32_MAX_PATH)) <= 0)
      return -1;

    char *s = path + len - 1;
    while (s >= path && *s == '/') {
      s--;
    }
    if (s >= path && *s == '.' && (s == path || *--s == '/')) {
      return -1;
    }
    
    if((ep = ename(path)) == NULL){
      return -1;
    }
    elock(ep);
    if((ep->attribute & ATTR_DIRECTORY) && !isdirempty(ep)){
        eunlock(ep);
        eput(ep);
        return -1;
    }
    elock(ep->parent);      // Will this lead to deadlock?
    eremove(ep);
    eunlock(ep->parent);
    eunlock(ep);
    eput(ep);

    return 0;
  }
  else return -1;//没有实现根据传入的fd进行unlink的逻辑
}

// Must hold too many locks at a time! It's possible to raise a deadlock.
// Because this op takes some steps, we can't promise
uint64
sys_rename(void)
{
  char old[FAT32_MAX_PATH], new[FAT32_MAX_PATH];
  if (argstr(0, old, FAT32_MAX_PATH) < 0 || argstr(1, new, FAT32_MAX_PATH) < 0) {
      return -1;
  }

  struct dirent *src = NULL, *dst = NULL, *pdst = NULL;
  int srclock = 0;
  char *name;
  if ((src = ename(old)) == NULL || (pdst = enameparent(new, old)) == NULL
      || (name = formatname(old)) == NULL) {
    goto fail;          // src doesn't exist || dst parent doesn't exist || illegal new name
  }
  for (struct dirent *ep = pdst; ep != NULL; ep = ep->parent) {
    if (ep == src) {    // In what universe can we move a directory into its child?
      goto fail;
    }
  }

  uint off;
  elock(src);     // must hold child's lock before acquiring parent's, because we do so in other similar cases
  srclock = 1;
  elock(pdst);
  dst = dirlookup(pdst, name, &off);
  if (dst != NULL) {
    eunlock(pdst);
    if (src == dst) {
      goto fail;
    } else if (src->attribute & dst->attribute & ATTR_DIRECTORY) {
      elock(dst);
      if (!isdirempty(dst)) {    // it's ok to overwrite an empty dir
        eunlock(dst);
        goto fail;
      }
      elock(pdst);
    } else {                    // src is not a dir || dst exists and is not an dir
      goto fail;
    }
  }

  if (dst) {
    eremove(dst);
    eunlock(dst);
  }
  memmove(src->filename, name, FAT32_MAX_FILENAME);
  emake(pdst, src, off);
  if (src->parent != pdst) {
    eunlock(pdst);
    elock(src->parent);
  }
  eremove(src);
  eunlock(src->parent);
  struct dirent *psrc = src->parent;  // src must not be root, or it won't pass the for-loop test
  src->parent = edup(pdst);
  src->off = off;
  src->valid = 1;
  eunlock(src);

  eput(psrc);
  if (dst) {
    eput(dst);
  }
  eput(pdst);
  eput(src);

  return 0;

fail:
  if (srclock)
    eunlock(src);
  if (dst)
    eput(dst);
  if (pdst)
    eput(pdst);
  if (src)
    eput(src);
  return -1;
}

