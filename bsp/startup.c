extern unsigned long _sidata, _sdata, _edata, _sbss, _ebss, _estack;
extern void __libc_init_array(void);
extern int main(void);
extern void _exit(int code);

/*
 * Minimal vector table for the QEMU semihosting test harness.
 * Only Reset_Handler and a default handler are populated -- this is NOT
 * sufficient for real hardware. A production vector table needs all 16
 * Cortex-M system vectors (NMI, HardFault, MemManage, BusFault, UsageFault,
 * SVCall, PendSV, SysTick) plus device-specific IRQ vectors.
 */

void Reset_Handler(void) {
	for (unsigned long * src = &_sidata, *dst = &_sdata; dst < &_edata; ) {
		*dst++ = *src++;
	}
	for (unsigned long * p = &_sbss; p < &_ebss; ) {
		*p++ = 0;
	}
	__libc_init_array();
	_exit(main());
}

_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wanalyzer-infinite-loop\"")
__attribute__((__noreturn__)) void Default_Handler(void) {
	for (; ; ) {
	}
}
_Pragma("GCC diagnostic pop")

typedef void (*vector_t)(void);

__attribute__((used, section(".isr_vector"))) vector_t const vector_table[] = {
	(vector_t) &_estack,
	Reset_Handler,
	Default_Handler,
	Default_Handler,
};
