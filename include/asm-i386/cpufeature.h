#ifndef __ASM_I386_CPUFEATURE_H
#define __ASM_I386_CPUFEATURE_H


#define X86_FEATURE_PGE		(0*32+13) /* Page Global Enable */

#define boot_cpu_has(bit)	test_bit(bit, boot_cpu_data.x86_capability)
#define cpu_has_pge		boot_cpu_has(X86_FEATURE_PGE)       // 打开全局页面映射

#endif