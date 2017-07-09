#include <stdio.h>
#include "graphic_api.h"
#include "robot_protocol.h"

#define ERROR_UNABLE_TO_OPEN_GRAPHIC		0x01
#define ERROR_UNABLE_TO_OPEN_ROBOT_PORT		0x02

void	displayLogo(void);

int		initialize(void);


int main(void)
{
	int errorCode;

	displayLogo();

	errorCode = initialize();
	if (errorCode == ERROR_UNABLE_TO_OPEN_GRAPHIC) {
		printf("[Error] Unable to open graphic.\n");
		return 1;
	}
	else if (errorCode == ERROR_UNABLE_TO_OPEN_ROBOT_PORT) {
		printf("[Error] Unable to open robot port.\n");
		return 1;
	}

	return 0;
}

void displayLogo(void) {
	printf("                                                              \n");
	printf("                                                              \n");
	printf("           *****************************************          \n");
	printf("                             Grobot                           \n");
	printf("             Welcome to Eagle Robot Platform Board            \n");
	printf("           *****************************************          \n");
	printf("                                                              \n");
}

int initialize(void) {
	if (open_graphic() < 0)
		return ERROR_UNABLE_TO_OPEN_GRAPHIC;
	
	if (openRobotPort() < 0)
		return ERROR_UNABLE_TO_OPEN_ROBOT_PORT;
	
	return 0;
}
