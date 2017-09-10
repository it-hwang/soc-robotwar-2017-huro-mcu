#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdbool.h>
#include <stdint.h>

int openTimer(void);
void closeTimer(void);
bool isTimerOpened(void);
uint64_t getTime(void);

#endif // __TIMER_H__
