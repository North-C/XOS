#ifndef _LINUX_INTERRUPT_H
#define _LINUX_INTERRUPT_H

#include <asm-i386/ptrace.h>
#include <linux/linkage.h>
#include <asm-i386/atomic.h>
#include <linux/cache.h>
#include <asm-i386/bitops.h>
#include <linux/kernel.h>

struct irqaction{
    void (*handler)(int, void *, struct pt_regs *);
    unsigned long flags;
    unsigned long mask;
    const char *name;
    void *dev_id;
    struct irqaction *next;
};

struct softirq_action
{
    void	(*action)(struct softirq_action *);
    void	*data;
};

enum {
	TIMER_BH = 0,   // 时钟中断
	TQUEUE_BH,
	DIGI_BH,
	SERIAL_BH,
	RISCOM8_BH,
	SPECIALIX_BH,
	AURORA_BH,
	ESP_BH,
	SCSI_BH,
	IMMEDIATE_BH,
	CYCLADES_BH,
	CM206_BH,
	JS_BH,
	MACSERIAL_BH,
	ISICOM_BH
};

// 声明软中断类型，表示相对优先级，索引号小的先执行
enum{
    HI_SOFTIRQ=0,     // 优先级高的tasklets
    NET_TX_SOFTIRQ,   // 发送网络数据包
    NET_RX_SOFTIRQ,   // 接收网络数据包
    TASKLET_SOFTIRQ   // 正常优先级的 tasklets
};

struct tasklet_struct
{
    struct tasklet_struct *next;   // 下一个tasklet
    unsigned long state;        // tasklet 的状态
    atomic_t count;             // 引用计数器
    void (*func)(unsigned long);    // tasklet 处理函数
    unsigned long data;         // 给 tasklet 处理函数的参数
};

enum{
    TASKLET_STATE_SCHED,       // Tasklet 正在被调度中
    TASKLET_STATE_RUN          // 正在某个 CPU 中执行
};

struct tasklet_head
{
	struct tasklet_struct *list;
} __attribute__ ((__aligned__(SMP_CACHE_BYTES)));


#define __cpu_raise_softirq(cpu, nr) do { softirq_pending(cpu) |= 1UL << (nr); } while (0)

asmlinkage void do_softirq(void);

// extern void FASTCALL(__tasklet_schedule(struct tasklet_struct *t));
extern void __tasklet_schedule(struct tasklet_struct *t);

static inline void tasklet_schedule(struct tasklet_struct *t)
{
	if (!test_and_set_bit(TASKLET_STATE_SCHED, &t->state))
		__tasklet_schedule(t);
}

// extern void FASTCALL(__tasklet_schedule(struct tasklet_struct *t));
extern void __tasklet_hi_schedule(struct tasklet_struct *t);

static inline void tasklet_hi_schedule(struct tasklet_struct *t)
{
    if (!test_and_set_bit(TASKLET_STATE_SCHED, &t->state))
		__tasklet_hi_schedule(t);
}

// 单核情况
#define tasklet_trylock(t) 1
#define tasklet_unlock_wait(t) do{ } while(0)
#define tasklet_unlock(t) do{ } while(0)

extern struct tasklet_struct bh_task_vec[32];

static inline void mark_bh(int nr)
{
	tasklet_hi_schedule(bh_task_vec + nr);
}

#endif