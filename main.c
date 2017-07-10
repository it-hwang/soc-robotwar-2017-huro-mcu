#include <stdio.h>
#include <unistd.h>

#include "processor.h"
#include "terminal.h"

#define _BLOCK_TIMEOUT_MILLISECONDS		3000

int		main(void);

void	_displayLogo(void);
int		_blockRunning(unsigned int milliseconds);


int main(void)
{
	_displayLogo();

	if (_blockRunning(_BLOCK_TIMEOUT_MILLISECONDS)) {
		printf("Program is interrupted.\n");
		return 0;
	}
	printf("Start program.\n");

	int errorCode;
	errorCode = openProcessor();
	if (errorCode == PROCESSOR_GRAPHIC_ERROR) {
		printf("[Error] Unable to open graphic.\n");
		return 1;
	}
	else if (errorCode == PROCESSOR_ROBOT_PORT_ERROR) {
		printf("[Error] Unable to open robot port.\n");
		return 1;
	}

	runProcessor();
	closeProcessor();

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

int _blockRunning(unsigned int milliseconds) {
	printf("Start program after %d seconds.\n", milliseconds);
	printf("1. Start immediately\n");
	printf("If you want to interrupt, please press any key except '1' key.\n");
	printf("\n");

	setConioTerminalMode();
	while (milliseconds > 0) {
		if (kbhit()) {
			if (getch() == '1') {
				resetTerminalMode();
				return 0;
			}
			else {
				resetTerminalMode();
				return 1;
			}
		}

		if (milliseconds % 1000 == 0) {
			resetTerminalMode();
			printf("Starting %d seconds before ...\n", milliseconds / 1000);
			setConioTerminalMode();
		}
		usleep(0);
		milliseconds -= 20;
	}

	if (kbhit()) {
		if (getch() == '1') {
			resetTerminalMode();
			return 0;
		}
		else {
			resetTerminalMode();
			return 1;
		}
	}

	resetTerminalMode();
	return 0;
}
