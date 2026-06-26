extern unsigned long _sidata, _sdata, _edata, _sbss, _ebss, _estack;
extern void __libc_init_array(void);
extern int main(void);
extern void _exit(int code);

void Reset_Handler(void) {
	for (unsigned long *src = &_sidata, *dst = &_sdata; dst < &_edata;) {
		*dst++ = *src++;
	}
	for (unsigned long *p = &_sbss; p < &_ebss;) {
		*p++ = 0;
	}
	__libc_init_array();
	_exit(main());
}

void Default_Handler(void) {
	for (;;) {
	}
}

typedef void (*vector_t)(void);

__attribute__((used, section(".isr_vector"))) vector_t const vector_table[] = {
	(vector_t) &_estack,
	Reset_Handler,
	Default_Handler,
	Default_Handler,
};
