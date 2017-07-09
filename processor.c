#include <stdio.h>
#include <unistd.h>

#include "processor.h"
#include "graphic_api.h"
#include "robot_protocol.h"

int		_isCheckerExists(void);
int		_createChecker(int pid);
void	_removeChecker(void);
int		_getProcessId(void);

void	_loop(void);


int openProcessor(void) {
	if (open_graphic() < 0)
		return PROCESSOR_GRAPHIC_ERROR;
	if (openRobotPort() < 0)
		return PROCESSOR_ROBOT_PORT_ERROR;
	
	return 0;
}

void closeProcessor(void) {
	_removeChecker();
	closeRobotPort();
	close_graphic();
}

int startProcessor(void) {
	return 0;
}

void stopProcessor(void) {

}

int isProcessorStarted(void) {
	return 0;
}


int _isCheckerExists(void) {
	return access(PROCESSOR_CHECKER_PATH, F_OK) == 0;
}

int _createChecker(int pid) {
	FILE* pFile = fopen(PROCESSOR_CHECKER_PATH, "w");
	if (pFile == NULL)
		return -1;
	
	fprintf(pFile, "%d", pid);
	fclose(pFile);
	return 0;
}

void _removeChecker(void) {
	if (_isCheckerExists())
		remove(PROCESSOR_CHECKER_PATH);
}

int _getProcessId(void) {
	if (!_isCheckerExists())
		return 0;
	
	FILE* pFile = fopen(PROCESSOR_CHECKER_PATH, "r");
	if (pFile == NULL)
		return -1;

	int pid;
	fscanf(pFile, "%d", &pid);
	fclose(pFile);
	return pid;
}

void _loop(void) {

}
