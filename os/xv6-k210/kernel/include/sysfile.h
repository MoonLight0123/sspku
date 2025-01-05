#include "types.h"

#define maxNameLen 16
typedef struct {
        uint64        d_ino;
        int64         d_off;
        unsigned short  d_reclen;
        unsigned char   d_type;
        char            d_name[16];
} linux_dirent64;

typedef struct {
        uint64 st_dev;
        uint64 st_ino;
        mode_t st_mode;
        uint32 st_nlink;
        uint32 st_uid;
        uint32 st_gid;
        uint64 st_rdev;
        unsigned long __pad;
        off_t st_size;
        uint32 st_blksize;
        int __pad2;
        uint64 st_blocks;
        long st_atime_sec;
        long st_atime_nsec;
        long st_mtime_sec;
        long st_mtime_nsec;
        long st_ctime_sec;
        long st_ctime_nsec;
        unsigned __unused[2];
} kstat;
