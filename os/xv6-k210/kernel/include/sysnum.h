// #ifndef __SYSNUM_H
// #define __SYSNUM_H

// // System call numbers
// #define SYS_fork         220
// #define SYS_exit         93
// #define SYS_wait         260
// #define SYS_pipe         4
// #define SYS_read         63
// #define SYS_kill         129
// #define SYS_exec         7
// #define SYS_fstat        8
// #define SYS_chdir        9
// #define SYS_dup         10
// #define SYS_getpid      172
// #define SYS_sbrk        12
// #define SYS_sleep       13
// #define SYS_uptime      14
// #define SYS_open        1024
// #define SYS_write       64
// #define SYS_remove      17
// #define SYS_trace       18
// #define SYS_sysinfo     19
// #define SYS_mkdir       20
// #define SYS_close       57
// #define SYS_test_proc   22
// #define SYS_dev         23
// #define SYS_readdir     24
// #define SYS_getcwd      17
// #define SYS_rename      26
// #define SYS_my_shutdown      27

// #define SYS_my_write       9000
// #endif
#ifndef __SYSNUM_H
#define __SYSNUM_H

// System call numbers
#define SYS_fork         1
#define SYS_exit         93
#define SYS_wait         3
#define SYS_pipe         4
#define SYS_read         63
#define SYS_kill         6
#define SYS_exec         7
#define SYS_fstat        8
#define SYS_chdir        9
#define SYS_dup         10
#define SYS_getpid      172
#define SYS_sbrk        12
#define SYS_sleep       13
#define SYS_uptime      14
#define SYS_open        15
#define SYS_write       16
#define SYS_remove      9999
#define SYS_trace       18
#define SYS_sysinfo     19
#define SYS_mkdir       20
#define SYS_close       57
// #define SYS_close       21
#define SYS_test_proc   22
// #define SYS_dev         23
#define SYS_dev         9998
// #define SYS_readdir     24
#define SYS_readdir     9997
#define SYS_getcwd      25
#define SYS_rename      26
#define SYS_my_shutdown      27

#define SYS_my_write       64
#define SYS_my_clone       220
#define SYS_my_wait4       260
#define SYS_my_brk       214
#define SYS_my_execve    221
#define SYS_my_yield    124
#define SYS_my_gettimeofday    169
#define SYS_my_times    153
#define SYS_my_sleep    101
#define SYS_my_getppid    173
#define SYS_my_uname    160
#define SYS_my_getcwd    17
#define SYS_my_openat    56
#define SYS_my_mkdirat   34
#define SYS_my_pipe2     59
#define SYS_my_dup     23
#define SYS_my_dup2     24
#define SYS_my_getdents64 61
#define SYS_my_chdir    49
#define SYS_my_fstat    80
#define SYS_my_unlinkat    35
#define SYS_my_mmap     222
#define SYS_my_munmap   215
#define SYS_my_mount   40
#define SYS_my_umount   39
#endif