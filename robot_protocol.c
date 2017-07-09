#include <stdlib.h>
#include "robot_protocol.h"
#include "uart_api.h"

#define _UART_BAUD_RATE		57600
#define _UART_BITS			8
#define _UART_STOPS			1

typedef struct _BIOLOID_PACKET {
	unsigned char	startCode0;		// start code 0			(1byte)
	unsigned char	startCode1;		// start code 1			(1byte)
	unsigned char	dataLow0;		// low data				(1byte)
	unsigned char	dataLow1;		// inverted low data	(1byte)
	unsigned char	dataHigh0;		// high data			(1byte)
	unsigned char	dataHigh1;		// inverted high data	(1byte)
} BIOLOID_PACKET, *LPBIOLOID_PACKET;

LPBIOLOID_PACKET _createPacket(void);


int openRobotPort(void) {
	int status;

	status = uart_open();
	if (status < 0)
		return status;
	
	uart_config(UART1, _UART_BAUD_RATE, _UART_BITS, UART_PARNONE, _UART_STOPS);

	return 0;
}

void sendDataToRobot(short data) {
	LPBIOLOID_PACKET	packet	= _createPacket();

	packet->dataLow0	= data & 0xff;
	packet->dataLow1	= ~(packet->dataLow0);
	packet->dataHigh0	= (data >> 8) & 0xff;
	packet->dataHigh1	= ~(packet->dataHigh0);

	uart1_buffer_write((unsigned char*)packet, sizeof(BIOLOID_PACKET));

	free(packet);
}

LPBIOLOID_PACKET _createPacket(void) {
	LPBIOLOID_PACKET	packet	= malloc(sizeof(BIOLOID_PACKET));
	
	packet->startCode0	= 0xff;
	packet->startCode1	= 0x55;
	packet->dataLow0	= 0x00;
	packet->dataLow1	= 0x00;
	packet->dataHigh0	= 0x00;
	packet->dataHigh1	= 0x00;

	return packet;
}
