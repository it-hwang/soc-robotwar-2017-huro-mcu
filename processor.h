#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#define PROCESSOR_CHECKER_PATH			"./main_processor.tmp"

#define PROCESSOR_FILE_ERROR			0x01
#define PROCESSOR_GRAPHIC_ERROR			0x02
#define PROCESSOR_ROBOT_PORT_ERROR		0x03
#define PROCESSOR_PROCESS_ERROR			0x04

int		openProcessor(void);
void	closeProcessor(void);

int		startProcessor(void);
void	stopProcessor(void);
int		isProcessorStarted(void);

#endif //__PROCESSOR_H__
