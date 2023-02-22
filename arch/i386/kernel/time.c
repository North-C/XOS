#include <linux/time.h>
#include <asm-i386/signal.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/stdio.h>

extern int setup_irq(unsigned int irq, struct irqaction * new);

// unsigned long get_cmos_time(void)
// {
//     unsigned int year, mon, day, hour, min, sec;
// 	int i;

// 	spin_lock(&rtc_lock);
// 	/* The Linux interpretation of the CMOS clock register contents:
// 	 * When the Update-In-Progress (UIP) flag goes from 1 to 0, the
// 	 * RTC registers show the second which has precisely just started.
// 	 * Let's hope other operating systems interpret the RTC the same way.
// 	 */
// 	/* read RTC exactly on falling edge of update flag */
// 	for (i = 0 ; i < 1000000 ; i++)	/* may take up to 1 second... */
// 		if (CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP)
// 			break;
// 	for (i = 0 ; i < 1000000 ; i++)	/* must try at least 2.228 ms */
// 		if (!(CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP))
// 			break;
// 	do { /* Isn't this overkill ? UIP above should guarantee consistency */
// 		sec = CMOS_READ(RTC_SECONDS);
// 		min = CMOS_READ(RTC_MINUTES);
// 		hour = CMOS_READ(RTC_HOURS);
// 		day = CMOS_READ(RTC_DAY_OF_MONTH);
// 		mon = CMOS_READ(RTC_MONTH);
// 		year = CMOS_READ(RTC_YEAR);
// 	} while (sec != CMOS_READ(RTC_SECONDS));
// 	if (!(CMOS_READ(RTC_CONTROL) & RTC_DM_BINARY) || RTC_ALWAYS_BCD)
// 	  {
// 	    BCD_TO_BIN(sec);
// 	    BCD_TO_BIN(min);
// 	    BCD_TO_BIN(hour);
// 	    BCD_TO_BIN(day);
// 	    BCD_TO_BIN(mon);
// 	    BCD_TO_BIN(year);
// 	  }
// 	spin_unlock(&rtc_lock);
// 	if ((year += 1900) < 1970)
// 		year += 100;
// 	return mktime(year, mon, day, hour, min, sec);
// }

/*
 * timer_interrupt() needs to keep up the real-time clock,
 * as well as call the "do_timer()" routine every clocktick
 */
static inline void do_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	do_timer(regs);
}

static void timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	do_timer_interrupt(irq, NULL, regs);
}

static struct irqaction irq0 = {timer_interrupt, SA_INTERRUPT, 0, "timer", NULL, NULL};

void time_init(void)
{
    // extern int x86_udelay_tsc;
	// 设置系统时间
    // xtime.tv_sec = get_cmos_time();
    // xtime.tv_usec= 0;

    
	// 设置时钟中断
    setup_irq(0, &irq0); 
	printk("time_init done\n") ;
}