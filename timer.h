#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

int openTimer(void);
void closeTimer(void);
uint64_t getTime(void);

#endif // __TIMER_H__
