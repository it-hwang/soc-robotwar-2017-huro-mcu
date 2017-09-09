#include <stdbool.h>

#include "timer.h"
#include "amazon2_timer_api.h"
#include "log.h"

#define _TIMER_ID           2
#define _TIMER_INTERVAL     1000

static bool _isOpened = false;

int openTimer(void) {
    if (_isOpened) {
		printLog("Timer is opened already.\n");
        return 0;
    }

	if (amazon2_timer_open() < 0) {
		printLog("Timer Open Error!\n");
        return -1;
    }

	amazon2_timer_int_disable(_TIMER_ID);
    amazon2_timer_stop(_TIMER_ID);
    amazon2_timer_count_clear(_TIMER_ID);

	if (amazon2_timer_config(_TIMER_ID, _TIMER_INTERVAL) < 0) {
		printLog("Timer Config Error!\n");
        return -1;
    }

    amazon2_timer_int_enable(_TIMER_ID);
    amazon2_timer_run(_TIMER_ID);

    _isOpened = true;
    return 0;
}

void closeTimer(void) {
    if (!_isOpened)
        return;

    amazon2_timer_int_disable(_TIMER_ID);
    amazon2_timer_stop(_TIMER_ID);
    amazon2_timer_count_clear(_TIMER_ID);
    
    _isOpened = false;
}

uint64_t getTime(void) {
    if (_isOpened)
        return (uint64_t)amazon2_timer_read_count(_TIMER_ID) * _TIMER_INTERVAL;
    else
        return 0;
}
