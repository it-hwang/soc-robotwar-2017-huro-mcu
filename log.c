#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "log.h"


static FILE* _pLogFile = NULL;
static struct timespec _startTime;


// 생성할 로그파일의 적합한 이름을 찾는다.
static bool _findNextLogFileName(char* filePath) {
    static const int MAX_FILES = 100000000;

    int i;
    for (i = 0; i < MAX_FILES; ++i) {
        sprintf(filePath, "./logs/log%d.txt", i);
        if (access(filePath, 0) != F_OK)
            return true;
    }
    return false;
}

int openLogFile(char* filePath) {
    static size_t BUFFER_SIZE = 256;

    bool hasOpened = (_pLogFile != NULL);
    if (hasOpened)
        return -1;

    char buffer[BUFFER_SIZE];
    if (!_findNextLogFileName(buffer))
        return -1;

    mkdir("./logs", 0755);
    FILE* pFile = fopen(buffer, "w");
    if (pFile == NULL)
        return -1;
    
    clock_gettime(CLOCK_MONOTONIC, &_startTime);
    _pLogFile = pFile;
    strncpy(filePath, buffer, BUFFER_SIZE);
    return 0;
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
