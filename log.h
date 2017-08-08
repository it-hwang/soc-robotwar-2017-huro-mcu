#ifndef __LOG_H__
#define __LOG_H__

#include <stdbool.h>

bool openLogFile(char* filePath);
void closeLogFile(void);
void printLog(char* szFormat, ...);

#endif // __LOG_H__
