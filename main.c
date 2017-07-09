#include <stdio.h>
#include "graphic_api.h"
#include "robot_protocol.h"

#define _ERROR_UNABLE_TO_OPEN_GRAPHIC		0x01
#define _ERROR_UNABLE_TO_OPEN_ROBOT_PORT	0x02

void	_displayLogo(void);

int		_initialize(void);
void	_finalize(void);


int main(void)
{
	int errorCode;

	_displayLogo();

	errorCode = _initialize();
	if (errorCode == _ERROR_UNABLE_TO_OPEN_GRAPHIC) {
		printf("[Error] Unable to open graphic.\n");
		return 1;
	}
	else if (errorCode == _ERROR_UNABLE_TO_OPEN_ROBOT_PORT) {
		printf("[Error] Unable to open robot port.\n");
		return 1;
	}

	_finalize();
	return 0;
}

void _displayLogo(void) {
	printf("                                                              \n");
	printf("                                                              \n");
	printf("           *****************************************          \n");
	printf("                             Grobot                           \n");
	printf("             Welcome to Eagle Robot Platform Board            \n");
	printf("           *****************************************          \n");
	printf("                                                              \n");
}

int _initialize(void) {
	if (open_graphic() < 0)
		return _ERROR_UNABLE_TO_OPEN_GRAPHIC;
	
	if (openRobotPort() < 0)
		return _ERROR_UNABLE_TO_OPEN_ROBOT_PORT;
	
	return 0;
}

void _finalize(void) {
	closeRobotPort();
	close_graphic();
}
