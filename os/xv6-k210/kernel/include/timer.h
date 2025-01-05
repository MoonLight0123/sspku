#ifndef __TIMER_H
#define __TIMER_H

#include "types.h"
#include "spinlock.h"

#define CPU_FREQ_HZ 390000000

extern struct spinlock tickslock;
extern uint ticks;

typedef struct
{
    uint64 sec;  // 自 Unix 纪元起的秒数
    uint64 usec; // 微秒数
} TimeVal;


typedef struct
{
	long tms_utime;  
	long tms_stime;  
	long tms_cutime; 
	long tms_cstime; 
} tms;


void timerinit();
void set_next_timeout();
void timer_tick();
uint64 gettimeofday(uint64 taddr,int nouse);
void gettms(uint64 taddr,int ticks);
#endif
