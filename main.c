#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "processor.h"
#include "terminal.h"
#include "log.h"

#define _BLOCK_TIMEOUT_MILLISECONDS		3000

int main(void);

static void _displayLogo(void);
static int _blockRunning(unsigned int milliseconds);
static bool _findNextLogFileName(char* filePath);
static void _initLog(void);


int main(void)
{
	_displayLogo();
	
	_initLog();

	if (_blockRunning(_BLOCK_TIMEOUT_MILLISECONDS)) {
		printLog("사용자 요청에의해 프로그램이 중단되었습니다.\n");
		printf("Program is interrupted.\n");
		return 0;
	}
	printf("Start program.\n");

	int errorCode;
	errorCode = openProcessor();
	if (errorCode == PROCESSOR_GRAPHIC_ERROR) {
		printLog("[Error] Unable to open graphic.\n");
		printf("[Error] Unable to open graphic.\n");
		return 1;
	}
	else if (errorCode == PROCESSOR_ROBOT_PORT_ERROR) {
		printLog("[Error] Unable to open robot port.\n");
		printf("[Error] Unable to open robot port.\n");
		return 1;
	}

	printLog("프로세서 시작.\n");
	runProcessor();
	
	printLog("프로세서 종료.\n");
	closeProcessor();

	closeLogFile();

	return 0;
}


void _displayLogo(void) {
	printf("                                                              \n");
	printf("                                                              \n");
	printf("           *****************************************          \n");
	printf("                             Grobot                           \n");
	printf("            Welcome to Amazon Robot Platform Board            \n");
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
		usleep(0);  // 현재 보드에서 20밀리초 정도 쉰다.
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


bool _findNextLogFileName(char* filePath) {
    static const int MAX_FILES = 100000000;

    int i;
    for (i = 0; i < MAX_FILES; ++i) {
        sprintf(filePath, "./logs/log%d.txt", i);
        if (access(filePath, 0) != F_OK)
            return true;
    }
    return false;
}


void _initLog(void) {
	char logFilePath[1024] = "";
	_findNextLogFileName(logFilePath);

	if (openLogFile(logFilePath))
		printf("[Log] File path: %s\n", logFilePath);
	else
		printf("[Log] Unable to create log file.\n");
}
