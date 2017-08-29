// #define DEBUG

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "processor.h"
#include "terminal.h"
#include "log.h"
#include "robot_protocol.h"
#include "debug.h"

#define _MENU_TIMEOUT_MILLISECONDS		3000

// 느려짐 방지용 용량 늘리기
static char _dummyMemory[200000] = {0,};

static void _displayLogo(void);
static int _chooseMenu(unsigned int milliseconds);
static bool _findNextLogFileName(char* filePath);
static void _initLog(void);


int main(void) {
    _displayLogo();
    
    _initLog();

    int i = 0;
    _bigMemory[i] = 0;
    int command = _chooseMenu(_MENU_TIMEOUT_MILLISECONDS);
    if (command == 0) {
        printLog("Program is interrupted.\n");
        return 0;
    }

    int errorCode;
    errorCode = openProcessor();
    if (errorCode == PROCESSOR_GRAPHIC_ERROR) {
        printLog("[Error] Unable to open graphic.\n");
        return 1;
    }
    else if (errorCode == PROCESSOR_ROBOT_PORT_ERROR) {
        printLog("[Error] Unable to open robot port.\n");
        return 1;
    }

    printDebug("command: %d\n", command);
    runProcessor(command);
    printDebug("end\n");
    
    closeProcessor();

    closeLogFile();

    return 0;
}


static void _displayLogo(void) {
    printf("*** Welcome to Amazon Robot Platform Board ***\n");
}

static int _chooseMenu(unsigned int milliseconds) {
    printf("Start program after %.0f seconds.\n", (float)milliseconds / 1000);
    printf("1. Start Huro-C\n");
    printf("2. Adjust White Balance\n");
    printf("3. Capture screen\n");
    printf("4. Test\n");
    printf("If you want to interrupt, please press any key except menu keys.\n");
    printf("\n");

    setConioTerminalMode();
    while (milliseconds > 0) {
        if (milliseconds % 1000 == 0) {
            resetTerminalMode();
            printf("Starting %d seconds before ...\n", milliseconds / 1000);
            setConioTerminalMode();
        }
        mdelay(20);
        milliseconds -= 20;
        
        if (kbhit()) {
            char input = getch();
            if (input == '1') {
                resetTerminalMode();
                return 1;
            }
            else if (input == '2') {
                resetTerminalMode();
                return 2;
            }
            else if (input == '3') {
                resetTerminalMode();
                return 3;
            }
            else if (input == '4') {
                resetTerminalMode();
                return 4;
            }
            else {
                resetTerminalMode();
                return 0;
            }
        }
    }

    resetTerminalMode();
    return 1;
}


static bool _findNextLogFileName(char* filePath) {
    static const int MAX_FILES = 100000000;

    int i;
    for (i = 0; i < MAX_FILES; ++i) {
        sprintf(filePath, "./logs/log%d.txt", i);
        if (access(filePath, F_OK) != 0)
            return true;
    }
    return false;
}


void _initLog(void) {
    mkdir("./logs", 0755);
	
	char logFilePath[1024] = "";
	_findNextLogFileName(logFilePath);

	if (openLogFile(logFilePath))
		printf("[Log] File path: %s\n", logFilePath);
	else
		printf("[Log] Unable to create log file: %s\n", logFilePath);
}
