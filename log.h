#ifndef __LOG_H__
#define __LOG_H__

int openLogFile(char* filePath);
void closeLogFile(void);
void printLog(char* szFormat, ...);

#endif // __LOG_H__
