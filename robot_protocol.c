#include <stdlib.h>
#include <stdio.h>

#include "robot_protocol.h"
#include "uart_api.h"

#define _UART_BAUD_RATE		9600
#define _UART_BITS			8
#define _UART_STOPS			1

int openRobotPort(void) {
	int status;

	status = uart_open();
	if (status < 0)
		return status;
	
	uart_config(UART1, _UART_BAUD_RATE, _UART_BITS, UART_PARNONE, _UART_STOPS);

	return 0;
}

void closeRobotPort(void) {
	uart_close();
}

void DelayLoop(int delay_time)
{
	while(delay_time)
		delay_time--;
}

void Send_Command(unsigned char Ldata)
{
	unsigned char Command_Buffer[1] = {0,};

	Command_Buffer[0] = Ldata;

	uart1_buffer_write(Command_Buffer, 1);
}

void waitMotion(void)
{
	unsigned char Command_Buffer[1] = {0,};
	
	uart1_buffer_read(Command_Buffer, 1);
}

#define ERROR	0
#define OK	1
