#include <linux/interrupt.h>
#include <linux/types.h>
#include <asm-i386/smp.h>
#include <asm-i386/hardirq.h>
#include <linux/cache.h>
#include <linux/threads.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <asm-i386/softirq.h>
#include <linux/debug.h>
#include <asm-i386/system.h>
#include <asm-i386/interrupt.h>

// 每个CPU自己的软中断控制结构
irq_cpustat_t irq_stat[NR_CPUS] ____cacheline_aligned;

// 全局的软中断控制结构
static struct softirq_action softirq_vec[32] __cacheline_aligned;

struct tasklet_struct bh_task_vec[32];

// static inline void wakeup_softirq(unsigned cpu)
// {
//     struct task_struct * tsk = ksoftirq_task(cpu);

//     if(tsk && tsk->state != TASK_RUNNING)
//         wake_up_process(tsk);
// }

// 通过修改标志位，发出软中断请求
static void cpu_raise_softirq(unsigned int cpu, unsigned int nr)
{
	__cpu_raise_softirq(cpu, nr);    

	// if (!(local_irq_count(cpu) | local_bh_count(cpu)))
	// 	wakeup_softirqd(cpu);
}

/* Tasklets */
struct tasklet_head tasklet_vec[NR_CPUS] __cacheline_aligned;
struct tasklet_head tasklet_hi_vec[NR_CPUS] __cacheline_aligned;

void __tasklet_schedule(struct tasklet_struct *t)
{
	int cpu = smp_processor_id();
	unsigned long flags;

	local_irq_save(flags);     // 保存 EFLAGS，并关闭中断
	t->next = tasklet_vec[cpu].list;  // 将 t 链入 cpu 对应的队列
	tasklet_vec[cpu].list = t;    // 一个时间内只能将其链入一个队列当中
	cpu_raise_softirq(cpu, TASKLET_SOFTIRQ);        // 正式发出软中断请求
	local_irq_restore(flags);
}

void __tasklet_hi_schedule(struct tasklet_struct *t)
{
	int cpu = smp_processor_id();
	unsigned long flags;

	local_irq_save(flags);
	t->next = tasklet_hi_vec[cpu].list;   // 链接到 cpu 的 list 头部
	tasklet_hi_vec[cpu].list = t;
	cpu_raise_softirq(cpu, HI_SOFTIRQ);   
	local_irq_restore(flags);
}

/* 处理软中断请求 */
void do_softirq()
{
    int cpu = smp_processor_id();
	__u32 pending;
	unsigned long flags;
	__u32 mask;

    // 软中断服务程序不允许在 硬中断服务程序内部执行，也不允许在软中断服务程序内部执行
    if(in_interrupt())    // 是否处于中断上下文当中
        return;

    local_irq_save(flags);   // 获取 EFLAGS 标志位到 flags 当中

    pending = softirq_pending(cpu);

    if(pending){
        struct softirq_action *h;

        mask = ~pending;
        local_bh_disable();       // 使用 barrier 提供串行化

restart:
        softirq_pending(cpu) = 0;       //  重置 cpu 的软中断请求标志位

        local_irq_enable();

        h = softirq_vec;

        do{
            if(pending & 1)
                h->action(h);    // 调用请求
            h++;
            pending >>= 1;
        }while(pending);

        local_irq_disable();

        pending = softirq_pending(cpu);
		if (pending & mask) {
			mask &= ~pending;
			goto restart;
		}
		__local_bh_enable();
	}

	local_irq_restore(flags);
}

/** 老式的 bh */
static void (*bh_base[32])(void);

void init_bh(int nr, void (*routine)(void))
{
	bh_base[nr] = routine;
	mb();
}


// 通过该锁将 bh 进行串行化
spinlock_t global_bh_lock = SPIN_LOCK_UNLOCKED;

// 执行服务的程序
static void bh_action(unsigned long nr)
{
    int cpu = smp_processor_id();

	if (!spin_trylock(&global_bh_lock))   // 只能有一个 CPU 运行
		goto resched;

	if (!hardirq_trylock(cpu))    
		goto resched_unlock;

	if (bh_base[nr])
		bh_base[nr]();     // 调用服务程序

	hardirq_endlock(cpu);
	spin_unlock(&global_bh_lock);
	return;

resched_unlock:
	spin_unlock(&global_bh_lock);
resched:
	mark_bh(nr);
}

// 对其他软中断的初始化
void open_softirq(int nr, void (*action)(struct softirq_action*), void *data)
{
    softirq_vec[nr].data = data;
    softirq_vec[nr].action = action;
}

static void tasklet_action(struct softirq_action *a)
{
	int cpu = smp_processor_id();
	struct tasklet_struct *list;

	// 串行化，获取到中断请求
	local_irq_disable();
	list = tasklet_vec[cpu].list;
	tasklet_vec[cpu].list = NULL;
	local_irq_enable();

	while(list){
		struct tasklet_struct *t = list;
		list = list->next;

		if(tasklet_trylock(t)) {
			if (!atomic_read(&t->count)) {   // 没有其他CPU在处理
				if (!test_and_clear_bit(TASKLET_STATE_SCHED, &t->state))
					BUG();
				t->func(t->data);      // 执行调用
				tasklet_unlock(t);
				continue;
			}
			tasklet_unlock(t);
		}

		local_irq_disable();   // 关中断
		t->next = tasklet_vec[cpu].list;
		tasklet_vec[cpu].list = t;
		__cpu_raise_softirq(cpu, TASKLET_SOFTIRQ);
		local_irq_enable();	   // 开中断
	}
}

static void tasklet_hi_action(struct softirq_action *a)
{
	int cpu = smp_processor_id();
	struct tasklet_struct *list;

	local_irq_disable();
	list = tasklet_hi_vec[cpu].list;
	tasklet_hi_vec[cpu].list = NULL;
	local_irq_enable();

	while (list) {
		struct tasklet_struct *t = list;

		list = list->next;

		if (tasklet_trylock(t)) {
			if (!atomic_read(&t->count)) {
				if (!test_and_clear_bit(TASKLET_STATE_SCHED, &t->state))
					BUG();
				t->func(t->data);
				tasklet_unlock(t);
				continue;
			}
			tasklet_unlock(t);
		}

		local_irq_disable();
		t->next = tasklet_hi_vec[cpu].list;
		tasklet_hi_vec[cpu].list = t;
		__cpu_raise_softirq(cpu, HI_SOFTIRQ);
		local_irq_enable();
	}
}

void tasklet_init(struct tasklet_struct *t, void (*func)(unsigned long), unsigned long data)
{
    t->next = NULL;
    t->state = 0;
    atomic_set(&t->count, 0);
    t->func = func;
    t->data = data;
}

void softirq_init()
{
    int i;
    // 对 bh 的32个 tasklet_struct 进行初始化,都指向 bh_action
    for(i=0; i<32; i++)
        tasklet_init(bh_task_vec+i, bh_action, i);
    
    open_softirq(TASKLET_SOFTIRQ, tasklet_action, NULL);
    open_softirq(HI_SOFTIRQ, tasklet_hi_action, NULL);
}