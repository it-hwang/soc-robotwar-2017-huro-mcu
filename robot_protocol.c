#include <stdlib.h>
#include "robot_protocol.h"


#define _UART_BAUD_RATE		57600

typedef struct _BIOLOID_PACKET {
	unsigned char	startCode0;		// start code 0			(1byte)
	unsigned char	startCode1;		// start code 1			(1byte)
	unsigned char	dataLow0;		// low data				(1byte)
	unsigned char	dataLow1;		// inverted low data	(1byte)
	unsigned char	dataHigh0;		// high data			(1byte)
	unsigned char	dataHigh1;		// inverted high data	(1byte)
} BIOLOID_PACKET, *LPBIOLOID_PACKET;

LPBIOLOID_PACKET _createPacket(void);


void sendDataToRobot(short data) {

}

LPBIOLOID_PACKET _createPacket(void) {
	LPBIOLOID_PACKET packet	= malloc(sizeof(BIOLOID_PACKET));
	
	packet->startCode0	= 0xff;
	packet->startCode1	= 0x55;
	packet->dataLow0	= 0x00;
	packet->dataLow1	= 0x00;
	packet->dataHigh0	= 0x00;
	packet->dataHigh1	= 0x00;

	return packet;
}
