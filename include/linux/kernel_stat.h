#ifndef _LINUX_KERNEL_STAT_H
#define _LINUX_KERNEL_STAT_H

#include <linux/threads.h>

#define DK_MAX_MAJOR 16
#define DK_MAX_DISK 16

struct kernel_stat {
	unsigned int per_cpu_user[NR_CPUS],
	             per_cpu_nice[NR_CPUS],
	             per_cpu_system[NR_CPUS];
	unsigned int dk_drive[DK_MAX_MAJOR][DK_MAX_DISK];
	unsigned int dk_drive_rio[DK_MAX_MAJOR][DK_MAX_DISK];
	unsigned int dk_drive_wio[DK_MAX_MAJOR][DK_MAX_DISK];
	unsigned int dk_drive_rblk[DK_MAX_MAJOR][DK_MAX_DISK];
	unsigned int dk_drive_wblk[DK_MAX_MAJOR][DK_MAX_DISK];
	unsigned int pgpgin, pgpgout;
	unsigned int pswpin, pswpout;
	unsigned int context_swtch;
};

extern struct kernel_stat kstat;

#endif