#include <stdlib.h>
#include <stdio.h>

#include "robot_protocol.h"
#include "uart_api.h"

#define _UART_BAUD_RATE		57600
#define _UART_BITS			8
#define _UART_STOPS			1

typedef struct _BIOLOID_PACKET {
	unsigned char startCode0;	// start code 0			(1byte)
	unsigned char startCode1;	// start code 1			(1byte)
	unsigned char dataLow0;		// low data				(1byte)
	unsigned char dataLow1; 	// inverted low data	(1byte)
	unsigned char dataHigh0;	// high data			(1byte)
	unsigned char dataHigh1;	// inverted high data	(1byte)
} BIOLOID_PACKET;

BIOLOID_PACKET* _createPacket(void);


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
	unsigned char Command_Buffer[6] = {0,};
	
	unsigned char Ldata1 = ~Ldata;

	Command_Buffer[0] = 0xff;	// Start Byte -> 0xff
	Command_Buffer[1] = 0x55; // Start Byte1 -> 0x55
    Command_Buffer[2] = Ldata;
	Command_Buffer[3] = Ldata1;
	Command_Buffer[4] = 0x00;  // 0x00
	Command_Buffer[5] = 0xff; // 0xff

	uart1_buffer_write(Command_Buffer, 6);
}

void waitMotion(void)
{
	unsigned char Command_Buffer[6] = {0,};
	
	uart1_buffer_read(Command_Buffer, 6);
}

#define ERROR	0
#define OK	1
