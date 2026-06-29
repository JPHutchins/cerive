#include <sys/stat.h>

enum {
	sys_open = 0x01,
	sys_write = 0x05,
	sys_exit_extended = 0x20,
	adp_stopped_application_exit = 0x20026,
};

static long semihost(long const op, void * const arg) {
	register long r0 __asm__("r0") = op;
	register void * r1 __asm__("r1") = arg;
	__asm__ volatile("bkpt 0xAB" : "+r"(r0) : "r"(r1) : "memory");
	return r0;
}

static int console = -1;

static void ensure_console(void) {
	if (console < 0) {
		static char const tt[] = ":tt";
		long const args[3] = {(long) tt, 4, sizeof(tt) - 1};
		console = (int) semihost(sys_open, (void *) args);
	}
}

int _write(int const fd, char const * const buf, int const len) {
	(void) fd;
	ensure_console();
	long const args[3] = {console, (long) buf, len};
	return len - (int) semihost(sys_write, (void *) args);
}

_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wanalyzer-infinite-loop\"")
__attribute__((__noreturn__)) void _exit(int const code) {
	long const args[2] = {adp_stopped_application_exit, code};
	semihost(sys_exit_extended, (void *) args);
	for (; ; ) {
	}
}
_Pragma("GCC diagnostic pop")

int _read(int const fd, char * const buf, int const len) {
	(void) fd, (void) buf, (void) len;
	return 0;
}
int _close(int const fd) {
	(void) fd;
	return -1;
}
int _lseek(int const fd, int const off, int const whence) {
	(void) fd, (void) off, (void) whence;
	return 0;
}
int _fstat(int const fd, struct stat * const st) {
	(void) fd;
	st->st_mode = S_IFCHR;
	return 0;
}
int _isatty(int const fd) {
	(void) fd;
	return 1;
}
int _kill(int const pid, int const sig) {
	(void) pid, (void) sig;
	return -1;
}
int _getpid(void) {
	return 1;
}

extern char _end;
static char * heap = &_end;
void * _sbrk(int const incr) {
	char * const prev = heap;
	heap += incr;
	return prev;
}

void _init(void) {}
void _fini(void) {}
