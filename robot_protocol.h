#ifndef __ROBOT_PROTOCOL_H__
#define __ROBOT_PROTOCOL_H__

int openRobotPort(void);
void closeRobotPort(void);
void sendDataToRobot(short data);
void DelayLoop(int delay_time);
void Send_Command(unsigned char Ldata);
void waitMotion(void);
void setHeadVertical(int angle);
void setHeadHorizontal(int angle);
void setHead(int horizontalAngle, int verticalAngle);

#endif // __ROBOT_PROTOCOL_H__


