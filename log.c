#include <stdio.h>
#include <stdarg.h>
#include "log.h"


static FILE* _pLogFile = NULL;

int openLogFile(void) {
    

    return 0;
}

void closeLogFile(void) {

}

void printLog(char* szFormat, ...) {
    va_list lpStart;
    va_start(lpStart, szFormat);
    vfprintf(_pLogFile, szFormat, lpStart);
    va_end(lpStart);
}
