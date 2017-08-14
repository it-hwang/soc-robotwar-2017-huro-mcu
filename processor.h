#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#define PROCESSOR_GRAPHIC_ERROR 0x01
#define PROCESSOR_ROBOT_PORT_ERROR 0x02

int openProcessor(void);
void closeProcessor(void);

int runProcessor(int command);

#endif //__PROCESSOR_H__
