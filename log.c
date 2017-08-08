#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "log.h"


static FILE* _pLogFile = NULL;
static struct timespec _startTime;


bool openLogFile(char* filePath) {
    bool hasOpened = (_pLogFile != NULL);
    if (hasOpened)
        return false;

    FILE* pFile = fopen(filePath, "a");
    if (pFile == NULL)
        return false;
    
    clock_gettime(CLOCK_MONOTONIC, &_startTime);
    _pLogFile = pFile;
    return true;
}


void closeLogFile(void) {
    bool hasOpened = (_pLogFile != NULL);
    if (!hasOpened)
        return;
    
    fclose(_pLogFile);
    _pLogFile = NULL;
}


// BUG: 제대로된 시간을 받아오지 못한다.
static uint64_t _getElapsedMilliseconds(void) {
    static struct timespec endTime;
    clock_gettime(CLOCK_MONOTONIC, &endTime);

    return (endTime.tv_sec - _startTime.tv_sec) * 1000 +
           (endTime.tv_nsec - _startTime.tv_nsec) / 1000000;
}

static void _printLogTimestamp(void) {
    uint64_t elapsedTime = _getElapsedMilliseconds();
    int elapsedSeconds = elapsedTime / 1000;
    int elapsedMilliseconds = elapsedTime % 1000;
    fprintf(_pLogFile, "[%4d.%03d] ", elapsedSeconds, elapsedMilliseconds);
}

void printLog(char* szFormat, ...) {
    if (_pLogFile == NULL)
        return;

    // 타임스탬프 출력이 제대로 되지않아 주석처리하였다.
    // _printLogTimestamp();

    va_list lpStart;
    va_start(lpStart, szFormat);
    vfprintf(_pLogFile, szFormat, lpStart);
    va_end(lpStart);

    fprintf(_pLogFile, "\n");
    fflush(_pLogFile);
}
