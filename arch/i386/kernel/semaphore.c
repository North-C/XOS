#include <asm-i386/semaphore.h>


// void __up(struct semaphore *sem)
// {
// 	wake_up(&sem->wait);
// }

// void __down(struct semaphore * sem)
// {
// 	struct task_struct *tsk = current;
// 	DECLARE_WAITQUEUE(wait, tsk);
// 	tsk->state = TASK_UNINTERRUPTIBLE;
// 	add_wait_queue_exclusive(&sem->wait, &wait);

// 	spin_lock_irq(&semaphore_lock);
// 	sem->sleepers++;
// 	for (;;) {
// 		int sleepers = sem->sleepers;

// 		/*
// 		 * Add "everybody else" into it. They aren't
// 		 * playing, because we own the spinlock.
// 		 */
// 		if (!atomic_add_negative(sleepers - 1, &sem->count)) {
// 			sem->sleepers = 0;
// 			break;
// 		}
// 		sem->sleepers = 1;	/* us - see -1 above */
// 		spin_unlock_irq(&semaphore_lock);

// 		schedule();
// 		tsk->state = TASK_UNINTERRUPTIBLE;
// 		spin_lock_irq(&semaphore_lock);
// 	}
// 	spin_unlock_irq(&semaphore_lock);
// 	remove_wait_queue(&sem->wait, &wait);
// 	tsk->state = TASK_RUNNING;
// 	wake_up(&sem->wait);
// }

// /*
//  * The semaphore operations have a special calling sequence that
//  * allow us to do a simpler in-line version of them. These routines
//  * need to convert that sequence back into the C sequence when
//  * there is contention on the semaphore.
//  *
//  * %ecx contains the semaphore pointer on entry. Save the C-clobbered
//  * registers (%eax, %edx and %ecx) except %eax when used as a return
//  * value..
//  */
// asm(
// ".align 4\n"
// ".globl __down_failed\n"
// "__down_failed:\n\t"
// 	"pushl %eax\n\t"
// 	"pushl %edx\n\t"
// 	"pushl %ecx\n\t"
// 	"call __down\n\t"
// 	"popl %ecx\n\t"
// 	"popl %edx\n\t"
// 	"popl %eax\n\t"
// 	"ret"
// );

// asm(
// ".align 4\n"
// ".globl __up_wakeup\n"
// "__up_wakeup:\n\t"
// 	"pushl %eax\n\t"
// 	"pushl %edx\n\t"
// 	"pushl %ecx\n\t"
// 	"call __up\n\t"
// 	"popl %ecx\n\t"
// 	"popl %edx\n\t"
// 	"popl %eax\n\t"
// 	"ret"
// );