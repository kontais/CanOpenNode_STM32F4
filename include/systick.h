/*-----------------------------------------------------------------------
* systick.h  -
*
*
*
* Copyright (C) 2016 XCMG Group.
*
*-----------------------------------------------------------------------*/
#ifndef _SYS_TICK_H_
#define _SYS_TICK_H_

extern volatile uint32_t system_tick;

uint8_t time_out(uint32_t *start, uint32_t interval);

#endif /* _SYS_TICK_H_ */
