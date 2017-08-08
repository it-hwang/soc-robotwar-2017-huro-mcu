#include <stdio.h>
#include <unistd.h>

#include "processor.h"
#include "terminal.h"
#include "log.h"

#define _BLOCK_TIMEOUT_MILLISECONDS		3000

int main(void);

void _displayLogo(void);
int _blockRunning(unsigned int milliseconds);


int main(void)
{
	_displayLogo();
	
	char logFilePath[1024];
	if (openLogFile(logFilePath) == 0)
		printf("[Log] File path: %s\n", logFilePath);
	else
		printf("[Log] Unable to create log file.\n");

	if (_blockRunning(_BLOCK_TIMEOUT_MILLISECONDS)) {
		printLog("사용자 요청에의해 프로그램이 중단되었습니다.");
		printf("Program is interrupted.\n");
		return 0;
	}
	printf("Start program.\n");

	int errorCode;
	errorCode = openProcessor();
	if (errorCode == PROCESSOR_GRAPHIC_ERROR) {
		printLog("[Error] Unable to open graphic.");
		printf("[Error] Unable to open graphic.\n");
		return 1;
	}
	else if (errorCode == PROCESSOR_ROBOT_PORT_ERROR) {
		printLog("[Error] Unable to open robot port.");
		printf("[Error] Unable to open robot port.\n");
		return 1;
	}

	printLog("프로세서 시작.");
	runProcessor();
	
	printLog("프로세서 종료.");
	closeProcessor();

	closeLogFile();

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
	printf("Start program after %.0f seconds.\n", (float)milliseconds / 1000);
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
