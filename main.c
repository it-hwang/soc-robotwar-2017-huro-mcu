#include <pthread.h>
#include <stdio.h>
#include "graphic_api.h"
#include "robot_protocol.h"

#define _ERROR_UNABLE_TO_OPEN_GRAPHIC		0x01
#define _ERROR_UNABLE_TO_OPEN_ROBOT_PORT	0x02
#define _ERROR_UNABLE_TO_CREATE_THREAD		0x03

int		main(void);

int		_initialize(void);
void	_finalize(void);

void	_loop(void* data);
void	_startMainThread(void);
void	_stopMainThread(void);
pthread_t	_mainThread;
int			_mainThreadId;

void	_displayLogo(void);



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

void _displayLogo(void) {
	printf("                                                              \n");
	printf("                                                              \n");
	printf("           *****************************************          \n");
	printf("                             Grobot                           \n");
	printf("             Welcome to Eagle Robot Platform Board            \n");
	printf("           *****************************************          \n");
	printf("                                                              \n");
}
