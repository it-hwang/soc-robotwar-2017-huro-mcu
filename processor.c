#include <stdio.h>

#include "processor.h"
#include "graphic_api.h"
#include "robot_protocol.h"


int openProcessor(void) {
	if (open_graphic() < 0) {
		closeProcessor();
		return PROCESSOR_GRAPHIC_ERROR;
	}
	if (openRobotPort() < 0) {
		closeProcessor();
		return PROCESSOR_ROBOT_PORT_ERROR;
	}

	return 0;
}

void closeProcessor(void) {
	closeRobotPort();
	close_graphic();
}

int runProcessor(void) {
	printf("run!\n");

	return 0;
}
