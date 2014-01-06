#ifndef WEENSYOS_SCHEDOS_H
#define WEENSYOS_SCHEDOS_H
#include "types.h"

/*****************************************************************************
 * schedos.h
 *
 *   Constants and variables shared by SchedOS's kernel and applications.
 *
 *****************************************************************************/

#define PART_ON 2

// System call numbers.
// An application calls a system call by causing the specified interrupt.

#define INT_SYS_YIELD			48
#define INT_SYS_EXIT			49
#define INT_SYS_USER1			50
#define INT_SYS_USER2			51
#define INT_SYS_PRIORITY		52
#define INT_SYS_PROPORTIONAL_SHARE 	53
#define INT_SYS_CHAR			54
#define INT_SYS_TICKET			55
#define INT_SYS_RM_TICKET		56

// The current screen cursor position (stored at memory location 0x198000).
extern uint16_t * volatile cursorpos;

// Sysrem lock
extern uint32_t lock;

#endif
