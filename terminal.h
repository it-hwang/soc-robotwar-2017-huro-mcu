#ifndef __TERMINAL_H__
#define __TERMINAL_H__

void resetTerminalMode(void);
void setConioTerminalMode(void);

int kbhit(void);
int getch(void);

#endif //__TERMINAL_H__
