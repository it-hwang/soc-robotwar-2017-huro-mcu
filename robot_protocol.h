#ifndef __ROBOT_PROTOCOL_H__
#define __ROBOT_PROTOCOL_H__

int openRobotPort(void);
void closeRobotPort(void);
void sendDataToRobot(short data);
void DelayLoop(int delay_time);
void Send_Command(unsigned char Ldata);

#endif // __ROBOT_PROTOCOL_H__


