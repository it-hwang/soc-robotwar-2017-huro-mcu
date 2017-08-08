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
	unsigned char Command_Buffer[1] = {0,};
	
	Command_Buffer[0] = Ldata;

	uart1_buffer_write(Command_Buffer, 1);
}

void waitMotion(void)
{
	unsigned char Command_Buffer[1] = {0,};
	
	uart1_buffer_read(Command_Buffer, 1);
	printf("%d\n", Command_Buffer[0]);
}

void setHeadVertical(int angle) {
	if (angle < -90 || angle > 90)
		return;

	int offset = angle + 100;

	Send_Command(1);
	waitMotion();
	printf("1");
	Send_Command(17);
	waitMotion();
	printf("2");
	Send_Command(offset);
	waitMotion();
	printf("3");
}

void setHeadHorizontal(int angle) {
	if (angle < -90 || angle > 90)
		return;

	int offset = angle + 100;

	Send_Command(1);
	waitMotion();
	printf("4");
	Send_Command(11);
	waitMotion();
	printf("5");
	Send_Command(offset);
	waitMotion();
	printf("6");
}

void setHead(int horizontalAngle, int verticalAngle) {
	setHeadHorizontal(horizontalAngle);
	setHeadVertical(verticalAngle);
}

#define ERROR	0
#define OK	1
