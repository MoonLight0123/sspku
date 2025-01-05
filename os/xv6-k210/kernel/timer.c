// Timer Interrupt handler


#include "include/types.h"
#include "include/param.h"
#include "include/riscv.h"
#include "include/sbi.h"
#include "include/spinlock.h"
#include "include/timer.h"
#include "include/printf.h"
#include "include/proc.h"
#include "include/vm.h"

struct spinlock tickslock;
uint ticks;//在trapinithart函数中开始计时，这个ticks就是内核时间

void timerinit() {
    initlock(&tickslock, "time");
    #ifdef DEBUG
    printf("timerinit\n");
    #endif
}

void
set_next_timeout() {
    // There is a very strange bug,
    // if comment the `printf` line below
    // the timer will not work.

    // this bug seems to disappear automatically
    printf("");
    sbi_set_timer(r_time() + INTERVAL);
}

void timer_tick() {
    acquire(&tickslock);
    ticks++;
    wakeup(&ticks);
    release(&tickslock);
    set_next_timeout();
}

// #define CPU_FREQ_HZ 390000000 cpu的频率
uint64 gettimeofday(uint64 taddr,int nouse)
{
    uint64 cycles=r_time();
    TimeVal temp;
    temp.sec=cycles / CPU_FREQ_HZ;
    uint64 remaining_cycles = cycles % CPU_FREQ_HZ;
    temp.usec=(remaining_cycles * 1000000ULL) / CPU_FREQ_HZ;
    copyout2(taddr,&temp,sizeof(TimeVal));
    return 0;//success
}

void gettms(uint64 taddr,int ticks)
{
    tms temp;
    temp.tms_cstime=ticks;
    temp.tms_cutime=ticks;
    temp.tms_stime=ticks;
    temp.tms_utime=ticks;
    copyout2(taddr,&temp,sizeof(tms));
    return ;//success
}
