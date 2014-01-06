#include "schedos-app.h"
#include "x86sync.h"

/*****************************************************************************
 * schedos-1
 *
 *   This tiny application prints red "1"s to the console.
 *   It yields the CPU to the kernel after each "1" using the sys_yield()
 *   system call.  This lets the kernel (schedos-kern.c) pick another
 *   application to run, if it wants.
 *
 *   The other schedos-* processes simply #include this file after defining
 *   PRINTCHAR appropriately.
 *
 *****************************************************************************/

#ifndef PRINTCHAR
#define PRINTCHAR	('1' | 0x0C00)
#endif

#ifndef PRIORITY
#define PRIORITY 5
#endif

#ifndef PROPORTIONAL_SHARE
#define PROPORTIONAL_SHARE 1
#endif

void
start(void)
{
	int extra_credit_on = 0;
	int i;

	sys_set_priority(PRIORITY);
	sys_set_proportional_share(PROPORTIONAL_SHARE);
	sys_ticket();

	for (i = 0; i < RUNCOUNT; i++) {
		#if PART_ON == 1
		// Write characters to the console, yielding after each one.
		*cursorpos++ = PRINTCHAR;
		#endif

		#if PART_ON == 2
		if (!extra_credit_on) {
			while(atomic_swap(&lock, 1) != 0)
				continue;

			*cursorpos++ = PRINTCHAR;
			atomic_swap(&lock, 0);
		} else {
			sys_print_char((uint16_t) PRINTCHAR);
		}
		#endif
		sys_yield();
	}

	sys_exit(0);
}

