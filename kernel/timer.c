#include <linux/spinlock.h>
#include <asm-i386/timex.h>
#include <linux/timex.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <asm-i386/interrupt.h>
#include <linux/stdio.h>


long tick = (1000000 + HZ/2) /HZ;  // 时钟中断周期

// 保存系统时间
struct timeval xtime __attribute__((aligned (16)));

int tickadj = 500/HZ ? : 1;
unsigned long volatile jiffies;  // 记录系统启动至今的时钟中断次数

// 保存 xtime 最后更新的时间
unsigned long wall_jiffies; 

rwlock_t xtime_lock = RW_LOCK_UNLOCKED;


/*
 * phase-lock loop variables
 */
/* TIME_ERROR prevents overwriting the CMOS clock */
int time_state = TIME_OK;		/* clock synchronization status	*/
int time_status = STA_UNSYNC;		/* clock status bits		*/
long time_offset;			/* time adjustment (us)		*/
long time_constant = 2;			/* pll time constant		*/
long time_tolerance = MAXFREQ;		/* frequency tolerance (ppm)	*/
long time_precision = 1;		/* clock precision (us)		*/
long time_maxerror = NTP_PHASE_LIMIT;	/* maximum error (us)		*/
long time_esterror = NTP_PHASE_LIMIT;	/* estimated error (us)		*/
long time_phase;			/* phase offset (scaled us)	*/
long time_freq = ((1000000 + HZ/2) % HZ - HZ/2) << SHIFT_USEC;
					/* frequency offset (scaled ppm)*/
long time_adj;				/* tick adjust (scaled 1 / HZ)	*/
long time_reftime;			/* time at last adjustment (s)	*/

long time_adjust;
long time_adjust_step;

static void update_wall_time_on_tick(void)
{
    if ( (time_adjust_step = time_adjust) != 0 ) {
	    if (time_adjust > tickadj)
		    time_adjust_step = tickadj;
	    else if (time_adjust < -tickadj)
		    time_adjust_step = -tickadj;
	     
	    /* Reduce by this step the amount of time left  */
	    time_adjust -= time_adjust_step;
	}
	xtime.tv_usec += tick + time_adjust_step;
	/*
	 * Advance the phase, once it gets to one microsecond, then
	 * advance the tick more.
	 */
	time_phase += time_adj;
	if (time_phase <= -FINEUSEC) {
		long ltemp = -time_phase >> SHIFT_SCALE;
		time_phase += ltemp << SHIFT_SCALE;
		xtime.tv_usec -= ltemp;
	}
	else if (time_phase >= FINEUSEC) {
		long ltemp = time_phase >> SHIFT_SCALE;
		time_phase -= ltemp << SHIFT_SCALE;
		xtime.tv_usec += ltemp;
	}
}

static void update_wall_time(unsigned long ticks)
{
    do{
        ticks--;
        update_wall_time_on_tick();
    }while(ticks);

    if(xtime.tv_usec >= 1000000){
        xtime.tv_usec -= 1000000;
        xtime.tv_sec++;
        // second_overflow();   //TODO: 处理 second 溢出的情况
    }
}

void do_timer(struct pt_regs *regs)
{
    (*(unsigned long *)&jiffies)++;
    // update_process_times();
    mark_bh(TIMER_BH);
}

// 利用 spinlock 原子性
static inline void update_times(void)
{
    unsigned long ticks;

    write_lock_irq(&xtime_lock);      
    vxtime_lock();

    ticks = jiffies - wall_jiffies;
    if(ticks){
        wall_jiffies += ticks;
        update_wall_time(ticks);
    }

    vxtime_unlock();
    write_unlock_irq(&xtime_lock);
}

// TODO:
void timer_bh(void)
{
	cli();
    printk("soft_irq!!!!!\n");
    sti();
    // update_times();     // 更新时钟
    // run_timer_list();
}