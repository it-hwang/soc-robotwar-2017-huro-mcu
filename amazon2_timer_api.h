/*
 * pos_timer_api.h
 */

#ifndef __AMAZON2_TIMER_API_H__
#define __AMAZON2_TIMER_API_H__

#define TIMER_DEV_NAME "/dev/amazon2_timers"

#define	TIMER2		2
#define	TIMER3		3


struct timer_config {
    int timer;
    unsigned int interval_usec;
};

/* TIMER IOCTL command */
#define TIMER_CONFIG      0x24300 // arg: struct timer_config
#define TIMER_RUN         0x24301 // arg: timer channel
#define TIMER_STOP        0x24302 // arg: timer channel
#define TIMER_INT_ENABLE  0x24303 // arg: timer channel
#define TIMER_INT_DISABLE 0x24304 // arg: timer channel
#define TIMER_READ_COUNT  0x24305 // arg: timer channel
#define TIMER_COUNT_CLEAR 0x24306 // arg: timer channel
#define TIMER_READ_CUR_COUNT 0x24307 // arg: timer channel

/* TIMER API Function Prototypes */
int amazon2_timer_open (void);
void amazon2_timer_close (void);
int amazon2_timer_config(int timer, unsigned int interval_usec);
void amazon2_timer_run(int timer);
void amazon2_timer_stop(int timer);
void amazon2_timer_int_enable(int timer);
void amazon2_timer_int_disable(int timer);
unsigned int amazon2_timer_read_count(int timer); /* Return Global Count Variable */
void amazon2_timer_count_clear(int timer); /* Clear Global Count Variable */
unsigned int amazon2_timer_read_current_count(int timer); /* Return Timer Current Count Register */

#endif // __AMAZON2_TIMER_API_H__
